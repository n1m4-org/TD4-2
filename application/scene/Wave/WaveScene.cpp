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
#include "engine/math/MathUtils.h"
#include "gameobject/component/collision/CollisionManager.h"
#include "manager/scene/CameraManager.h"
#include "manager/scene/LightManager.h"
#include "math/Easing.h"
#include "scene/factory/SceneFactory.h"
#include "scene/manager/SceneManager.h"
#include "time/TimeManager.h"
#include "audio/Audio.h"
#include "application/scene/state/SceneEnterState.h"
#include "application/scene/state/SceneExitState.h"
#include "input/Input.h"
#include "engine/manager/effect/PostProcessManager.h"
#include "engine/effects/postprocess/CRTEffect.h"

#include <algorithm>
#include <numbers>
#include <Windows.h>

using namespace GameObjectComponent;

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#include "manager/editor/DebugUIManager.h"
#endif

REGISTER_SCENE(WaveScene)

namespace
{
	// TestSceneと同様のディレクショナルライト設定
	constexpr Vector3 kLightDirection = {-0.2f, -1.0f, 0.3f};
	constexpr float kLightIntensity = 0.6f;
	constexpr Vector3 kReflectHandLocalPosition = {0.0f, 0.0f, 1.25f};
	constexpr Vector3 kReflectHandLocalScale = {1.5f, 1.125f, 1.125f};
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

	ParticleManager::GetInstance()->Load("reflect", "Resources/json/particle/player_reflect.json");
	ParticleManager::GetInstance()->Load("bomber", "Resources/json/particle/BombEffect.json");
	ParticleManager::GetInstance()->Load("bullet_hit", "Resources/json/particle/hit.json");
	ParticleManager::GetInstance()->Load("hand", "Resources/json/particle/hand.json");
	ParticleManager::GetInstance()->Load("smash", "Resources/json/particle/smash.json");
	ParticleManager::GetInstance()->Load("prediction", "Resources/json/particle/prediction.json");

	// デフォルトライトマネージャーの設定（Object3d描画用）
	sceneManager_->GetObject3dCommon()->SetDefaultLightManager(lightManager);

	// シーン遷移演出の初期化(TestSceneと同様)
	transitionEffect_.Initialize(sceneManager_->GetSpriteCommon(), "./Resources/white1x1.png", 30, 30, WinApp::kClientWidth, WinApp::kClientHeight);

	// ステート登録（Enter / Exit）
	RegisterState("Enter", std::make_unique<SceneEnterState>(&transitionEffect_, ""));
	auto exitState = std::make_unique<SceneExitState>(&transitionEffect_, "Title");
	exitState_ = exitState.get();
	RegisterState("Exit", std::move(exitState));

	// 初期ステートを登場演出（Enter）に設定
	ChangeState("Enter");

	// カメラの実際の配置はIntro演出(UpdateIntroCamera)が最初のフレームから制御する
	auto activeCamera = sceneManager_->GetCameraManager()->GetActiveCamera();

	// 地面キューブの作成(TestSceneと同様。基準が無いと敵の位置が分かりづらいため)
	groundObject_ = std::make_unique<GameObject>("GroundCube");
	groundObject_->SetName("GroundCube");
	groundObject_->Initialize(sceneManager_->GetObject3dCommon(), sceneManager_->GetLightManager());
	groundObject_->SetModel("ground");
	groundObject_->GetModel()->SetUVScale({600.0f, 600.0f, 1.0f});
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
	reflectHand->SetModel("racket");
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
					// ダメージ音の再生
					Audio::GetInstance()->PlayWave("se_playerDamage");

					if (status->GetHp() <= 0)
					{
						// プレイヤー死亡時のSE
						Audio::GetInstance()->PlayWave("se_playerDead");

						if (cameraState_ != CameraState::GameOver)
						{
							cameraState_ = CameraState::GameOver;
							cameraTimer_ = 0.0f;
						}
					}
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

	// ポーズメニュー(TestSceneと同様)
	pauseMenu_ = std::make_unique<PauseMenu>();
	pauseMenu_->Initialize(sceneManager_->GetSpriteCommon());

