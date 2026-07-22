#include "WaveScene.h"

#include "application/collision/CollisionLayer.h"
#include "application/gameobject/component/action/common/PhysicsComponent.h"
#include "application/gameobject/component/action/common/StatusComponent.h"
#include "application/gameobject/component/action/common/UIComponent.h"
#include "application/gameobject/component/action/player/PlayerInputComponent.h"
#include "application/gameobject/component/action/player/PlayerMoveComponent.h"
#include "application/gameobject/component/action/player/PlayerReflectComponent.h"
#include "application/gameobject/component/action/player/PlayerSlowMotionComponent.h"
#include "application/gameobject/GameObjectTag.h"
#include "engine/effects/particle/ParticleManager.h"
#include "engine/gameobject/base/GameObject.h"
#include "engine/gameobject/component/collision/AABBColliderComponent.h"
#include "engine/gameobject/component/collision/OBBColliderComponent.h"
#include "engine/gameobject/manager/GameObjectManager.h"
#include "engine/graphics/3d/Object3dCommon.h"
#include "gameobject/component/collision/CollisionManager.h"
#include "manager/scene/CameraManager.h"
#include "manager/scene/LightManager.h"
#include "scene/factory/SceneNames.h"
#include "scene/manager/SceneManager.h"
#include "time/TimeManager.h"

using namespace GameObjectComponent;

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#include "manager/editor/DebugUIManager.h"
#endif

namespace
{
	// TestSceneと同様のディレクショナルライト設定
	constexpr Vector3 kLightDirection = {-0.2f, -1.0f, 0.3f};
	constexpr float kLightIntensity = 0.6f;
	constexpr Vector3 kReflectHandLocalPosition = {0.0f, 0.0f, 1.25f};
	constexpr Vector3 kReflectHandLocalScale = {1.5f, 0.35f, 0.35f};
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

	// プレイヤーの作成(TestSceneと同様)
	player_ = std::make_unique<GameObject>(GameObjectTag::Player);
	player_->SetName("Player");
	player_->Initialize(sceneManager_->GetObject3dCommon(), sceneManager_->GetLightManager());
	player_->SetModel("cube");
	player_->SetPosition({0.0f, 2.0f, 0.0f});
	player_->SetScale({2.0f, 2.0f, 2.0f});

	player_->AddComponent("Input", std::make_unique<PlayerInputComponent>());
	player_->AddComponent("Move", std::make_unique<PlayerMoveComponent>(activeCamera));
	player_->AddComponent("Status", std::make_unique<StatusComponent>(player_.get()));
	player_->AddComponent("Physics", std::make_unique<PhysicsComponent>(player_.get()));
	player_->AddComponent(
		"Reflect",
		std::make_unique<PlayerReflectComponent>(activeCamera, sceneManager_->GetSpriteCommon()));
	player_->AddComponent("SlowMotion", std::make_unique<PlayerSlowMotionComponent>(lightManager));
	player_->AddComponent(
		"UI",
		std::make_unique<UIComponent>(
			player_.get(),
			sceneManager_->GetSpriteCommon(),
			activeCamera,
			Vector3(0.0f, 6.0f, 0.0f)));

	// 反射判定用の子オブジェクト(TestSceneと同様)
	auto reflectHand = std::make_unique<GameObject>(GameObjectTag::Player);
	reflectHand->SetName("ReflectHand");
	reflectHand->Initialize(sceneManager_->GetObject3dCommon(), sceneManager_->GetLightManager());
	reflectHand->SetActive(false);
	reflectHand->SetPosition(kReflectHandLocalPosition);
	reflectHand->SetScale(kReflectHandLocalScale);
	auto reflectCollider = std::make_unique<GameObjectComponent::OBBColliderComponent>(reflectHand.get());
	reflectCollider->SetActive(false);
	reflectCollider->SetCollisionLayer(CollisionLayer::None);
	reflectCollider->SetCollisionMask(CollisionLayer::EnemyBullet | CollisionLayer::Enemy);
	reflectCollider->SetOnEnter([this](const CollisionInfo& info)
	{
		if (!info.otherCollider)
		{
			return;
		}

		if (player_)
		{
			auto reflectComp = player_->GetComponent<PlayerReflectComponent>();
			if (reflectComp)
			{
				ParticleManager::GetInstance()->Play("reflect", info.otherCollider->GetOwner()->GetPosition());
				reflectComp->NotifyReflectSucceeded();
			}
		}
	});
	reflectHand->AddComponent("ReflectCollider", std::move(reflectCollider));
	player_->AddChild("ReflectHand", std::move(reflectHand));

