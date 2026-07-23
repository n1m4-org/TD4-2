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
#include "manager/scene/CameraManager.h"
#include "manager/scene/LightManager.h"
// input
#include "input/Input.h"
// graphics / manager
#include "manager/effect/PostProcessManager.h"
#include "manager/graphics/LineManager.h"
#include <effects/particle/ParticleManager.h>
#include "effects/particle/ParticleEffect.h"
#include "math/MathUtils.h"
#include "math/Easing.h"
#include <cmath>
#include <numbers>

#ifdef USE_IMGUI
#include "manager/editor/DebugUIManager.h"
#include "externals/imgui/imgui.h"
#endif

// title states
#include "state/TitleEnterState.h"
#include "state/TitleWaitState.h"
#include "state/TitleExitState.h"

REGISTER_SCENE(TitleScene);

namespace
{
	// ロゴ揺れ演出のパラメータ（名前付き定数）
	constexpr float kLogoSwingSpeed = 1.5f;   // 揺れ速度
	constexpr float kLogoSwingAngle = 0.05f;  // 最大振幅（ラジアン）
	constexpr float kLogoFloatSpeed = 2.0f;   // 浮遊速度
	constexpr float kLogoFloatHeight = 12.0f; // 浮遊高さ（ピクセル）

	// スタートテキスト点滅演出のパラメータ（名前付き定数）
	constexpr float kStartTextBlinkSpeed = 2.5f; // 点滅速度
	constexpr float kStartTextMinAlpha = 0.2f;   // 最小透明度

	// 3Dカメラ演出のパラメータ
	constexpr float kCameraOrbitSpeed = 0.25f;  // カメラ旋回速度
	constexpr float kCameraRadius = 22.0f;      // カメラ旋回半径
	constexpr float kCameraHeight = 7.0f;       // カメラ高さ
	constexpr Vector3 kCameraTarget = { 0.0f, 1.5f, 0.0f }; // 注視点
}

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
	titleLogo_->SetSize({ 700.0f, 200.0f });
	titleLogo_->SetPosition({ 960.0f, 220.0f }); // 画面中央上に配置

	// スタートテキストのスプライト作成
	startText_ = std::make_unique<Sprite>();
	startText_->Initialize(spCommon, "title/start.png");
	startText_->SetAnchorPoint({ 0.5f, 0.5f });
	startText_->SetSize({ 800.0f, 100.0f });
	startText_->SetPosition({ 960.0f, 900.0f }); // 画面中央下に配置

	// 3D背景演出オブジェクトの初期化
	Object3dCommon* objCommon = sceneManager_->GetObject3dCommon();
	LightManager* lightManager = sceneManager_->GetLightManager();

	if (objCommon && lightManager)
	{
		// スカイドーム
		skydome_ = std::make_unique<Object3d>();
		skydome_->Initialize(objCommon, lightManager);
		skydome_->SetModel("skydome");

		// 地面
		ground_ = std::make_unique<Object3d>();
		ground_->Initialize(objCommon, lightManager);
		ground_->SetModel("plane");
		ground_->SetScale({ 60.0f, 1.0f, 60.0f });
		ground_->SetPosition({ 0.0f, 0.0f, 0.0f });

		// プレイヤーモデル（中央）
		playerModel_ = std::make_unique<Object3d>();
		playerModel_->Initialize(objCommon, lightManager);
		playerModel_->SetModel("cube");
		playerModel_->SetPosition({ 0.0f, 1.5f, 0.0f });
		playerModel_->SetScale({ 2.0f, 2.0f, 2.0f });

		// エネミーモデル（サイドアクセント）
		enemyModel_ = std::make_unique<Object3d>();
		enemyModel_->Initialize(objCommon, lightManager);
		enemyModel_->SetModel("bombenemy");
		enemyModel_->SetPosition({ 6.0f, 1.5f, 3.0f });
		enemyModel_->SetScale({ 1.5f, 1.5f, 1.5f });
	}

	// パーティクルのロード
	ParticleManager::GetInstance()->Load("reflect", "Resources/json/particle/player_reflect.json");

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

void TitleScene::OnDecision()
{
	if (isDecided_)
	{
		return;
	}

	isDecided_ = true;
	decisionTimer_ = 0.0f;

	// 決定音再生
	Audio::GetInstance()->PlayWave("check");
	Audio::GetInstance()->SetVolume("check", 1.0f);

	// プレイヤー中央位置に決定パーティクルを発生
	ParticleManager::GetInstance()->Play("reflect", { 0.0f, 1.5f, 0.0f });
}