	// 結果UI（クリア／ゲームオーバー）の生成（演出終了まで非表示。TestSceneと同様）
	InitializeResultUI();

	// 色収差(RGBシフト)を無効化(TestSceneと同様)
	auto post = sceneManager_->GetPostProcessManager();
	post->crtEffect_->SetChromaticAberrationEnabled(false);

	// --- Audio Load ---
	Audio::GetInstance()->LoadWave("game_BGM", "gameplayBGM.wav", SoundGroup::BGM);

	Audio::GetInstance()->LoadWave("se_spawn", "spawn.wav", SoundGroup::SE);
	Audio::GetInstance()->LoadWave("se_damage", "damage.wav", SoundGroup::SE);
	Audio::GetInstance()->LoadWave("se_enemyDead", "enemyDead.wav", SoundGroup::SE);

	Audio::GetInstance()->LoadWave("se_bomb", "bomb.wav", SoundGroup::SE);
	Audio::GetInstance()->LoadWave("se_bullet", "bullet.wav", SoundGroup::SE);
	Audio::GetInstance()->LoadWave("se_missile", "missile.wav", SoundGroup::SE);

	Audio::GetInstance()->LoadWave("se_reflection", "reflection.wav", SoundGroup::SE);
	Audio::GetInstance()->LoadWave("se_swing", "swing.wav", SoundGroup::SE);
	Audio::GetInstance()->LoadWave("se_playerDead", "playerDead.wav", SoundGroup::SE);
	Audio::GetInstance()->LoadWave("se_skill", "skill.wav", SoundGroup::SE);
	Audio::GetInstance()->LoadWave("se_playerDamage", "playerDamage.wav", SoundGroup::SE);
	Audio::GetInstance()->LoadWave("pause", "pause.wav", SoundGroup::SE);
	Audio::GetInstance()->LoadWave("select", "select.wav", SoundGroup::SE);
	Audio::GetInstance()->LoadWave("check", "check.wav", SoundGroup::SE);
	Audio::GetInstance()->LoadWave("result_select", "select.wav", SoundGroup::SE);
	Audio::GetInstance()->LoadWave("result_check", "check.wav", SoundGroup::SE);

	Audio::GetInstance()->LoadWave("clear", "clear.wav", SoundGroup::SE);

	// --- BGM Start ---
	Audio::GetInstance()->PlayWave("game_BGM", true);
	Audio::GetInstance()->SetVolume("game_BGM", 0.2f);

#ifdef USE_IMGUI
	DebugUIManager::GetInstance()->RegisterDebugUI(this, "Wave Scene", [this]() { this->DrawImGui(); }, DebugUIArea::Inspector);
#endif
}

void WaveScene::OnFinalize()
{
	GameObjectManager::GetInstance()->Finalize();
	CollisionManager::GetInstance()->Finalize();

	groundObject_.reset();
	player_.reset();
	topDownCamera_.reset();

	// 結果UI（クリア／ゲームオーバー）の解放
	resultBackground_.reset();
	clearTitleSprite_.reset();
	gameOverTitleSprite_.reset();
	retryButton_.reset();
	quitButton_.reset();

	Audio::GetInstance()->StopWave("game_BGM");
	Audio::GetInstance()->UnloadWave("game_BGM");

	Audio::GetInstance()->UnloadWave("se_spawn");
	Audio::GetInstance()->UnloadWave("se_damage");
	Audio::GetInstance()->UnloadWave("se_enemyDead");
	Audio::GetInstance()->UnloadWave("se_bomb");
	Audio::GetInstance()->UnloadWave("se_bullet");
	Audio::GetInstance()->UnloadWave("se_missile");
	Audio::GetInstance()->UnloadWave("se_reflection");
	Audio::GetInstance()->UnloadWave("se_swing");
	Audio::GetInstance()->UnloadWave("se_playerDead");
	Audio::GetInstance()->UnloadWave("clear");
	Audio::GetInstance()->UnloadWave("se_skill");
	Audio::GetInstance()->UnloadWave("se_playerDamage");
	Audio::GetInstance()->UnloadWave("pause");
	Audio::GetInstance()->UnloadWave("select");
	Audio::GetInstance()->UnloadWave("check");
	Audio::GetInstance()->UnloadWave("result_select");
	Audio::GetInstance()->UnloadWave("result_check");

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

	// 結果UI（演出終了後のオーバーレイ）：クリアかゲームオーバーのどちらかを表示
	if (isClearUIVisible_)
	{
		DrawResultUI(clearTitleSprite_.get());
	}
	else if (isGameOverUIVisible_)
	{
		DrawResultUI(gameOverTitleSprite_.get());
	}

	// ポーズメニュー（ポーズ中のみ内部で描画。手前に重ねる）
	if (pauseMenu_)
	{
		pauseMenu_->Draw();
	}

	// シーン遷移演出を描画（最前列）
	transitionEffect_.Draw();
}

