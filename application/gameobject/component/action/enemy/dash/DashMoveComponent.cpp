#include "DashMoveComponent.h"

#include "collision/CollisionLayer.h"
#include "effects/particle/ParticleManager.h"
#include "gameobject/base/GameObject.h"
#include "gameobject/component/action/common/PhysicsComponent.h"
#include "gameobject/component/action/common/TrailComponent.h"
#include "gameobject/component/action/common/StatusComponent.h"
#include "gameobject/component/action/player/PlayerReflectComponent.h"
#include "gameobject/component/base/ICollisionComponent.h"
#include "gameobject/GameObjectTag.h"
#include "time/TimeManager.h"
#include "audio/Audio.h"

namespace
{
	constexpr float kDirectionEpsilonSq = 0.000001f;
}

namespace GameObjectComponent
{

	DashMoveComponent::DashMoveComponent(GameObject* _player)
		:player_(_player)
	{
	}

	void DashMoveComponent::Update(GameObject* _owner)
	{

		if (!player_ || !_owner) return;

		InitializeComponents(_owner);
		if (!physics_ || !collider_) return;

		switch (state_)
		{
		case State::Idle:
			Idle(_owner);
			break;
		case State::Dash:
			Dash(_owner);
			break;
		case State::Reflected:
			UpdateReflected(TimeManager::GetInstance().GetGameContext().deltaTime);
			break;
		}

	}

	void DashMoveComponent::InitializeComponents(GameObject* _owner)
	{
		if (owner_ && physics_ && collider_)
		{
			return;
		}

		owner_ = _owner;
		physics_ = _owner->GetComponent<PhysicsComponent>().get();
		collider_ = _owner->GetComponent<ICollisionComponent>().get();
		if (!collider_)
		{
			return;
		}

		collider_->SetCollisionLayer(CollisionLayer::Enemy);
		// プレイヤーの反射判定・弾も検知できるようにする
		collider_->SetCollisionMask(
			CollisionLayer::Player |
			CollisionLayer::PlayerReflect |
			CollisionLayer::PlayerBullet |
			CollisionLayer::Terrain |
			CollisionLayer::Bumpers);

		// コールバックではダッシュ敵自身の状態と移動だけを変更する。
		collider_->SetOnEnter([this](const CollisionInfo& info)
		{
			HandleCollision(info);
		});
		collider_->SetOnStay([this](const CollisionInfo& info)
		{
			ResolveTerrainCollision(info);
		});
		collider_->SetOnExit([](const CollisionInfo&) {});
	}

	bool DashMoveComponent::Reflect(const Vector3& direction)
	{
		// ダッシュ中の敵だけ跳ね返せる。多重反射も拒否する。
		if (!owner_ || !physics_ || !collider_ || state_ != State::Dash)
		{
			return false;
		}

		Vector3 normalizedDirection = direction;
		normalizedDirection.y = 0.0f;
		if (normalizedDirection.LengthSquared() <= kDirectionEpsilonSq)
		{
			return false;
		}
		normalizedDirection.NormalizeSelf();

		reflectedVelocity_ = normalizedDirection * reflectedSpeed_;
		remainingReflectedTime_ = reflectedLifetimeSeconds_;
		state_ = State::Reflected;

		physics_->SetMovementVelocity(reflectedVelocity_);
		physics_->SetUseGravity(false);

		// 反射後はプレイヤー側の攻撃として敵へ当たるレイヤーに切り替える。
		collider_->SetCollisionLayer(CollisionLayer::PlayerBullet);
		collider_->SetCollisionMask(
			CollisionLayer::Player |
			CollisionLayer::PlayerBullet |
			CollisionLayer::Enemy |
			CollisionLayer::Terrain |
			CollisionLayer::Bumpers);
		owner_->SetTag(GameObjectTag::PlayerBullet);

		return true;
	}

