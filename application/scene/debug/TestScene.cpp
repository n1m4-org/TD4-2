#include "TestScene.h"
#include "application/collision/CollisionLayer.h"
#include "application/gameobject/component/action/common/PhysicsComponent.h"
#include "application/gameobject/component/action/common/StatusComponent.h"
#include "application/gameobject/component/action/common/UIComponent.h"
#include "application/gameobject/component/action/enemy/bomb/BombMoveComponent.h"
#include "application/gameobject/component/action/enemy/bullet/BulletBehaviorComponent.h"
#include "application/gameobject/component/action/enemy/charge/ChargeMoveComponent.h"
#include "application/gameobject/component/action/enemy/EnemyDeathDirectionComponent.h"
#include "application/gameobject/component/action/enemy/horming/HormingMoveComponent.h"
#include "application/gameobject/component/action/player/PlayerInputComponent.h"
#include "application/gameobject/component/action/player/PlayerMoveComponent.h"
#include "application/gameobject/component/action/player/PlayerReflectComponent.h"
#include "application/gameobject/component/action/player/PlayerSlowMotionComponent.h"
#include "application/gameobject/GameObjectTag.h"
#include "engine/effects/particle/ParticleManager.h"
#include "engine/gameobject/component/collision/AABBColliderComponent.h"
#include "engine/gameobject/component/collision/CollisionManager.h"
#include "engine/gameobject/component/collision/OBBColliderComponent.h"
#include "engine/gameobject/manager/GameObjectManager.h"
#include "engine/math/MathUtils.h"
#include "engine/graphics/3d/Object3dCommon.h"
#include "engine/time/TimeManager.h"
#include "engine/math/Easing.h"
#include "input/Input.h"
#include "manager/editor/GameObjectEditor.h"
#include "manager/scene/CameraManager.h"
#include "manager/scene/LightManager.h"
#include "scene/manager/SceneManager.h"
#include "engine/manager/effect/PostProcessManager.h"
#include "engine/effects/postprocess/CRTEffect.h"

#include "engine/scene/factory/SceneFactory.h"
REGISTER_SCENE(TestScene);

using namespace GameObjectComponent;