void WaveScene::DrawShadow()
{
	GameObjectManager::GetInstance()->DrawShadow(sceneManager_->GetCameraManager()->GetActiveCamera());
}

void WaveScene::DrawGBuffer()
{
	GameObjectManager::GetInstance()->DrawGBuffer(sceneManager_->GetCameraManager());
}

void WaveScene::UpdateCamera()
{
	switch (cameraState_)
	{
	case CameraState::Intro:
		UpdateIntroCamera();
		break;
	case CameraState::Playing:
		UpdateFollowCamera();
		break;
	case CameraState::Clear:
		UpdateClearDirection();
		break;
	case CameraState::GameOver:
		UpdateGameOverCamera();
		GameOverDirection();
		break;
	}
}

void WaveScene::UpdateFollowCamera()
{
	topDownCamera_->Update();
}

void WaveScene::UpdateIntroCamera()
{
	// TestSceneのUpdateIntroCameraと同様、遠景からプレイヤー付近へイージングで寄せる
	float deltaTime = TimeManager::GetInstance().GetGameContext().deltaTime;

	cameraTimer_ += deltaTime;

	float t = cameraTimer_ / kIntroTime;
	t = std::clamp(t, 0.0f, 1.0f);
	t = EaseOutQuad(t);

	auto camera = sceneManager_->GetCameraManager()->GetActiveCamera();

	Vector3 startPos = {0.0f, 130.0f, 80.0f};
	Vector3 endPos = player_->GetPosition() + Vector3(0.0f, 90.0f, -40.0f);
	camera->SetTranslate(MathUtils::Lerp(startPos, endPos, t));

	Vector3 startRot = {0.6f, 0.0f, 0.0f};
	Vector3 endRot = {1.2f, 0.0f, 0.0f};
	camera->SetRotate(MathUtils::Lerp(startRot, endRot, t));

	if (cameraTimer_ >= kIntroTime)
	{
		cameraState_ = CameraState::Playing;
		cameraTimer_ = 0.0f;

		// 最初の演出（Intro）が終わったら自動でウェーブを開始する
		if (waveSystem_ && !waveSystem_->HasStarted())
		{
			waveSystem_->Start();
		}
	}
}

void WaveScene::StartClearDirection()
{
	if (!player_ || isClearDirectionStarted_)
	{
		return;
	}

	isClearDirectionStarted_ = true;
	cameraState_ = CameraState::Clear;
	cameraTimer_ = 0.0f;

	Audio::GetInstance()->PlayWave("clear");

	auto camera = sceneManager_->GetCameraManager()->GetActiveCamera();

	// 演出開始時のカメラ状態を保存
	clearStartCameraPosition_ = camera->GetTranslate();
	clearStartCameraRotation_ = camera->GetRotate();

	// 演出開始時のプレイヤー状態を保存
	clearPlayerBasePosition_ = player_->GetPosition();
	clearPlayerBaseRotation_ = player_->GetRotation();

	// プレイヤーの物理移動を止める
	auto physics = player_->GetComponent<PhysicsComponent>();
	if (physics)
	{
		physics->SetMovementVelocity({0.0f, 0.0f, 0.0f});
		physics->SetExternalVelocity({0.0f, 0.0f, 0.0f});
	}
}