	// AABBコライダーの追加
	player_->AddComponent("Collider", std::make_unique<GameObjectComponent::AABBColliderComponent>(player_.get()));
	if (auto collider = player_->GetComponent<GameObjectComponent::AABBColliderComponent>())
	{
		collider->SetCollisionLayer(CollisionLayer::Player);
		collider->SetCollisionMask(CollisionLayer::Enemy | CollisionLayer::Stage | CollisionLayer::Terrain | CollisionLayer::Bumpers | CollisionLayer::EnemyBullet);

		auto handlePlayerCollision = [this](const CollisionInfo& info)
		{
			if (!info.otherCollider)
				return;
			uint32_t targetLayers = CollisionLayer::Terrain | CollisionLayer::Stage | CollisionLayer::Bumpers;
			if (!(info.otherCollider->GetCollisionLayer() & targetLayers))
				return;
			if (!player_)
				return;

			Vector3 pos = player_->GetPosition();
			pos += info.normal * info.depth;
			player_->SetPosition(pos);

			auto physics = player_->GetComponent<PhysicsComponent>();
			if (!physics)
				return;

			if (info.normal.y > 0.0f)
			{
				physics->SetGrounded(true);
				Vector3 vel = physics->GetExternalVelocity();
				if (vel.y < 0.0f)
				{
					vel.y = 0.0f;
					physics->SetExternalVelocity(vel);
				}
			}
		};

		collider->SetOnEnter([this, handlePlayerCollision](const CollisionInfo& info)
		{
			handlePlayerCollision(info);

			if (!info.otherCollider)
			{
				return;
			}

			if (info.otherCollider->GetCollisionLayer() & CollisionLayer::EnemyBullet)
			{
				auto status = player_->GetComponent<StatusComponent>();
				if (status)
				{
					status->SetHp(status->GetHp() - 10);
				}
			}
		});
		collider->SetOnStay([handlePlayerCollision](const CollisionInfo& info)
		{
			handlePlayerCollision(info);
		});
		collider->SetOnExit([](const CollisionInfo& info) {});
	}
	GameObjectManager::GetInstance()->Register(player_.get());

	// 追従カメラの初期化(TestSceneと同様)
	topDownCamera_ = std::make_unique<TopDownCamera>();
	topDownCamera_->Initialize(activeCamera);
	topDownCamera_->SetPitch(1.2f);
	topDownCamera_->SetOffset({0.0f, 0.0f, -40.0f});
	topDownCamera_->Start(105.0f, &player_->GetPosition());

	waveSystem_ = std::make_unique<WaveSystem>();
	waveSystem_->Initialize(sceneManager_->GetSpriteCommon(), activeCamera);
	waveSystem_->SetPlayer(player_.get());

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
	player_.reset();
	topDownCamera_.reset();

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
	topDownCamera_->Update();

	CollisionManager::GetInstance()->UpdatePreviousPositions();

	waveSystem_->Update(TimeManager::GetInstance().GetGameContext().deltaTime);

	GameObjectManager::GetInstance()->Update();
	CollisionManager::GetInstance()->CheckCollisions();

	if (waveSystem_->IsCompleted())
	{
		clearSceneRequested_ = true;
		sceneManager_->ChangeScene(SceneNames::Title);
	}
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

	if (ImGui::Button("Restart"))
	{
		for (auto* enemy : GameObjectManager::GetInstance()->FindAllWithTag("Enemy"))
		{
			enemy->Destroy();
			}
		waveSystem_->Restart();
	}

	if (ImGui::Button("Force Complete"))
	{
		// 全Wave完了フラグを立てつつ敵を全消しし、疑似的なゲームクリア状態を作り出す
		waveSystem_->ForceComplete();
		for (auto* enemy : GameObjectManager::GetInstance()->FindAllWithTag("Enemy"))
		{
			enemy->Destroy();
		}
	}
}
#endif