void TitleScene::CommonUpdate()
{
	float dt = TimeManager::GetInstance().GetGameContext().deltaTime;
	logoAnimTimer_ += dt;
	startTextAnimTimer_ += dt;
	cameraAngle_ += dt * kCameraOrbitSpeed;

	// --- 3D シネマティックカメラ演出 ---
	if (auto cameraMgr = sceneManager_->GetCameraManager())
	{
		if (auto camera = cameraMgr->GetActiveCamera())
		{
			Vector3 camPos;
			if (isDecided_)
			{
				decisionTimer_ += dt;
				float t = std::clamp(decisionTimer_ / 1.2f, 0.0f, 1.0f);
				float easedT = EaseOutQuad(t);

				// 決定時はプレイヤーにぐっとズームインする演出
				Vector3 normalPos = {
					std::sin(cameraAngle_) * kCameraRadius,
					kCameraHeight,
					-std::cos(cameraAngle_) * kCameraRadius
				};
				Vector3 zoomPos = {
					std::sin(cameraAngle_) * 8.0f,
					3.0f,
					-std::cos(cameraAngle_) * 8.0f
				};

				camPos = MathUtils::Lerp(normalPos, zoomPos, easedT);
			}
			else
			{
				// 通常時は中央を中心に優雅に周回旋回
				camPos = {
					std::sin(cameraAngle_) * kCameraRadius,
					kCameraHeight + std::sin(logoAnimTimer_ * 0.5f) * 1.5f, // ゆっくり上下微動
					-std::cos(cameraAngle_) * kCameraRadius
				};
			}

			camera->SetTranslate(camPos);

			// プレイヤー注視の回転計算
			Vector3 dir = kCameraTarget - camPos;
			float yaw = std::atan2(dir.x, dir.z);
			float pitch = -std::atan2(dir.y, std::sqrt(dir.x * dir.x + dir.z * dir.z));
			camera->SetRotate({ pitch, yaw, 0.0f });
		}
	}

	// 3Dモデルの自転アニメーション
	if (playerModel_)
	{
		playerModel_->SetRotation({ 0.0f, logoAnimTimer_ * 0.8f, 0.0f });
		playerModel_->Update();
	}
	if (enemyModel_)
	{
		enemyModel_->SetRotation({ 0.0f, -logoAnimTimer_ * 1.2f, 0.0f });
		enemyModel_->Update();
	}
	if (skydome_)
	{
		skydome_->Update();
	}
	if (ground_)
	{
		ground_->Update();
	}

	// --- 2D UI アニメーション演出 ---
	// タイトルロゴの上下浮遊 ＋ 揺れ回転
	if (titleLogo_)
	{
		float floatOffsetY = std::sin(logoAnimTimer_ * kLogoFloatSpeed) * kLogoFloatHeight;
		float rotation = std::sin(logoAnimTimer_ * kLogoSwingSpeed) * kLogoSwingAngle;

		titleLogo_->SetPosition({ 960.0f, 220.0f + floatOffsetY });
		titleLogo_->SetRotation(rotation);
		titleLogo_->Update();
	}

	// スタートテキストの息づく脈動（Pulse）＋アルファ点滅演出
	if (startText_)
	{
		if (isDecided_)
		{
			// 決定後は高速フリッカー点滅（ボタンが決定された視覚フィードバック）
			float flicker = (std::fmod(decisionTimer_ * 20.0f, 1.0f) > 0.5f) ? 1.0f : 0.1f;
			Vector4 color = startText_->GetColor();
			color.w = flicker;
			startText_->SetColor(color);
			startText_->SetSize({ 860.0f, 108.0f }); // 決定時に拡大
		}
		else
		{
			float sinVal = (std::sin(startTextAnimTimer_ * kStartTextBlinkSpeed) + 1.0f) * 0.5f;
			float alpha = kStartTextMinAlpha + (1.0f - kStartTextMinAlpha) * sinVal;
			float pulseScale = 1.0f + std::sin(startTextAnimTimer_ * 3.0f) * 0.04f;

			Vector4 color = startText_->GetColor();
			color.w = alpha;
			startText_->SetColor(color);
			startText_->SetSize({ 800.0f * pulseScale, 100.0f * pulseScale });
		}
		startText_->Update();
	}

	transitionEffect_.Update();
}

void TitleScene::Draw3D()
{
	if (skydome_)
	{
		skydome_->Draw();
	}
	if (ground_)
	{
		ground_->Draw();
	}
	if (playerModel_)
	{
		playerModel_->Draw();
	}
	if (enemyModel_)
	{
		enemyModel_->Draw();
	}
}

void TitleScene::Draw2D()
{
	if (titleLogo_)
	{
		titleLogo_->Draw();
	}
	if (startText_)
	{
		startText_->Draw();
	}
	transitionEffect_.Draw();
}

void TitleScene::DrawImGui()
{
#ifdef USE_IMGUI
    ImGui::Text("Current State: %s", GetCurrentStateName().c_str());
#endif
}