	void DashMoveComponent::Idle(const GameObject* _owner)
	{
		if (elapsedTime_ > triggerTime_)
		{
			elapsedTime_ = 0.f;

			Vector3 toPlayer = player_->GetPosition() - _owner->GetPosition();
			dashDirection_ = toPlayer.Normalize();
			currentSpd_ = speed_;
			Audio::GetInstance()->PlayWave("se_spawn");
			state_ = State::Dash;
			return;
		}

		elapsedTime_ += TimeManager::GetInstance().GetGameContext().deltaTime;
	}

	void DashMoveComponent::Dash(const GameObject* _owner)
	{
		const float dt = TimeManager::GetInstance().GetGameContext().deltaTime;
		elapsedTime_ += dt;
		float brakeForce = BrakeCoefficient * elapsedTime_ * elapsedTime_;

		currentSpd_ -= brakeForce * dt;

		if (currentSpd_ <= MinSpd)
		{
			currentSpd_ = 0.f;
			elapsedTime_ = 0.f;
			state_ = State::Idle;
		}

		physics_->SetMovementVelocity(dashDirection_ * currentSpd_);
	}

	void DashMoveComponent::UpdateReflected(float deltaTime)
	{
		remainingReflectedTime_ -= deltaTime;
		if (remainingReflectedTime_ <= 0.0f)
		{
			collider_->SetActive(false);
			owner_->Destroy();
			return;
		}

		physics_->SetMovementVelocity(reflectedVelocity_);
	}

	void DashMoveComponent::HandleCollision(const CollisionInfo& info)
	{
		if (!info.otherCollider)
		{
			return;
		}

		const auto otherLayer = info.otherCollider->GetCollisionLayer();

		if (otherLayer & CollisionLayer::Terrain)
		{
			ResolveTerrainCollision(info);
		}

		// ダッシュ中にプレイヤーの反射判定に当たったら、プレイヤーの弾として跳ね返す
		if ((otherLayer & CollisionLayer::PlayerReflect) && info.other)
		{
			// PlayerReflect のコライダーは ReflectHand についているので、
			// 親をたどってプレイヤー本体を取得する
			GameObject* player = info.other->GetParent();
			auto reflect = player ? player->GetComponent<PlayerReflectComponent>() : nullptr;
			if (reflect && Reflect(reflect->GetReflectDirectionFrom(owner_->GetPosition())))
			{
				// 反射成功演出
				reflect->NotifyReflectSucceeded();

				// 吹っ飛ばしエフェクトを再生
				if (auto trail = owner_->GetComponent<TrailComponent>())
				{
					trail->Play(owner_);
				}
			}
			return;
		}

		// 反射前はプレイヤーの反射弾でダメージを受ける
		if (state_ != State::Reflected && (otherLayer & CollisionLayer::PlayerBullet))
		{
			if (auto status = owner_->GetComponent<StatusComponent>())
			{
				status->ApplyDamage(10);
			}
			return;
		}

		// 反射されたダッシュ敵が別の敵へ届いたら消滅する
		if (state_ == State::Reflected && (otherLayer & CollisionLayer::Enemy))
		{
			ParticleManager::GetInstance()->Play("bullet_hit", owner_->GetPosition());
			collider_->SetActive(false);
			owner_->Destroy();
			return;
		}
	}

	void DashMoveComponent::ResolveTerrainCollision(const CollisionInfo& info)
	{
		if (!info.otherCollider ||
			!(info.otherCollider->GetCollisionLayer() & (CollisionLayer::Terrain | CollisionLayer::Bumpers)))
		{
			return;
		}

		Vector3 position = owner_->GetPosition();
		position += info.normal * info.depth;
		owner_->SetPosition(position);

		if (info.normal.y <= 0.0f)
		{
			return;
		}

		physics_->SetGrounded(true);
		Vector3 velocity = physics_->GetExternalVelocity();
		if (velocity.y < 0.0f)
		{
			velocity.y = 0.0f;
			physics_->SetExternalVelocity(velocity);
		}
	}
}