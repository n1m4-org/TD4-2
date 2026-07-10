#include "ChargeMoveComponent.h"

#include "engine/gameobject/base/GameObject.h"
#include "engine/time/TimeManager.h"
#include "../../common/PhysicsComponent.h"
#include "application/gameobject/component/action/common/StatusComponent.h"
#include "engine/gameobject/manager/GameObjectManager.h"
#include "application/collision/CollisionLayer.h"
#include "engine/gameobject/component/collision/AABBColliderComponent.h"
#include "engine/gameobject/component/collision/CollisionManager.h"

#include "../bullet/BulletBehaviorComponent.h"

GameObjectComponent::ChargeMoveComponent::ChargeMoveComponent(GameObject* _player)
	: player_(_player)
{
}

void GameObjectComponent::ChargeMoveComponent::Update(GameObject* owner)
{
	// プレイヤーが存在しない場合は処理を中断
	if (!player_)
	{
		return;
	}

	// 物理コンポーネントを取得
	physics_ = owner->GetComponent<PhysicsComponent>().get();

	 switch (state_)
	{
	case State::Move:
		Move(owner);
		break;
		
	case State::Charge:
		Charge(owner);
		break;

	case State::Fire:
		Fire(owner);
		break;

	case State::Cooldown:
		Cooldown(owner);
		break;
	}

}

void GameObjectComponent::ChargeMoveComponent::Move(GameObject* owner)
{
	// deltaTimeを取得
	float deltaTime = TimeManager::GetInstance().GetGameContext().deltaTime;

	// プレイヤーの位置を取得
	Vector3 playerPosition = player_->GetPosition();

	// プレイヤーへの向きを取得
	Vector3 directionToPlayer = playerPosition - owner->GetPosition();

	// 正規化
	directionToPlayer.NormalizeSelf();

	// 移動量を計算
	Vector3 movement = directionToPlayer * moveSpeed_ * deltaTime;

	// オブジェクトを移動
	owner->SetPosition(owner->GetPosition() + movement);

	// プレイヤーの方向を取得
	Vector3 lookDirection = directionToPlayer;
	// 正規化
	lookDirection.NormalizeSelf();
	// 回転を計算（Y軸回転のみ）
	float yaw = atan2f(lookDirection.x, lookDirection.z);
	// 回転を設定
	owner->SetRotation(Vector3{0.0f, yaw, 0.0f});


	// プレイヤーとの距離を計算
	Vector3 directionToPlayerForDistance = playerPosition - owner->GetPosition();
	// 距離を取得
	float distanceToPlayer = directionToPlayerForDistance.Length();
	// プレイヤーとの距離が一定距離以内ならチャージ開始
	if (distanceToPlayer <= chargeStartDistance_)
	{
		isChargeStart_ = true;
		chargeTime_ = 0.0f;

		state_ = State::Charge;
	}
	
}

void GameObjectComponent::ChargeMoveComponent::Charge(GameObject* owner)
{
	// フラグが立っている場合、チャージ時間を加算
	if (isChargeStart_)
	{
		// チャージ時間を加算
		chargeTime_ += TimeManager::GetInstance().GetGameContext().deltaTime;
		// チャージ時間が一定時間を超えたら攻撃状態に移行
		if (chargeTime_ >= kChargeTime)
		{
			isAttacking_ = true;
			isChargeStart_ = false;
			chargeTime_ = 0.0f;

			state_ = State::Fire;
		}

		// 確認用回転させる
		owner->SetRotation(owner->GetRotation() + Vector3{0.0f, 1.0f, 0.0f});

	}
	//else
	//{
	//	// プレイヤーとの距離を計算
	//	Vector3 playerPosition = player_->GetPosition();
	//	Vector3 directionToPlayer = playerPosition - owner->GetPosition();
	//	// 距離を取得
	//	float distanceToPlayer = directionToPlayer.Length();
	//	// プレイヤーとの距離が一定距離以内ならチャージ開始
	//	if (distanceToPlayer <= chargeStartDistance_)
	//	{
	//		isChargeStart_ = true;
	//		chargeTime_ = 0.0f;
	//	}
	//}

}

