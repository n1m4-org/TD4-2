#include "WaveScene.h"

#include "application/collision/CollisionLayer.h"
#include "engine/gameobject/base/GameObject.h"
#include "engine/gameobject/component/collision/AABBColliderComponent.h"
#include "engine/gameobject/manager/GameObjectManager.h"
#include "engine/graphics/3d/Object3dCommon.h"
#include "gameobject/component/collision/CollisionManager.h"
#include "manager/scene/CameraManager.h"
#include "manager/scene/LightManager.h"
#include "scene/manager/SceneManager.h"
#include "time/TimeManager.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#include "manager/editor/DebugUIManager.h"
#endif

namespace
{
	// TestSceneと同様のディレクショナルライト設定
	constexpr Vector3 kLightDirection = {-0.2f, -1.0f, 0.3f};
	constexpr float kLightIntensity = 0.6f;
}

void WaveScene::Initialize()
{
	GameObjectManager::GetInstance()->Initialize();
	CollisionManager::GetInstance()->Initialize();

	// ライトの調整(TestSceneと同様)
	auto lightManager = sceneManager_->GetLightManager();
	DirectionalLight dirLight = lightManager->GetDirectionalLight();
	dirLight.direction = kLightDirection;
	dirLight.intensity = kLightIntensity;
	lightManager->SetDirectionalLight(dirLight);

	// デフォルトライトマネージャーの設定（Object3d描画用）
	sceneManager_->GetObject3dCommon()->SetDefaultLightManager(lightManager);

	// カメラをスポーン範囲(x:-8~8, z:8~15付近)を見渡せる位置に固定
	auto activeCamera = sceneManager_->GetCameraManager()->GetActiveCamera();
	activeCamera->SetTranslate({0.0f, 12.0f, -15.0f});
	activeCamera->SetRotate({0.3f, 0.0f, 0.0f});

	// 地面キューブの作成(TestSceneと同様。基準が無いと敵の位置が分かりづらいため)
	groundObject_ = std::make_unique<GameObject>("GroundCube");
	groundObject_->SetName("GroundCube");
	groundObject_->Initialize(sceneManager_->GetObject3dCommon(), sceneManager_->GetLightManager());
	groundObject_->SetModel("cube");
	groundObject_->GetModel()->SetUVScale({300.0f, 300.0f, 1.0f});
	groundObject_->SetPosition({0.0f, -10.0f, 0.0f});
	groundObject_->SetScale({150.0f, 10.0f, 150.0f});
	if (auto* obj3d = groundObject_->GetObject3d())
	{
		obj3d->SetCastShadow(false);
	}

	groundObject_->AddComponent("Collider", std::make_unique<GameObjectComponent::AABBColliderComponent>(groundObject_.get()));
	if (auto collider = groundObject_->GetComponent<GameObjectComponent::AABBColliderComponent>())
	{
		collider->SetCollisionLayer(CollisionLayer::Terrain);
		collider->SetCollisionMask(CollisionLayer::Player | CollisionLayer::Enemy);
	}
	GameObjectManager::GetInstance()->Register(groundObject_.get());

	waveSystem_ = std::make_unique<WaveSystem>();
	waveSystem_->Initialize(sceneManager_->GetSpriteCommon(), activeCamera);

#ifdef USE_IMGUI
	DebugUIManager::GetInstance()->RegisterDebugUI(this, "Wave Scene", [this]() { this->DrawImGui(); }, DebugUIArea::Inspector);
#endif

	StartState(SceneState::Playing);
}

void WaveScene::Finalize()
{
	GameObjectManager::GetInstance()->Finalize();
	CollisionManager::GetInstance()->Finalize();

	groundObject_.reset();

#ifdef USE_IMGUI
	if (DebugUIManager::HasInstance())
	{
		DebugUIManager::GetInstance()->UnregisterDebugUI(this);
	}
#endif
}

void WaveScene::Draw3D()
{
	GameObjectManager::GetInstance()->Draw3D(sceneManager_->GetCameraManager());
}

void WaveScene::Draw2D()
{
	GameObjectManager::GetInstance()->Draw2D();
}

void WaveScene::DrawShadow()
{
	GameObjectManager::GetInstance()->DrawShadow(sceneManager_->GetCameraManager()->GetActiveCamera());
}

void WaveScene::DrawGBuffer()
{
	GameObjectManager::GetInstance()->DrawGBuffer(sceneManager_->GetCameraManager());
}

void WaveScene::OnUpdatePlaying()
{
	CollisionManager::GetInstance()->UpdatePreviousPositions();

	waveSystem_->Update(TimeManager::GetInstance().GetGameContext().deltaTime);

	GameObjectManager::GetInstance()->Update();
	CollisionManager::GetInstance()->CheckCollisions();
}

#ifdef USE_IMGUI
void WaveScene::DrawImGui()
{
	ImGui::Text("Wave: %u / %zu", waveSystem_->GetCurrentWaveIndex() + 1, waveSystem_->GetWaveCount());
	ImGui::Text("Completed: %s", waveSystem_->IsCompleted() ? "true" : "false");
	ImGui::Text("Spawned Enemies: %zu", GameObjectManager::GetInstance()->FindAllWithTag("Enemy").size());

	if (!waveSystem_->HasStarted())
	{
		if (ImGui::Button("Start"))
		{
			waveSystem_->Start();
		}
	}

	if (waveSystem_->IsCompleted())
	{
		if (ImGui::Button("Clear Enemies"))
		{
			for (auto* enemy : GameObjectManager::GetInstance()->FindAllWithTag("Enemy"))
			{
				enemy->Destroy();
			}
		}
	}
}
#endif