void TestScene::Initialize()
{
	// カメラの設定
	sceneManager_->GetCameraManager()->GetActiveCamera()->SetTranslate({0.0f, 10.0f, 30.0f});
	sceneManager_->GetCameraManager()->GetActiveCamera()->SetRotate({0.1f, 0.0f, 0.0f});

	// ライトの調整
	auto lightManager = sceneManager_->GetLightManager();
	DirectionalLight dirLight = lightManager->GetDirectionalLight();
	dirLight.direction = kLightDirection;
	dirLight.intensity = kLightIntensity;
	lightManager->SetDirectionalLight(dirLight);

	// スポットライトの作成
	lightManager->AddSpotLight("player_spot_light");
	// スポットライトの初期設定 (明るさを０にしておく)
	lightManager->SetSpotLightIntensity("player_spot_light", 0.0f);
	lightManager->SetSpotLightDirection("player_spot_light", {0.0f, -1.0f, -0.3f});
	lightManager->SetSpotLightDistance("player_spot_light", 50.0);

	// デフォルトライトマネージャーの設定（Object3d描画用）
	sceneManager_->GetObject3dCommon()->SetDefaultLightManager(sceneManager_->GetLightManager());

	// デバッグカメラの初期化
	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize(sceneManager_->GetCameraManager()->GetActiveCamera());
	debugCamera_->Start({0.0f, 10.0f, -30.0f}, {0.2f, 0.0f, 0.0f});

	// 追従カメラの初期化
	topDownCamera_ = std::make_unique<TopDownCamera>();
	topDownCamera_->Initialize(sceneManager_->GetCameraManager()->GetActiveCamera());

	// ゲームオブジェクトマネージャーの初期化
	GameObjectManager::GetInstance()->Initialize();

	// コリジョンマネージャーの初期化
	CollisionManager::GetInstance()->Initialize();

	// GameObjectEditorの初期化
	GameObjectEditor::GetInstance()->Initialize();

#ifdef USE_IMGUI
	DebugUIManager::GetInstance()->RegisterDebugUI(this, "Collision Layer Test", [this]()
	{ this->DrawImGui(); }, DebugUIArea::Console);
#endif

	// パーティクルのロード
	ParticleManager::GetInstance()->Load("reflect", "Resources/json/particle/player_reflect.json");
	ParticleManager::GetInstance()->Load("bomber", "Resources/json/particle/BombEffect.json");
	ParticleManager::GetInstance()->Load("bullet_hit", "Resources/json/particle/hit.json");

	// 1. テスト用キューブオブジェクトの作成
	player_ = std::make_unique<GameObject>(GameObjectTag::Player);
	player_->SetName("TestCube");
	player_->Initialize(sceneManager_->GetObject3dCommon(), sceneManager_->GetLightManager());
	player_->SetModel("cube");
	player_->SetPosition({0.0f, 2.0f, 0.0f});
	player_->SetScale({2.0f, 2.0f, 2.0f});

	// アクション・物理・ステータスコンポーネントの追加
	player_->AddComponent("Input", std::make_unique<PlayerInputComponent>());
	player_->AddComponent("Move", std::make_unique<PlayerMoveComponent>(sceneManager_->GetCameraManager()->GetActiveCamera()));

	// targetObject_はこの時点ではまだ作られていないので、nullptrで追加しておく
	player_->AddComponent("Horming", std::make_unique<HormingMoveComponent>(nullptr));
	player_->AddComponent("Status", std::make_unique<StatusComponent>(player_.get()));
	player_->AddComponent("Physics", std::make_unique<PhysicsComponent>(player_.get()));
	player_->AddComponent(
		"Reflect",
		std::make_unique<PlayerReflectComponent>(
			sceneManager_->GetCameraManager()->GetActiveCamera(),
			sceneManager_->GetSpriteCommon()));
	player_->AddComponent("SlowMotion", std::make_unique<PlayerSlowMotionComponent>(sceneManager_->GetLightManager()));
	player_->AddComponent(
		"UI",
		std::make_unique<UIComponent>(
			player_.get(),
			sceneManager_->GetSpriteCommon(),
			sceneManager_->GetCameraManager()->GetActiveCamera(),
			Vector3(0.0f, 6.0f, 0.0f) // プレイヤーの頭上少し上
			));

	// 円弧の基準位置として、プレイヤーの少し前へ反射判定を配置する。
	auto reflectHand = std::make_unique<GameObject>(GameObjectTag::Player);
	reflectHand->SetName("ReflectHand");
	reflectHand->Initialize(sceneManager_->GetObject3dCommon(), sceneManager_->GetLightManager());
	reflectHand->SetActive(false);
	reflectHand->SetPosition(kReflectHandLocalPosition);
	reflectHand->SetScale(kReflectHandLocalScale);
	auto reflectCollider = std::make_unique<OBBColliderComponent>(reflectHand.get());
	reflectCollider->SetActive(false); // 初期状態は非アクティブ（反射発動時のみ有効化）
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
	player_->AddComponent("Collider", std::make_unique<AABBColliderComponent>(player_.get()));
	if (auto collider = player_->GetComponent<AABBColliderComponent>())
	{
		collider->SetCollisionLayer(CollisionLayer::Player);
		collider->SetCollisionMask(CollisionLayer::Enemy | CollisionLayer::Stage | CollisionLayer::Terrain | CollisionLayer::Bumpers | CollisionLayer::EnemyBullet);

		// 衝突時の共通押し戻し・接地処理
		auto handleCubeCollision = [this](const CollisionInfo& info)
		{
			if (!info.otherCollider)
				return;
			// Terrain, Stage, Bumpers のいずれかであれば押し戻す
			uint32_t targetLayers = CollisionLayer::Terrain | CollisionLayer::Stage | CollisionLayer::Bumpers;
			if (!(info.otherCollider->GetCollisionLayer() & targetLayers))
				return;
			if (!player_)
				return;

			// 衝突情報（法線とめり込み深さ）から押し戻しベクトルを計算して位置を補正
			Vector3 pos = player_->GetPosition();
			pos += info.normal * info.depth;
			player_->SetPosition(pos);

			// 接地判定と速度リセット
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

		collider->SetOnEnter([this, handleCubeCollision](const CollisionInfo& info)
		{
			// 押し戻し
			handleCubeCollision(info);

			if (!info.otherCollider)
			{
				return;
			}

			// 弾が当たったらHPを減らす
			if (info.otherCollider->GetCollisionLayer() & CollisionLayer::EnemyBullet)
			{
				auto status = player_->GetComponent<StatusComponent>();
				if (status)
				{
					status->SetHp(status->GetHp() - 10);
				
				  if (status->GetHp() <= 0)
					{
						cameraState_ = CameraState::GameOver;
						cameraTimer_ = 0.0f;
					}
				}
			}


		});
		collider->SetOnStay([handleCubeCollision](const CollisionInfo& info)
		{
			// 押し戻し
			handleCubeCollision(info);
		});
		collider->SetOnExit([](const CollisionInfo& info) {});


	}

	// こいつに追従カメラを追従させる
	topDownCamera_->SetPitch(1.2f);
	topDownCamera_->SetOffset({0.0f, 0.0f, -40.0f});
	topDownCamera_->Start(105.0f, &player_->GetPosition());
	// マネージャーに登録
	GameObjectManager::GetInstance()->Register(player_.get());

	// 2. テスト用ターゲットオブジェクトの作成
	targetObject_ = std::make_unique<GameObject>(GameObjectTag::Enemy);
	targetObject_->SetName("TestTarget");
	targetObject_->Initialize(sceneManager_->GetObject3dCommon(), sceneManager_->GetLightManager());
	targetObject_->SetModel("cube");
	targetObject_->SetPosition({5.0f, 2.0f, 0.0f});
	targetObject_->SetScale({2.0f, 2.0f, 2.0f});
	targetObject_->SetColor({1.0f, 0.0f, 0.0f, 1.0f}); // 分かりやすく赤色にする

	// アクション・物理・ステータスコンポーネントの追加
	targetObject_->AddComponent("Status", std::make_unique<StatusComponent>(targetObject_.get()));
	targetObject_->AddComponent("Physics", std::make_unique<PhysicsComponent>(targetObject_.get()));
	targetObject_->GetComponent<PhysicsComponent>()->SetVelocity({0.0f, 0.0f, -5.0f});

	// AABBコライダーの追加
	targetObject_->AddComponent("Collider", std::make_unique<AABBColliderComponent>(targetObject_.get()));
	if (auto collider = targetObject_->GetComponent<AABBColliderComponent>())
	{
		collider->SetCollisionLayer(CollisionLayer::Enemy);
		collider->SetCollisionMask(CollisionLayer::Player | CollisionLayer::PlayerBullet | CollisionLayer::Terrain | CollisionLayer::Bumpers);

		auto handleTargetCollision = [this](const CollisionInfo& info)
		{
			if (!info.otherCollider)
				return;
			if (!(info.otherCollider->GetCollisionLayer() & CollisionLayer::Terrain))
				return;
			if (!targetObject_)
				return;

			// 衝突情報（法線とめり込み深さ）から押し戻しベクトルを計算して位置を補正
			Vector3 pos = targetObject_->GetPosition();
			pos += info.normal * info.depth;
			targetObject_->SetPosition(pos);

			// 接地判定と速度リセット
			auto physics = targetObject_->GetComponent<PhysicsComponent>();
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

		collider->SetOnEnter([handleTargetCollision](const CollisionInfo& info)
		{ handleTargetCollision(info); });
		collider->SetOnStay([handleTargetCollision](const CollisionInfo& info)
		{ handleTargetCollision(info); });
		collider->SetOnExit([](const CollisionInfo& info) {});
	}
	GameObjectManager::GetInstance()->Register(targetObject_.get());

	// 3. 地面キューブオブジェクトの作成
	groundObject_ = std::make_unique<GameObject>(GameObjectTag::Terrain);
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

	// 地面のAABBコライダーの追加
	groundObject_->AddComponent("Collider", std::make_unique<AABBColliderComponent>(groundObject_.get()));
	if (auto collider = groundObject_->GetComponent<AABBColliderComponent>())
	{
		collider->SetCollisionLayer(CollisionLayer::Terrain);
		collider->SetCollisionMask(CollisionLayer::Player | CollisionLayer::Enemy);
	}
	GameObjectManager::GetInstance()->Register(groundObject_.get());

	// 障害物：バンパー
	bumper_ = std::make_unique<GameObject>(GameObjectTag::Bumper);
	bumper_->Initialize(sceneManager_->GetObject3dCommon(), sceneManager_->GetLightManager());
	bumper_->SetName("Bumper");
	bumper_->SetModel("cube");
	bumper_->SetScale({2.0f, 2.0f, 2.0f});
	bumper_->SetPosition({5.0f, 2.0f, -30.0f});

	auto bumperCollider = std::make_unique<AABBColliderComponent>(bumper_.get());
	bumperCollider->SetCollisionLayer(CollisionLayer::Bumpers);
	bumperCollider->SetCollisionMask(CollisionLayer::Player | CollisionLayer::Enemy);

	// 押し戻しと跳ね返りの共通処理
	auto handleBumperCollision = [](const CollisionInfo& info)
	{
		if (!info.otherCollider)
			return;

		auto physics = info.otherCollider->GetOwner()->GetComponent<PhysicsComponent>();
		if (physics)
		{
			// 反対に弾き飛ばす
			Vector3 bounceVelocity = physics->GetMovementVelocity() * -1.0f;
			physics->SetMovementVelocity(bounceVelocity);
		}
	};

	// 衝突した瞬間（OnEnter）に跳ね返り速度を与える
	bumperCollider->SetOnEnter([handleBumperCollision](const CollisionInfo& info)
	{
		handleBumperCollision(info);
	});

	// 衝突中（OnStay）も押し戻しを継続
	bumperCollider->SetOnStay([handleBumperCollision](const CollisionInfo& info)
	{
		handleBumperCollision(info);
	});

	bumper_->AddComponent("Collider", std::move(bumperCollider));

	GameObjectManager::GetInstance()->Register(bumper_.get());

	// ボムエネミーの生成
	InitializeBombEnemy();


	// チャージ敵
	chargeEnemy_ = std::make_unique<GameObject>(GameObjectTag::Enemy);
	chargeEnemy_->Initialize(sceneManager_->GetObject3dCommon(), sceneManager_->GetLightManager());
	chargeEnemy_->SetName("ChargeEnemy");
	chargeEnemy_->SetModel("chargeEnemy");
	chargeEnemy_->SetScale({2.0f, 2.0f, 2.0f});
	chargeEnemy_->SetPosition({-5.0f, 2.0f, -30.0f});

	// アクション・物理・ステータスコンポーネントの追加
	chargeEnemy_->AddComponent("Move", std::make_unique<ChargeMoveComponent>(player_.get()));
	chargeEnemy_->AddComponent("Status", std::make_unique<StatusComponent>(chargeEnemy_.get()));
	chargeEnemy_->AddComponent("Physics", std::make_unique<PhysicsComponent>(chargeEnemy_.get()));
	// 死亡演出コンポーネント
	chargeEnemy_->AddComponent("DeathDirection", std::make_unique<EnemyDeathDirectionComponent>("bullet_hit"));
	chargeEnemy_->AddComponent(
		"UI",
		std::make_unique<UIComponent>(
			chargeEnemy_.get(),
			sceneManager_->GetSpriteCommon(),
			sceneManager_->GetCameraManager()->GetActiveCamera(),
			Vector3(0.0f, 6.0f, 0.0f) // 敵の頭上少し上
			));

	// HPを設定
	auto status = chargeEnemy_->GetComponent<StatusComponent>();
	status->SetHp(5);

	// AABBコライダーの追加
	chargeEnemy_->AddComponent("Collider", std::make_unique<AABBColliderComponent>(chargeEnemy_.get()));
	if (auto collider = chargeEnemy_->GetComponent<AABBColliderComponent>())
	{
		collider->SetCollisionLayer(CollisionLayer::Enemy);
		collider->SetCollisionMask(
			CollisionLayer::PlayerBullet |
			CollisionLayer::Terrain |
			CollisionLayer::Bumpers);

		auto handleTargetCollision = [this](const CollisionInfo& info)
		{
			if (!info.otherCollider)
			{
				return;
			}

			uint32_t layer = info.otherCollider->GetCollisionLayer();

			// Terrain に衝突した場合は押し戻しと接地判定を行う
			if (layer & CollisionLayer::Terrain)
			{
				// 衝突情報（法線とめり込み深さ）から押し戻しベクトルを計算して位置を補正
				Vector3 pos = chargeEnemy_->GetPosition();
				pos += info.normal * info.depth;
				chargeEnemy_->SetPosition(pos);

				// 接地判定と速度リセット
				auto physics = chargeEnemy_->GetComponent<PhysicsComponent>();
				if (physics)
				{
					if (info.normal.y > 0.0f)
					{
						// 接地状態を設定
						physics->SetGrounded(true);

						Vector3 vel = physics->GetExternalVelocity();

						// 下方向の速度をリセット
						if (vel.y < 0.0f)
						{
							vel.y = 0.0f;
							physics->SetExternalVelocity(vel);
						}
					}
				}
			}

			// 弾が当たったらHPを減らす
			if (layer & CollisionLayer::PlayerBullet)
			{
				auto status = chargeEnemy_->GetComponent<StatusComponent>();

				if (status)
				{
					status->SetHp(status->GetHp() - 1);

					// 攻撃を食らったらシェイクする
					auto move = chargeEnemy_->GetComponent<ChargeMoveComponent>();
					if (move && status->GetHp() > 0)
					{
						move->StartShake();
					}
				}
			}
		};

		collider->SetOnEnter([handleTargetCollision](const CollisionInfo& info)
		{
			handleTargetCollision(info);
		});

		collider->SetOnStay([handleTargetCollision](const CollisionInfo& info)
		{
			handleTargetCollision(info);
		});

		collider->SetOnExit([](const CollisionInfo& info) {});
	}

	GameObjectManager::GetInstance()->Register(chargeEnemy_.get());

	// ホーミング敵
	hormingTest_ = std::make_unique<GameObject>(GameObjectTag::Enemy);
	hormingTest_->SetName("HormingTestCube");
	hormingTest_->Initialize(sceneManager_->GetObject3dCommon(), sceneManager_->GetLightManager());
	hormingTest_->SetModel("cube");
	hormingTest_->SetPosition({0.0f, 2.0f, 4.0f});
	hormingTest_->SetScale({2.0f, 2.0f, 2.0f});

	// HPを持たせる
	hormingTest_->AddComponent("Status", std::make_unique<StatusComponent>(hormingTest_.get()));
	if (auto status = hormingTest_->GetComponent<StatusComponent>())
	{
		// 3回当たったら倒れるようにHP3
		status->SetHp(3);
	}

	// 当たり判定を付ける
	hormingTest_->AddComponent("Collider", std::make_unique<AABBColliderComponent>(hormingTest_.get()));
	if (auto collider = hormingTest_->GetComponent<AABBColliderComponent>())
	{
		collider->SetCollisionLayer(CollisionLayer::Enemy);

		// 跳ね返した弾(PlayerBullet)と当たるようにする
		collider->SetCollisionMask(
			CollisionLayer::PlayerBullet |
			CollisionLayer::Terrain |
			CollisionLayer::Bumpers);

		collider->SetOnEnter([this](const CollisionInfo& info)
		{
			if (!info.otherCollider)
			{
				return;
			}

			// 跳ね返した弾に当たった場合
			if (info.otherCollider->GetCollisionLayer() & CollisionLayer::PlayerBullet)
			{
				auto status = hormingTest_->GetComponent<StatusComponent>();
				if (!status)
				{
					return;
				}

				// 1ダメージ
				status->ApplyDamage(1);

				// 弾の削除は HormingMoveComponent 側の KillBullet に任せる
				// ここで info.other->Destroy() はしない
			}
		});

		collider->SetOnStay([](const CollisionInfo& info) {});
		collider->SetOnExit([](const CollisionInfo& info) {});
	}

	// 一定間隔でプレイヤーに向かってホーミング弾を発射する
	hormingTest_->AddComponent("Horming", std::make_unique<HormingMoveComponent>(player_.get()));
	// 死亡演出をつける
	hormingTest_->AddComponent("DeathEffect", std::make_unique<EnemyDeathDirectionComponent>("bullet_hit"));


	GameObjectManager::GetInstance()->Register(hormingTest_.get());
}

void TestScene::InitializeBombEnemy()
{
	bombEnemy_ = std::make_unique<GameObject>(GameObjectTag::Enemy);
	bombEnemy_->SetName("BombEnemy");
	bombEnemy_->Initialize(sceneManager_->GetObject3dCommon(), sceneManager_->GetLightManager());
	bombEnemy_->SetModel("bombenemy");
	bombEnemy_->SetPosition(kBombEnemyPosition);
	bombEnemy_->SetScale(kBombEnemyScale);

	bombEnemy_->AddComponent("Move", std::make_unique<BombMoveComponent>(player_.get()));
	bombEnemy_->AddComponent("Status", std::make_unique<StatusComponent>(bombEnemy_.get()));
	bombEnemy_->AddComponent("Physics", std::make_unique<PhysicsComponent>(bombEnemy_.get()));
	bombEnemy_->AddComponent(
		"UI",
		std::make_unique<UIComponent>(
			bombEnemy_.get(),
			sceneManager_->GetSpriteCommon(),
			sceneManager_->GetCameraManager()->GetActiveCamera(),
			Vector3(0.0f, 6.0f, 0.0f) // 敵の頭上少し上
			));
	bombEnemy_->AddComponent("Collider", std::make_unique<AABBColliderComponent>(bombEnemy_.get()));
	bombEnemy_->AddComponent("ExplosionCollider", std::make_unique<SphereColliderComponent>(bombEnemy_.get()));

	GameObjectManager::GetInstance()->Register(bombEnemy_.get());
}

void TestScene::UpdateCamera()
{
	// カメラの状態に応じて更新処理を切り替える
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

void TestScene::UpdateIntroCamera()
{
	float deltaTime = TimeManager::GetInstance().GetGameContext().deltaTime;

	cameraTimer_ += deltaTime;

	float t = cameraTimer_ / kIntroTime;
	t = std::clamp(t, 0.0f, 1.0f);

	t = EaseOutQuad(t);

	auto camera = sceneManager_->GetCameraManager()->GetActiveCamera();

	// 開始位置
	Vector3 startPos = {0.0f, 130.0f, 80.0f};

	// 終了位置
	Vector3 endPos = player_->GetPosition() + Vector3(0.0f, 90.0f, -40.0f);

	// 補間してカメラの位置を更新
	camera->SetTranslate(MathUtils::Lerp(startPos, endPos, t));

	// 開始回転
	Vector3 startRot = {0.6f, 0.0f, 0.0f};
	Vector3 endRot = {1.2f, 0.0f, 0.0f};

	camera->SetRotate(MathUtils::Lerp(startRot, endRot, t));

	if (cameraTimer_ >= kIntroTime)
	{
		cameraState_ = CameraState::Playing;
		cameraTimer_ = 0.0f;
	}
}

void TestScene::StartClearDirection()
{
	if (!player_ || isClearDirectionStarted_)
	{
		return;
	}

	isClearDirectionStarted_ = true;
	cameraState_ = CameraState::Clear;
	cameraTimer_ = 0.0f;

	auto camera =
		sceneManager_->GetCameraManager()->GetActiveCamera();

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

void TestScene::UpdateClearDirection()
{
	if (!player_)
	{
		return;
	}

	const float deltaTime =
		TimeManager::GetInstance().GetGameContext().deltaTime;

	cameraTimer_ += deltaTime;

	auto camera =
		sceneManager_->GetCameraManager()->GetActiveCamera();

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
	// カメラはプレイヤー側を向く
	const Vector3 clearCameraPosition =
		clearPlayerBasePosition_ +
		playerForward * kClearCameraDistance +
		Vector3{0.0f, kClearCameraHeight, 0.0f};

	const Vector3 clearCameraRotation = {
		0.2f,
		playerYaw + std::numbers::pi_v<float>,
		0.0f};

	// ─────────────────────────────
	// 前半：カメラをプレイヤー正面へ移動
	// ─────────────────────────────
	if (cameraTimer_ <= kClearCameraMoveTime)
	{
		float t =
			cameraTimer_ / kClearCameraMoveTime;

		t = std::clamp(t, 0.0f, 1.0f);

		// 徐々に減速しながら正面へ移動
		const float easedT = EaseOutQuad(t);

		camera->SetTranslate(
			MathUtils::Lerp(
				clearStartCameraPosition_,
				clearCameraPosition,
				easedT));

		camera->SetRotate(
			MathUtils::Lerp(
				clearStartCameraRotation_,
				clearCameraRotation,
				easedT));

		return;
	}

	// カメラは正面位置で固定
	camera->SetTranslate(clearCameraPosition);
	camera->SetRotate(clearCameraRotation);

	// ─────────────────────────────
	// 後半：プレイヤーが回転しながらジャンプ
	// ─────────────────────────────
	const float actionElapsed =
		cameraTimer_ - kClearCameraMoveTime;

	float actionT =
		actionElapsed / kClearPlayerActionTime;

	actionT = std::clamp(actionT, 0.0f, 1.0f);

	// 0 → 1 → 0になる放物線
	const float jumpRate =
		4.0f * actionT * (1.0f - actionT);

	Vector3 playerPosition =
		clearPlayerBasePosition_;

	playerPosition.y +=
		kClearJumpHeight * jumpRate;

	// Y軸を1回転
	Vector3 playerRotation =
		clearPlayerBaseRotation_;

	playerRotation.y +=
		2.0f *
		std::numbers::pi_v<float> *
		actionT;

	player_->SetPosition(playerPosition);
	player_->SetRotation(playerRotation);

	// 演出終了時
	if (actionT >= 1.0f)
	{
		// 地面と回転を元の状態へ正確に戻す
		player_->SetPosition(clearPlayerBasePosition_);
		player_->SetRotation(clearPlayerBaseRotation_);

		// 仮実装なので、終了後もクリア画面のカメラ位置で停止
		cameraTimer_ =
			kClearCameraMoveTime +
			kClearPlayerActionTime;
	}
}

void TestScene::UpdateFollowCamera()
{
	topDownCamera_->Update();

}


void TestScene::UpdateGameOverCamera()
{
	float deltaTime = TimeManager::GetInstance().GetGameContext().deltaTime;

	cameraTimer_ += deltaTime;
	
	float t = cameraTimer_ / kGameOverTime;
	t = std::clamp(t, 0.0f, 1.0f);

	t = EaseOutQuad(t);

	auto camera = sceneManager_->GetCameraManager()->GetActiveCamera();

	// 開始位置と終了位置を設定
	Vector3 startPos = player_->GetPosition() + Vector3(0, 90, -40);
	Vector3 endPos = player_->GetPosition() + Vector3(0, 125, -55);

	camera->SetTranslate(MathUtils::Lerp(startPos, endPos, t));

	Vector3 startRot = {1.2f, 0, 0};
	Vector3 endRot = {0.8f, 0, 0};

	camera->SetRotate(MathUtils::Lerp(startRot, endRot, t));
}

void TestScene::GameOverDirection()
{
	auto post = sceneManager_->GetPostProcessManager();

	// エフェクト自体を有効化
	post->crtEffect_->SetEnabled(true);
	// CRTエフェクト自体を有効化
	post->crtEffect_->SetCrtEnabled(true);
	// 色収差(RGBシフト)を有効化
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

void TestScene::OnFinalize()
{
	// 登録されたオブジェクトの登録解除とクリア
	GameObjectManager::GetInstance()->Finalize();
	CollisionManager::GetInstance()->Finalize();
	if (GameObjectEditor::HasInstance())
	{
		GameObjectEditor::GetInstance()->Finalize();
	}
#ifdef USE_IMGUI
	if (DebugUIManager::HasInstance())
	{
		DebugUIManager::GetInstance()->UnregisterDebugUI(this);
	}
#endif
	player_.reset();
	groundObject_.reset();
	targetObject_.reset();
	debugCamera_.reset();
	topDownCamera_.reset();
}

void TestScene::CommonUpdate()
{
	static bool isDebugCameraActive = false;

	if (Input::GetInstance()->TriggerKey(DIK_F7))
	{
		isDebugCameraActive = !isDebugCameraActive;
	}

	// 通常プレイ中にCキーを押したら開始
	if (!isDebugCameraActive &&
		cameraState_ == CameraState::Playing &&
		Input::GetInstance()->TriggerKey(DIK_C))
	{
		StartClearDirection();
	}

	if (isDebugCameraActive)
	{
		debugCamera_->Update();
	}
	else
	{
		UpdateCamera();
	}

	if (cameraState_ != CameraState::Playing)
	{
		return;
	}

	// コリジョンマネージャーの前フレーム位置更新
	CollisionManager::GetInstance()->UpdatePreviousPositions();

	// ゲームオブジェクトマネージャーの更新
	GameObjectManager::GetInstance()->Update();

	// 衝突判定の実行
	CollisionManager::GetInstance()->CheckCollisions();
}

void TestScene::Draw3D()
{
	// ゲームオブジェクトの3D描画
	GameObjectManager::GetInstance()->Draw3D(sceneManager_->GetCameraManager());
}

void TestScene::Draw2D()
{
	// ゲームオブジェクトの2D描画
	GameObjectManager::GetInstance()->Draw2D();
}

#ifdef USE_IMGUI
void TestScene::DrawImGui()
{
}
#endif

void TestScene::DrawShadow()
{
	// ゲームオブジェクトのシャドウ描画
	GameObjectManager::GetInstance()->DrawShadow(sceneManager_->GetCameraManager()->GetActiveCamera());
}

void TestScene::DrawGBuffer()
{
	// ゲームオブジェクトのGBuffer描画
	GameObjectManager::GetInstance()->DrawGBuffer(sceneManager_->GetCameraManager());
}
