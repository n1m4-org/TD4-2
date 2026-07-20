#include "BombMoveComponent.h"
#include "../../common/PhysicsComponent.h"
#include "engine/effects/particle/ParticleManager.h"
#include "engine/gameobject/base/GameObject.h"
#include "engine/time/TimeManager.h"
#include <cmath>


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

#pragma endregion 変数の初期化

	// フィジクスコンポーネントの取得
	physics_ = owner->GetComponent<PhysicsComponent>().get();

	// プレイヤーの座標を更新
	playerPosition_ = player_->GetPosition();

	// 自分（ボム）からプレイヤーへのベクトルと距離
	Vector3 toPlayer = playerPosition_ - owner->GetPosition();
	float distance = toPlayer.Length();
	toPlayer.NormalizeSelf();


	// 索敵範囲内に入ったら追尾開始（＝点火）
	if (!isDashing_ && distance <= chaseRange_)
	{
		isDashing_ = true;
		dashDirection_ = toPlayer; 
	}

	if (isDashing_)
	{
		// 点火中はライフスパンを消費
		--lifespan_;

		if (lifespan_ <= 0.0f)
		{
			// 寿命切れで爆発（1回だけ）
			Explode(owner);
			return;
		}

		// 進行方向を毎フレーム少しずつプレイヤー方向へ寄せる（ドリフト挙動）
		dashDirection_ += (toPlayer - dashDirection_) * turnRate_;
		dashDirection_.NormalizeSelf();
		physics_->SetMovementVelocity(dashDirection_ * chaseSpeed_);
	}
	// 点火中・反射後は残り時間に応じて赤点滅（残りが少ないほど速く）
	if (isDashing_ || isReflected_)
	{
		// 残り時間の割合（1→0）
		float remain = (isReflected_ ? reflectedLifespan_ : lifespan_) / ignitionTime_;

		// 残りが減るほど位相の進みを速くする
		blinkPhase_ += blinkSpeedMin_ + (blinkSpeedMax_ - blinkSpeedMin_) * (1.0f - remain);

		// sinが正の間だけ赤くする（パキッと切り替わる点滅）
		bool redOn = std::sin(blinkPhase_) > 0.0f;
		owner->SetColor(redOn ? Vector4{1.0f, 0.2f, 0.2f, 1.0f}
							  : Vector4{1.0f, 1.0f, 1.0f, 1.0f});
	}
	// 索敵範囲外で突進中でない場合は待機
	else if (!isDashing_ && !isReflected_)
	{
		// 索敵範囲外：待機
		physics_->SetMovementVelocity({0.0f, 0.0f, 0.0f});
	}

	// リフレクト後の処理
	if (isReflected_)
	{
		// 一旦反対方向
		physics_->SetMovementVelocity(reflectedVelocity_);

		// ライフスパン消費
		--reflectedLifespan_;

		// 移動不可
		isDashing_ = false;

		if (reflectedLifespan_ <= 0.0f)
		{
			// 寿命切れで爆発
			Explode(owner);
			return;
		}
	}

	// 寿命が尽きたら消す
	if (lifespan_ <= 0.0f && !hasExploded_)
	{
		Explode(owner);
		return;
	}

	// 進行方向を正面に向かせる（動いている時だけ）
	Vector3 vel = physics_->GetMovementVelocity();
	if (vel.x * vel.x + vel.z * vel.z > 0.0001f)
	{
		float yaw = std::atan2(vel.x, vel.z);
		owner->SetRotation({0.0f, yaw, 0.0f});
	}
}

void GameObjectComponent::BombMoveComponent::Explode(GameObject* owner)
{
	physics_->SetMovementVelocity({0.0f, 0.0f, 0.0f});
	ParticleManager::GetInstance()->Play("bomber", owner->GetPosition());
	hasExploded_ = true;
	owner->SetActive(false); 
}
