#include "TestScene.h"
#include "application/collision/CollisionLayer.h"
#include "application/gameobject/component/action/common/PhysicsComponent.h"
#include "application/gameobject/component/action/common/StatusComponent.h"
#include "application/gameobject/component/action/common/UIComponent.h"
#include "application/gameobject/component/action/enemy/bomb/BombMoveComponent.h"
#include "application/gameobject/component/action/enemy/charge/ChargeMoveComponent.h"
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
#include "engine/graphics/3d/Object3dCommon.h"
#include "externals/imgui/imgui.h"
#include "input/Input.h"
#include "manager/editor/GameObjectEditor.h"
#include "manager/scene/CameraManager.h"
#include "manager/scene/LightManager.h"
#include "scene/manager/SceneManager.h"

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
	reflectCollider->SetSizeOffset({ 2.0f, 2.0f, 2.0f });
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
	chargeEnemy_->SetModel("cube");
	chargeEnemy_->SetScale({2.0f, 2.0f, 2.0f});
	chargeEnemy_->SetPosition({-5.0f, 2.0f, -30.0f});

	// アクション・物理・ステータスコンポーネントの追加
	chargeEnemy_->AddComponent("Move", std::make_unique<ChargeMoveComponent>(player_.get()));
	chargeEnemy_->AddComponent("Status", std::make_unique<StatusComponent>(chargeEnemy_.get()));
	chargeEnemy_->AddComponent("Physics", std::make_unique<PhysicsComponent>(chargeEnemy_.get()));
	chargeEnemy_->AddComponent(
		"UI",
		std::make_unique<UIComponent>(
			chargeEnemy_.get(),
			sceneManager_->GetSpriteCommon(),
			sceneManager_->GetCameraManager()->GetActiveCamera(),
			Vector3(0.0f, 6.0f, 0.0f) // 敵の頭上少し上
		));

	// AABBコライダーの追加
	chargeEnemy_->AddComponent("Collider", std::make_unique<AABBColliderComponent>(chargeEnemy_.get()));
	if (auto collider = chargeEnemy_->GetComponent<AABBColliderComponent>())
	{
		collider->SetCollisionLayer(CollisionLayer::Enemy);
		collider->SetCollisionMask(CollisionLayer::PlayerBullet | CollisionLayer::Terrain | CollisionLayer::Bumpers);

		auto handleTargetCollision = [this](const CollisionInfo& info)
		{
			if (!info.otherCollider)
				return;
			if (!(info.otherCollider->GetCollisionLayer() & CollisionLayer::Terrain))
				return;
			if (!targetObject_)
				return;

			// 衝突情報（法線とめり込み深さ）から押し戻しベクトルを計算して位置を補正
			Vector3 pos = chargeEnemy_->GetPosition();
			pos += info.normal * info.depth;
			chargeEnemy_->SetPosition(pos);

			// 接地判定と速度リセット
			auto physics = chargeEnemy_->GetComponent<PhysicsComponent>();
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

		// ホーミングテスト用キューブオブジェクトの作成
		hormingTest_ = std::make_unique<GameObject>(GameObjectTag::Enemy);
		hormingTest_->SetName("HormingTestCube");
		hormingTest_->Initialize(sceneManager_->GetObject3dCommon(), sceneManager_->GetLightManager());
		hormingTest_->SetModel("cube");
		hormingTest_->SetPosition({0.0f, 2.0f, 4.0f});
		hormingTest_->SetScale({2.0f, 2.0f, 2.0f});

		// Hキーで cubeObject_ の位置へスプライン移動する
		hormingTest_->AddComponent("Horming", std::make_unique<HormingMoveComponent>(player_.get()));
		collider->SetOnEnter([handleTargetCollision](const CollisionInfo& info)
		{ handleTargetCollision(info); });
		collider->SetOnStay([handleTargetCollision](const CollisionInfo& info)
		{ handleTargetCollision(info); });
		collider->SetOnExit([](const CollisionInfo& info) {});
	}
	GameObjectManager::GetInstance()->Register(chargeEnemy_.get());

	GameObjectManager::GetInstance()->Register(hormingTest_.get());

	StartState(SceneState::Playing);
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

void TestScene::Finalize()
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

void TestScene::OnUpdatePlaying()
{
	static bool isDebugCameraActive = false;

	if (Input::GetInstance()->TriggerKey(DIK_F7))
	{
		isDebugCameraActive = !isDebugCameraActive;
	}

	if (isDebugCameraActive)
	{
		debugCamera_->Update();
	}
	else
	{
		topDownCamera_->Update();
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
