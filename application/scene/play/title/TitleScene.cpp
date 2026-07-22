#include "TitleScene.h"

// engine/ecs
#include "engine/ecs/components/TransformComponent.h"
#include "engine/ecs/components/InstancedRenderComponent.h"
#include "engine/ecs/system/InstancedRenderSystem.h"
#include "engine/ecs/system/HierarchySystem.h"

// engine/graphics
#include "engine/manager/graphics/ModelManager.h"
#include "engine/graphics/3d/InstancedModelRenderer.h"
#include "engine/graphics/3d/Object3dCommon.h"

// engine/time
#include "engine/time/TimeManager.h"

// audio
#include "audio/Audio.h"
// scene
#include "engine/scene/manager/SceneManager.h"
#include "engine/scene/factory/SceneFactory.h"
// input
#include "input/Input.h"
// graphics / manager
#include "manager/effect/PostProcessManager.h"
#include "manager/graphics/LineManager.h"
#include <effects/particle/ParticleManager.h>
#include "effects/particle/ParticleEffect.h"
#ifdef USE_IMGUI
#include "manager/editor/DebugUIManager.h"
#include "externals/imgui/imgui.h"
#endif

// title states
#include "state/TitleEnterState.h"
#include "state/TitleWaitState.h"
#include "state/TitleExitState.h"

REGISTER_SCENE(TitleScene);

void TitleScene::Initialize()
{
#ifdef USE_IMGUI
    DebugUIManager::GetInstance()->RegisterDebugUI(this, "Title Scene", [this]() { this->DrawImGui(); }, DebugUIArea::Hierarchy);
#endif

	SpriteCommon* spCommon = sceneManager_->GetSpriteCommon();

	// タイトルロゴのスプライトを作成
	titleLogo_ = std::make_unique<Sprite>();
	titleLogo_->Initialize(spCommon, "title/logo.png");
	titleLogo_->SetAnchorPoint({ 0.5f, 0.5f });
	titleLogo_->SetPosition({960.0f, 200.0f}); // 画面中央上に配置

	// スタートテキストのスプライト作成
	startText_ = std::make_unique<Sprite>();
	startText_->Initialize(spCommon, "title/start.png");
	startText_->SetAnchorPoint({ 0.5f, 0.5f });
	startText_->SetPosition({960.0f, 850.0f}); // 画面中央下に配置

	// シーン遷移演出の初期化
	transitionEffect_.Initialize(spCommon, "./Resources/white1x1.png", 30, 30, WinApp::kClientWidth, WinApp::kClientHeight);

	// 各ステートの登録
	RegisterState("Enter", std::make_unique<TitleEnterState>());
	RegisterState("Wait", std::make_unique<TitleWaitState>());
	RegisterState("Exit", std::make_unique<TitleExitState>());

	// 初期ステートを登場演出（Enter）に設定
	ChangeState("Enter");
}

void TitleScene::OnFinalize()
{
#ifdef USE_IMGUI
    if (DebugUIManager::HasInstance()) {
        DebugUIManager::GetInstance()->UnregisterDebugUI(this);
    }
#endif
}

void TitleScene::CommonUpdate()
{
	titleLogo_->Update();
	startText_->Update();
	transitionEffect_.Update();
}

void TitleScene::Draw3D()
{

}

void TitleScene::Draw2D()
{
	titleLogo_->Draw();
	startText_->Draw();
	transitionEffect_.Draw();
}

void TitleScene::DrawImGui()
{
#ifdef USE_IMGUI
    ImGui::Text("Current State: %s", GetCurrentStateName().c_str());
#endif
}