void WaveScene::UpdateClearDirection()
{
	if (!player_)
	{
		return;
	}

	const float deltaTime = TimeManager::GetInstance().GetGameContext().deltaTime;

	cameraTimer_ += deltaTime;

	auto camera = sceneManager_->GetCameraManager()->GetActiveCamera();

	const float playerYaw = clearPlayerBaseRotation_.y;

	// プレイヤーが向いている正面方向
	Vector3 playerForward = {
		std::sin(playerYaw),
		0.0f,
		std::cos(playerYaw)};

	if (playerForward.LengthSquared() > 0.000001f)
	{
		playerForward.NormalizeSelf();
	}
	else
	{
		playerForward = {0.0f, 0.0f, 1.0f};
	}

	// プレイヤーの正面側へカメラを置く
	const Vector3 clearCameraPosition =
		clearPlayerBasePosition_ +
		playerForward * kClearCameraDistance +
		Vector3{0.0f, kClearCameraHeight, 0.0f};

	const Vector3 clearCameraRotation = {
		0.2f,
		playerYaw + std::numbers::pi_v<float>,
		0.0f};

	// 前半：カメラをプレイヤー正面へ移動
	if (cameraTimer_ <= kClearCameraMoveTime)
	{
		float t = cameraTimer_ / kClearCameraMoveTime;
		t = std::clamp(t, 0.0f, 1.0f);

		const float easedT = EaseOutQuad(t);

		camera->SetTranslate(MathUtils::Lerp(clearStartCameraPosition_, clearCameraPosition, easedT));
		camera->SetRotate(MathUtils::Lerp(clearStartCameraRotation_, clearCameraRotation, easedT));

		return;
	}

	// カメラは正面位置で固定
	camera->SetTranslate(clearCameraPosition);
	camera->SetRotate(clearCameraRotation);

	// 後半：プレイヤーが回転しながらジャンプ
	const float actionElapsed = cameraTimer_ - kClearCameraMoveTime;

	float actionT = actionElapsed / kClearPlayerActionTime;
	actionT = std::clamp(actionT, 0.0f, 1.0f);

	// 0 → 1 → 0になる放物線
	const float jumpRate = 4.0f * actionT * (1.0f - actionT);

	Vector3 playerPosition = clearPlayerBasePosition_;
	playerPosition.y += kClearJumpHeight * jumpRate;

	// Y軸を1回転
	Vector3 playerRotation = clearPlayerBaseRotation_;
	playerRotation.y += 2.0f * std::numbers::pi_v<float> * actionT;

	player_->SetPosition(playerPosition);
	player_->SetRotation(playerRotation);

	// 演出終了時
	if (actionT >= 1.0f)
	{
		player_->SetPosition(clearPlayerBasePosition_);
		player_->SetRotation(clearPlayerBaseRotation_);

		cameraTimer_ = kClearCameraMoveTime + kClearPlayerActionTime;

		// 演出が終わったのでクリアUIを表示する
		isClearUIVisible_ = true;
	}
}

void WaveScene::UpdateGameOverCamera()
{
	float deltaTime = TimeManager::GetInstance().GetGameContext().deltaTime;

	cameraTimer_ += deltaTime;

	float t = cameraTimer_ / kGameOverTime;
	t = std::clamp(t, 0.0f, 1.0f);
	t = EaseOutQuad(t);

	auto camera = sceneManager_->GetCameraManager()->GetActiveCamera();

	Vector3 startPos = player_->GetPosition() + Vector3(0, 90, -40);
	Vector3 endPos = player_->GetPosition() + Vector3(0, 125, -55);

	camera->SetTranslate(MathUtils::Lerp(startPos, endPos, t));

	Vector3 startRot = {1.2f, 0, 0};
	Vector3 endRot = {0.8f, 0, 0};

	camera->SetRotate(MathUtils::Lerp(startRot, endRot, t));

	// 演出が終わったのでゲームオーバーUIを表示する
	if (cameraTimer_ >= kGameOverTime)
	{
		isGameOverUIVisible_ = true;
	}
}

