#include "ChargeMoveComponent.h"

#include "../../common/PhysicsComponent.h"
#include "application/collision/CollisionLayer.h"
#include "application/gameobject/component/action/common/StatusComponent.h"
#include "application/gameobject/component/action/player/PlayerReflectComponent.h"
#include "engine/gameobject/base/GameObject.h"
#include "engine/gameobject/component/collision/AABBColliderComponent.h"
#include "engine/gameobject/component/collision/CollisionManager.h"
#include "engine/gameobject/manager/GameObjectManager.h"
#include "engine/time/TimeManager.h"

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
	Vector3 directionToPlayer = Vector3::Normalize(playerPosition - owner->GetPosition());

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
		Vector3 rotation = owner->GetRotation();
		rotation.y += rotationSpeed_ * TimeManager::GetInstance().GetGameContext().deltaTime;
		owner->SetRotation(rotation);
	}
}

void GameObjectComponent::ChargeMoveComponent::Cooldown(GameObject* owner)
{
	// クールタイム進行
	coolTime_ += TimeManager::GetInstance().GetGameContext().deltaTime;

	StrafeMove(owner);

	// クールタイムが一定時間を超えたら攻撃状態を解除
	if (coolTime_ >= kCoolTime)
	{
		isAttacking_ = false;
		coolTime_ = 0.0f;

		
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
	// 弾生成
	bulletSpawnComponent_ = std::make_unique<BulletSpawnComponent>();
	// 弾を生成
	auto bullet = bulletSpawnComponent_->Fire("Bullet", owner->GetPosition(), owner->GetRotation());

	// 挙動のコンポーネント
	bullet->AddComponent("Behavior", std::make_unique<BulletBehaviorComponent>(4.0f));
	
	//　物理コンポーネントの追加
	auto physics = std::make_unique<PhysicsComponent>(bullet);
	physics->SetUseGravity(false);
	physics->SetMovementVelocity(bulletDirection_);
	bullet->AddComponent("Physics", std::move(physics));

	// AABBコライダーの追加
	bullet->AddComponent("Collider", std::make_unique<AABBColliderComponent>(bullet));
	if (auto collider = bullet->GetComponent<AABBColliderComponent>())
	{
		collider->SetCollisionLayer(CollisionLayer::EnemyBullet);
		collider->SetCollisionMask(CollisionLayer::Player | CollisionLayer::Bumpers | CollisionLayer::PlayerReflect);

		// 弾自身のコールバックなので、ここでは弾の反射または破棄だけを行う。
		collider->SetOnEnter([bullet](const CollisionInfo& info)
		{
			// マスクのレイヤーに衝突した場合、弾を破壊する
			if (!info.otherCollider)
			{
				return;
			}

			if (info.other &&
				(info.otherCollider->GetCollisionLayer() & CollisionLayer::PlayerReflect))
			{
				// 行き先はプレイヤーから取得し、速度とレイヤー変更は弾自身のAPIへ任せる。
				auto reflect = info.other->GetComponent<PlayerReflectComponent>();
				auto behavior = bullet->GetComponent<BulletBehaviorComponent>();
				auto bulletPhysics = bullet->GetComponent<PhysicsComponent>();
				if (!reflect || !behavior || !bulletPhysics)
				{
					return;
				}

				// 現在速度の大きさを保ったまま、反射時に確定した方向へ向け直す。
				const Vector3 direction = reflect->GetReflectDirectionFrom(bullet->GetPosition());
				const float speed = bulletPhysics->GetMovementVelocity().Length();
				behavior->Reflect(bullet, direction, speed);
				return;
			}

			// プレイヤー、敵、バンパーに衝突した場合、弾を破壊する
			if (info.otherCollider->GetCollisionLayer() == CollisionLayer::Player ||
				info.otherCollider->GetCollisionLayer() == CollisionLayer::Enemy ||
				info.otherCollider->GetCollisionLayer() == CollisionLayer::Bumpers)
			{
				bullet->Destroy();
			}
		});
		collider->SetOnStay([](const CollisionInfo&) {});
		collider->SetOnExit([](const CollisionInfo&) {});
	}
}

void GameObjectComponent::ChargeMoveComponent::StrafeMove(GameObject* owner)
{
	// タイマー更新
	strafeTimer_ += TimeManager::GetInstance().GetGameContext().deltaTime;

	// 確認用のサイズを元に戻す
	owner->SetScale(Vector3{2.0f, 2.0f, 2.0f});

	if (strafeTimer_ >= changeTime_)
	{
		moveRight_ = Random(0, 1) > 0.3f;

		changeTime_ = Random(1.0f, 2.5f);
		currentStrafeSpeed_ = Random(4.5f, 6.5f);

		strafeTimer_ = 0.0f;
	}

	// プレイヤーへの方向
	Vector3 toPlayer = player_->GetPosition() - owner->GetPosition();
	float distance = toPlayer.Length();
	toPlayer.NormalizeSelf();

	// 横方向
	Vector3 side = {-toPlayer.z, 0.0f, toPlayer.x};
	Vector3 movement = side;
	movement += toPlayer * Random(-0.3f, 0.3f);

	movement.NormalizeSelf();


	// 左右移動
	if (!moveRight_)
	{
		side *= -1.0f;
	}

	owner->SetRotation(Vector3{0.0f, atan2f(toPlayer.x, toPlayer.z), 0.0f});
	owner->SetPosition(owner->GetPosition() + side * currentStrafeSpeed_ * TimeManager::GetInstance().GetGameContext().deltaTime);
}

float GameObjectComponent::ChargeMoveComponent::Random(float min, float max)
{
	return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}
