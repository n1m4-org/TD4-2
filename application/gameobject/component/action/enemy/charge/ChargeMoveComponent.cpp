#include "ChargeMoveComponent.h"

#include "engine/gameobject/base/GameObject.h"
#include "engine/time/TimeManager.h"
#include "../../common/PhysicsComponent.h"

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
		// 一旦サイズをでかくする
		owner->SetScale(Vector3{4.0f, 4.0f, 4.0f});
		state_ = State::Cooldown;
	}
}