void WaveScene::GameOverDirection()
{
	auto post = sceneManager_->GetPostProcessManager();

	// エフェクト自体を有効化
	post->crtEffect_->SetEnabled(true);
	post->crtEffect_->SetCrtEnabled(true);
	post->crtEffect_->SetChromaticAberrationEnabled(true);

	effectTimer_ += TimeManager::GetInstance().GetGameContext().deltaTime;

	// 0.35秒周期で色収差をON/OFFする
	float interval = 0.35f;
	float time = fmod(effectTimer_, interval);

	if (time < 0.25f)
	{
		post->crtEffect_->SetChromaticAberrationOffset(rgbShiftStrength_);
	}
	else
	{
		post->crtEffect_->SetChromaticAberrationOffset(0.0f);
	}
}

void WaveScene::InitializeResultUI()
{
	auto* spriteCommon = sceneManager_->GetSpriteCommon();

	// 仮画像：白1x1テクスチャを色分けして使う（後で正式な画像へ差し替え予定）
	const std::string kTexturePath = "./Resources/white1x1.png";

	// 背景の暗幕
	resultBackground_ = std::make_unique<Sprite>();
	resultBackground_->Initialize(spriteCommon, kTexturePath);
	resultBackground_->SetPosition({0.0f, 0.0f});
	resultBackground_->SetSize({Sprite::kCoordinateWidth, Sprite::kCoordinateHeight});
	resultBackground_->SetColor({0.0f, 0.05f, 0.15f, 0.5f});

	// 「クリア！」帯
	clearTitleSprite_ = std::make_unique<Sprite>();
	clearTitleSprite_->Initialize(spriteCommon, kTexturePath);
	clearTitleSprite_->SetAnchorPoint({0.5f, 0.5f});
	clearTitleSprite_->SetPosition(kResultTitlePos);
	clearTitleSprite_->SetSize(kResultTitleSize);
	clearTitleSprite_->SetColor({1.0f, 0.85f, 0.2f, 0.95f});

	// 「ゲームオーバー」帯
	gameOverTitleSprite_ = std::make_unique<Sprite>();
	gameOverTitleSprite_->Initialize(spriteCommon, kTexturePath);
	gameOverTitleSprite_->SetAnchorPoint({0.5f, 0.5f});
	gameOverTitleSprite_->SetPosition(kResultTitlePos);
	gameOverTitleSprite_->SetSize(kResultTitleSize);
	gameOverTitleSprite_->SetColor({0.75f, 0.12f, 0.12f, 0.95f});

	// ボタンを横並びに配置（中央を挟んで左：もう一度／右：ゲームを終了）
	const float halfSeparation = kResultButtonSize.x * 0.5f + kResultButtonGap * 0.5f;
	const Vector2 retryPos = {kResultUICenterX - halfSeparation, kResultButtonRowY};
	const Vector2 quitPos = {kResultUICenterX + halfSeparation, kResultButtonRowY};

	retryButton_ = std::make_unique<MenuButton>();
	retryButton_->Initialize(spriteCommon, kTexturePath, retryPos, kResultButtonSize);
	retryButton_->SetColors({0.2f, 0.5f, 0.2f, 0.9f}, {0.4f, 1.0f, 0.4f, 1.0f});

	quitButton_ = std::make_unique<MenuButton>();
	quitButton_->Initialize(spriteCommon, kTexturePath, quitPos, kResultButtonSize);
	quitButton_->SetColors({0.5f, 0.2f, 0.2f, 0.9f}, {1.0f, 0.4f, 0.4f, 1.0f});
}

void WaveScene::UpdateResultUI()
{
	if (resultBackground_)
	{
		resultBackground_->Update();
	}
	if (clearTitleSprite_)
	{
		clearTitleSprite_->Update();
	}
	if (gameOverTitleSprite_)
	{
		gameOverTitleSprite_->Update();
	}

	const Vector2 mousePos = Input::GetInstance()->GetMousePosition();
	const bool clicked = Input::GetInstance()->IsMouseButtonTriggered(0);

	// ホバー開始時SE
	if (retryButton_ && retryButton_->IsHoveredEnter())
	{
		Audio::GetInstance()->SetVolume("result_select", 1.0f);
		Audio::GetInstance()->PlayWave("result_select");
	}
	if (quitButton_ && quitButton_->IsHoveredEnter())
	{
		Audio::GetInstance()->SetVolume("result_select", 1.0f);
		Audio::GetInstance()->PlayWave("result_select");
	}

	// もう一度：Waveを最初からやり直す
	if (retryButton_ && retryButton_->Update(mousePos, clicked))
	{
		Audio::GetInstance()->SetVolume("result_check", 1.0f);
		Audio::GetInstance()->PlayWave("result_check");

		if (exitState_)
		{
			exitState_->SetNextSceneName("Wave");
		}
		ChangeState("Exit");
		return;
	}

	// 終了
	if (quitButton_ && quitButton_->Update(mousePos, clicked))
	{
		Audio::GetInstance()->SetVolume("result_check", 1.0f);
		Audio::GetInstance()->PlayWave("result_check");

		PostQuitMessage(0);
	}
}