void GameObjectComponent::ChargeMoveComponent::Cooldown(GameObject* owner)
{
	// クールタイム進行
	coolTime_ += TimeManager::GetInstance().GetGameContext().deltaTime;

	// クールタイムが一定時間を超えたら攻撃状態を解除
	if (coolTime_ >= kCoolTime)
	{
		isAttacking_ = false;
		coolTime_ = 0.0f;

		// 確認用のサイズを元に戻す
		owner->SetScale(Vector3{2.0f, 2.0f, 2.0f});
		state_ = State::Move;
	}
}

void GameObjectComponent::ChargeMoveComponent::Fire(GameObject* owner)
{
	if (isAttacking_)
	{
		// プレイヤーへの向きを取得
		Vector3 playerPosition = player_->GetPosition();
		bulletDirection_ = playerPosition - owner->GetPosition();

		// 弾生成
		BulletInitialize(owner);

		// 一旦サイズをでかくする
		owner->SetScale(Vector3{4.0f, 4.0f, 4.0f});
		state_ = State::Cooldown;

	}
}

void GameObjectComponent::ChargeMoveComponent::BulletInitialize(GameObject* owner)
{

	bullet_ = GameObjectManager::GetInstance()->CreateGameObject("Bullet", "Bullet");
	bullet_->SetName("Bullet");
	bullet_->SetModel("cube");
	bullet_->SetScale({1.0f, 1.0f, 1.0f});
	bullet_->SetPosition(owner->GetPosition());
	bullet_->SetRotation(owner->GetRotation());

	bullet_->AddComponent("Behavior", std::make_unique<BulletBehaviorComponent>(bulletDirection_, 3.0f));
	bullet_->AddComponent("Status", std::make_unique<StatusComponent>(bullet_));

	// AABBコライダーの追加
	bullet_->AddComponent("Collider", std::make_unique<AABBColliderComponent>(bullet_));
	if (auto collider = bullet_->GetComponent<AABBColliderComponent>())
	{
		collider->SetCollisionLayer(CollisionLayer::EnemyBullet);
		collider->SetCollisionMask(CollisionLayer::Player | CollisionLayer::Stage | CollisionLayer::Terrain | CollisionLayer::Bumpers);

		// 衝突時の共通押し戻し・接地処理
		auto handleCubeCollision = [this](const CollisionInfo& info)
		{
			if (!info.otherCollider)
				return;
			// Terrain, Stage, Bumpers のいずれかであれば押し戻す
			uint32_t targetLayers = CollisionLayer::Terrain | CollisionLayer::Stage | CollisionLayer::Bumpers;
			if (!(info.otherCollider->GetCollisionLayer() & targetLayers))
				return;
			if (!bullet_)
				return;

			// 衝突情報（法線とめり込み深さ）から押し戻しベクトルを計算して位置を補正
			Vector3 pos = bullet_->GetPosition();
			pos += info.normal * info.depth;
			bullet_->SetPosition(pos);

			// 接地判定と速度リセット
			auto physics = bullet_->GetComponent<PhysicsComponent>();
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
			handleCubeCollision(info);

			// 相手がPlayerの場合にHPを減らす
			if (!info.otherCollider) return;
			if (!(info.otherCollider->GetCollisionLayer() & CollisionLayer::Player)) return;


			auto status = bullet_->GetComponent<StatusComponent>();
			if (!status) return;

			int32_t prevHp = status->GetHp();
			if (status->ApplyDamage(10))
			{
				Logger::Log("Bullet Damaged! HP: " + std::to_string(prevHp) + " -> " + std::to_string(status->GetHp()) + "\n");
			} });
		collider->SetOnStay([handleCubeCollision](const CollisionInfo& info)
							{ handleCubeCollision(info); });
		collider->SetOnExit([](const CollisionInfo& info) {});
	}

	// マネージャーに登録
	GameObjectManager::GetInstance()->Register(bullet_);

}
