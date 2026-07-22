#include "EnemyDeathDirectionComponent.h"

#include "application/collision/CollisionLayer.h"
#include "application/gameobject/component/action/common/PhysicsComponent.h"
#include "application/gameobject/component/action/common/StatusComponent.h"
#include "engine/effects/particle/ParticleManager.h"
#include "engine/gameobject/base/GameObject.h"
#include "engine/gameobject/component/base/ICollisionComponent.h"
#include "engine/time/TimeManager.h"

#include <algorithm>
#include <cmath>

namespace GameObjectComponent
{
	EnemyDeathDirectionComponent::EnemyDeathDirectionComponent(
		const std::string& particleName)
		: particleName_(particleName)
	{
		Register("deathDuration", &deathDuration_);
		Register("popEndRate", &popEndRate_);
		Register("popScale", &popScale_);
		Register("shakePower", &shakePower_);
		Register("shakeSpeed", &shakeSpeed_);
		Register("rotationAmount", &rotationAmount_);
		Register("riseHeight", &riseHeight_);
	}

	void EnemyDeathDirectionComponent::Update(GameObject* owner)
	{
		if (!owner || isFinished_)
		{
			return;
		}

		if (!isInitialized_)
		{
			InitializeComponents(owner);
		}

		// まだ死亡演出中でなければ、HPを確認する
		if (!isDying_)
		{
			if (!status_)
			{
				return;
			}

			if (status_->GetHp() <= 0)
			{
				StartDeath(owner);
			}
		}

		if (!isDying_)
		{
			return;
		}

		const float deltaTime =
			TimeManager::GetInstance().GetGameContext().deltaTime;

		UpdateDeathEffect(owner, deltaTime);
	}

	void EnemyDeathDirectionComponent::InitializeComponents(GameObject* owner)
	{
		if (!owner)
		{
			return;
		}

		status_ = owner->GetComponent<StatusComponent>().get();
		physics_ = owner->GetComponent<PhysicsComponent>().get();
		collider_ = owner->GetComponent<ICollisionComponent>().get();

		isInitialized_ = true;
	}

	void EnemyDeathDirectionComponent::StartDeath(GameObject* owner)
	{
		if (!owner || isDying_ || isFinished_)
		{
			return;
		}

		if (!isInitialized_)
		{
			InitializeComponents(owner);
		}

		isDying_ = true;
		deathTimer_ = 0.0f;

		basePosition_ = owner->GetPosition();
		baseRotation_ = owner->GetRotation();
		baseScale_ = owner->GetScale();

		DisableEnemyActions(owner);
	}

	void EnemyDeathDirectionComponent::DisableEnemyActions(GameObject* owner)
	{
		if (!owner)
		{
			return;
		}

		if (physics_)
		{
			physics_->SetMovementVelocity({0.0f, 0.0f, 0.0f});
			physics_->SetExternalVelocity({0.0f, 0.0f, 0.0f});
			physics_->SetUseGravity(false);
		}

		if (collider_)
		{
			collider_->SetActive(false);
			collider_->SetCollisionLayer(CollisionLayer::None);
			collider_->SetCollisionMask(CollisionLayer::None);
		}
	}

	void EnemyDeathDirectionComponent::UpdateDeathEffect(
		GameObject* owner,
		float deltaTime)
	{
		if (!owner)
		{
			return;
		}

		deathTimer_ += deltaTime;

		const float safeDuration =
			(deathDuration_ > 0.001f) ? deathDuration_ : 0.001f;

		const float progress =
			Clamp01(deathTimer_ / safeDuration);

		const float safePopEndRate =
			std::clamp(popEndRate_, 0.01f, 0.99f);

		float scaleRate = 1.0f;

		// 前半：一瞬膨らませる
		if (progress < safePopEndRate)
		{
			const float popProgress =
				progress / safePopEndRate;

			// 0から1へ滑らかに変化
			const float easedPop =
				1.0f -
				(1.0f - popProgress) *
					(1.0f - popProgress);

			scaleRate =
				1.0f +
				(popScale_ - 1.0f) *
					easedPop;
		}
		// 後半：縮小して消す
		else
		{
			const float shrinkProgress =
				(progress - safePopEndRate) /
				(1.0f - safePopEndRate);

			// 終盤に勢いよく縮む
			const float inverse =
				1.0f - shrinkProgress;

			const float easedShrink =
				inverse * inverse;

			scaleRate =
				popScale_ *
				easedShrink;
		}

		// 終盤ほど揺れを弱くする
		const float shakeRemain =
			1.0f - progress;

		const float shakeX =
			std::sin(deathTimer_ * shakeSpeed_) *
			shakePower_ *
			shakeRemain;

		const float shakeZ =
			std::cos(deathTimer_ * shakeSpeed_ * 1.37f) *
			shakePower_ *
			shakeRemain;

		Vector3 position = basePosition_;
		position.x += shakeX;
		position.z += shakeZ;

		// 少し浮き上がる
		position.y +=
			riseHeight_ *
			progress;

		Vector3 rotation = baseRotation_;
		rotation.y +=
			rotationAmount_ *
			progress;

		owner->SetPosition(position);
		owner->SetRotation(rotation);
		owner->SetScale(baseScale_ * scaleRate);

		if (progress >= 1.0f)
		{
			FinishDeath(owner);
		}
	}

	void EnemyDeathDirectionComponent::FinishDeath(GameObject* owner)
	{
		if (!owner || isFinished_)
		{
			return;
		}

		isFinished_ = true;
		isDying_ = false;

		if (!particleName_.empty())
		{
			ParticleManager::GetInstance()->Play(
				particleName_,
				owner->GetPosition());
		}

		owner->Destroy();
	}

	float EnemyDeathDirectionComponent::Clamp01(float value) const
	{
		return std::clamp(value, 0.0f, 1.0f);
	}
} // namespace GameObjectComponent