void WaveScene::DrawResultUI(Sprite* titleSprite)
{
	if (resultBackground_)
	{
		resultBackground_->Draw();
	}
	if (titleSprite)
	{
		titleSprite->Draw();
	}
	if (retryButton_)
	{
		retryButton_->Draw();
	}
	if (quitButton_)
	{
		quitButton_->Draw();
	}
}

void WaveScene::CommonUpdate()
{
	transitionEffect_.Update();

	// クリア/ゲームオーバーの演出中・結果画面ではポーズを開けないようにする(TestSceneと同様)
	const bool isResultSequence =
		cameraState_ == CameraState::Clear ||
		cameraState_ == CameraState::GameOver ||
		isClearUIVisible_ ||
		isGameOverUIVisible_;

	// ESCでポーズメニューの切り替え（通常プレイ中のみ）
	if (pauseMenu_ && !isResultSequence)
	{
		const PauseMenu::Result pauseResult = pauseMenu_->Update();

		// 中央ボタン：現在のシーンを最初からやり直す
		if (pauseResult == PauseMenu::Result::Restart)
		{
			// ポーズ状態を次のシーンへ残さない
			TimeManager::GetInstance().Resume();

			if (exitState_)
			{
				exitState_->SetNextSceneName("Wave");
			}
			ChangeState("Exit");
			return;
		}

		// 右ボタン：タイトルへ戻る
		if (pauseResult == PauseMenu::Result::GoToTitle)
		{
			// ポーズ状態を次のシーンへ残さない
			TimeManager::GetInstance().Resume();

			if (exitState_)
			{
				exitState_->SetNextSceneName("Title");
			}
			ChangeState("Exit");
			return;
		}
	}

	// ポーズ中は通常のゲーム更新を止める
	if (pauseMenu_ && pauseMenu_->IsPaused())
	{
		return;
	}

	UpdateCamera();

	// クリア／ゲームオーバー演出が終わってUIが出ている間はボタン入力を処理する
	if (isClearUIVisible_ || isGameOverUIVisible_)
	{
		UpdateResultUI();
	}

	// Intro/クリア/ゲームオーバー演出中はゲームプレイの更新を止める(TestSceneと同様)
	if (cameraState_ != CameraState::Playing)
	{
		return;
	}

	CollisionManager::GetInstance()->UpdatePreviousPositions();

	waveSystem_->Update(TimeManager::GetInstance().GetGameContext().deltaTime);

	GameObjectManager::GetInstance()->Update();
	CollisionManager::GetInstance()->CheckCollisions();

	// 全Wave完了 かつ 敵を全滅させた場合をゲームクリアとして扱い、クリア演出を開始する
	const bool isWaveClear =
		waveSystem_->IsCompleted() &&
		GameObjectManager::GetInstance()->FindAllWithTag("Enemy").empty();

	if (isWaveClear)
	{
		StartClearDirection();
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

	ImGui::Separator();
	ImGui::Text("Spawn 1 Enemy");
	static const char* kDebugSpawnEnemyTypes[] = { "Dash", "Charge", "Homing", "Bomb" };
	for (const char* type : kDebugSpawnEnemyTypes)
	{
		if (ImGui::Button(type))
		{
			waveSystem_->SpawnSingleEnemy(type);
		}
		ImGui::SameLine();
	}
	ImGui::NewLine();
}
#endif
