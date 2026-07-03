#include "BombMoveComponent.h"
#include "../../common/PhysicsComponent.h"
#include "engine/gameobject/base/GameObject.h"
#include "engine/time/TimeManager.h"


GameObjectComponent::BombMoveComponent::BombMoveComponent(GameObject* player)
	: player_(player)
{
	
}


void GameObjectComponent::BombMoveComponent::Update(GameObject* owner)
{
	// プレイヤーの情報がない場合は処理を中断
	if (player_ == nullptr)return;

	// フィジクスコンポーネントの取得
	physics_ = owner->GetComponent<PhysicsComponent>().get();

	// デルタタイムの取得
	//float dt = TimeManager::GetInstance().GetGameContext().deltaTime;

	// プレイヤーの座標を更新
	playerPosition_ = player_->GetPosition();

	// 自分（ボム）からプレイヤーへのベクトルと距離
	Vector3 toPlayer = playerPosition_ - owner->GetPosition();
	float distance = toPlayer.Length();

	// 突進開始判定（まだ突進していない & 突進距離に入った）
	if (!isDashing_ && distance <= dashRange_)
	{
		isDashing_ = true;
		dashDirection_ = toPlayer.Normalize();
	}
	
	if (lifespan_ >= 0.0f && isDashing_)
	{
		
		if (distance <= chaseRange_)
		{
			// 索敵範囲内：毎フレーム方向を更新しながら追尾
			physics_->SetMovementVelocity(toPlayer.Normalize() * chaseSpeed_);
			--lifespan_;
		}
		else
		{
			// 範囲外：停止
			physics_->SetMovementVelocity({0.0f, 0.0f, 0.0f});
			--lifespan_;
		}
	}
	else
	{
		// 範囲外：停止
		physics_->SetMovementVelocity({0.0f, 0.0f, 0.0f});
	}
	
	
}