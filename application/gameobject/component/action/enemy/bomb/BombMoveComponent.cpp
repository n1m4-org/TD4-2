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
#pragma region 変数の初期化
	// プレイヤーの情報がない場合は処理を中断
	if (player_ == nullptr)
		return;

	// 爆発済みなら停止
	if (hasExploded_)
	{
		physics_->SetMovementVelocity({0.0f, 0.0f, 0.0f});
		return;
	}

	// リフレクト後の寿命を初期化
	reflectedLifespan_ = lifespan_;
#pragma endregion 変数の初期化

	// フィジクスコンポーネントの取得
	physics_ = owner->GetComponent<PhysicsComponent>().get();

	// デルタタイムの取得
	// float dt = TimeManager::GetInstance().GetGameContext().deltaTime;

	// プレイヤーの座標を更新
	playerPosition_ = player_->GetPosition();

	// 自分（ボム）からプレイヤーへのベクトルと距離
	Vector3 toPlayer = playerPosition_ - owner->GetPosition();
	float distance = toPlayer.Length();


	// 索敵範囲内に入ったら追尾開始（＝点火）
	if (!isDashing_ && distance <= chaseRange_)
	{
		isDashing_ = true;
	}

	if (isDashing_)
	{
		// 点火中はライフスパンを消費
		--lifespan_;

		if (lifespan_ <= 0.0f)
		{
			// 寿命切れで爆発（1回だけ）
			physics_->SetMovementVelocity({0.0f, 0.0f, 0.0f});
			ParticleManager::GetInstance()->Play("bomber", owner->GetPosition());
			hasExploded_ = true;
			return;
		}

		// 点火中も毎フレーム方向を更新して追尾し続ける
		physics_->SetMovementVelocity(toPlayer.Normalize() * chaseSpeed_);
	}
	// 索敵範囲外で突進中でない場合は待機
	else if (!isDashing_ && !isReflected_)
	{
		// 索敵範囲外：待機
		physics_->SetMovementVelocity({0.0f, 0.0f, 0.0f});
	}

	// ここに反射されたらフラグを立てるコードを追加
	// isReflected_ = 

	// プレイヤーからの反射情報を取得する
	// reflectedDirection_ = 
	// reflectedVelocity_ =

	// リフレクト後の処理
	if (isReflected_)
	{
		// 移動不可
		isDashing_ = false;

		// ライフスパンを消費
		--reflectedLifespan_;

		if (reflectedLifespan_ <= 0.0f)
		{
			// 寿命切れで爆発
			physics_->SetMovementVelocity({0.0f, 0.0f, 0.0f});
			ParticleManager::GetInstance()->Play("bomber", owner->GetPosition());
			hasExploded_ = true;
			return;
		}

	}
}