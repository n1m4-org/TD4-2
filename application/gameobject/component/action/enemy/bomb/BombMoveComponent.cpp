#include "BombMoveComponent.h"
#include "../../common/PhysicsComponent.h"
#include "engine/effects/particle/ParticleManager.h"
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
	
	// 爆発済みなら停止
	if (hasExploded_)
	{
		physics_->SetMovementVelocity({0.0f, 0.0f, 0.0f});
		return;
	}

	// フィジクスコンポーネントの取得
	physics_ = owner->GetComponent<PhysicsComponent>().get();

	// プレイヤーの座標を更新
	playerPosition_ = player_->GetPosition();

	// 自分（ボム）からプレイヤーへのベクトルと距離
	Vector3 toPlayer = Vector3::Normalize(playerPosition_ - owner->GetPosition());
	float distance = toPlayer.Length();


	// 突進開始判定（まだ突進していない & 突進距離に入った）
	if (!isDashing_ && distance <= dashRange_)
	{
		isDashing_ = true;
	}

	if (isDashing_)
	{
		// 突進中のみライフスパンを消費
		--lifespan_;

		if (lifespan_ <= 0.0f)
		{
			// 時間切れで爆発（1回だけ）
			physics_->SetMovementVelocity({0.0f, 0.0f, 0.0f});
			ParticleManager::GetInstance()->Play("bomber", owner->GetPosition());
			hasExploded_ = true;
			return;
		}

		// 突進中も毎フレーム方向を更新して追尾し続ける
		physics_->SetMovementVelocity(toPlayer.Normalize() * dashSpeed_);
	}
	else if (distance <= chaseRange_)
	{
		// 索敵範囲内：追尾
		physics_->SetMovementVelocity(toPlayer.Normalize() * chaseSpeed_);
	}
	else
	{
		// 範囲外：待機
		physics_->SetMovementVelocity({0.0f, 0.0f, 0.0f});
	}
}