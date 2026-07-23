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
	titleLogo_->SetSize({700, 200});
	titleLogo_->SetPosition({960.0f, 200.0f}); // 画面中央上に配置

	// スタートテキストのスプライト作成
	startText_ = std::make_unique<Sprite>();
	startText_->Initialize(spCommon, "title/start.png");
	startText_->SetAnchorPoint({ 0.5f, 0.5f });
	startText_->SetSize({800, 100});
	startText_->SetPosition({960.0f, 900.0f}); // 画面中央下に配置

	// シーン遷移演出の初期化
	transitionEffect_.Initialize(spCommon, "./Resources/white1x1.png", 30, 30, WinApp::kClientWidth, WinApp::kClientHeight);

	// 各ステートの登録
	RegisterState("Enter", std::make_unique<TitleEnterState>());
	RegisterState("Wait", std::make_unique<TitleWaitState>());
	RegisterState("Exit", std::make_unique<TitleExitState>());

	// 初期ステートを登場演出（Enter）に設定
	ChangeState("Enter");

	// BGMの読み込みと再生（ループ再生）
	Audio::GetInstance()->LoadWave("TitleBGM", "titleBGM.wav", SoundGroup::BGM);
	Audio::GetInstance()->LoadWave("check", "check.wav", SoundGroup::SE);

}

void TitleScene::OnFinalize()
{
#ifdef USE_IMGUI
    if (DebugUIManager::HasInstance()) {
        DebugUIManager::GetInstance()->UnregisterDebugUI(this);
    }
#endif

	// シーン終了時にBGMを停止・破棄する
	Audio::GetInstance()->StopWave("TitleBGM");
	Audio::GetInstance()->UnloadWave("TitleBGM");
}

#include <cmath>

namespace
{
	// ロゴ揺れ演出のパラメータ（名前付き定数）
	constexpr float kLogoSwingSpeed = 1.5f;   // 揺れ速度
	constexpr float kLogoSwingAngle = 0.05f;  // 最大振幅（ラジアン）

	// スタートテキスト点滅演出のパラメータ（名前付き定数）
	constexpr float kStartTextBlinkSpeed = 2.5f; // 点滅速度
	constexpr float kStartTextMinAlpha = 0.2f;   // 最小透明度
}

void TitleScene::CommonUpdate()
{
	float dt = TimeManager::GetInstance().GetGameContext().deltaTime;

	// タイトルロゴのゆらゆら揺れる回転演出
	if (titleLogo_)
	{
		logoAnimTimer_ += dt;
		float rotation = std::sin(logoAnimTimer_ * kLogoSwingSpeed) * kLogoSwingAngle;
		titleLogo_->SetRotation(rotation);
		titleLogo_->Update();
	}

	// スタートテキストのゆっくりアルファ点滅演出
	if (startText_)
	{
		startTextAnimTimer_ += dt;
		float sinVal = (std::sin(startTextAnimTimer_ * kStartTextBlinkSpeed) + 1.0f) * 0.5f;
		float alpha = kStartTextMinAlpha + (1.0f - kStartTextMinAlpha) * sinVal;

		Vector4 color = startText_->GetColor();
		color.w = alpha;
		startText_->SetColor(color);
		startText_->Update();
	}

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
