#include "EnemySpawnDirectionComponent.h"

#include "engine/gameobject/base/GameObject.h"
#include "engine/gameobject/component/base/ICollisionComponent.h"
#include "engine/time/TimeManager.h"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace GameObjectComponent
{
	EnemySpawnDirectionComponent::EnemySpawnDirectionComponent()
	{
		Register("spawnDuration", &spawnDuration_);
		Register("startScaleRate", &startScaleRate_);
		Register("startHeightOffset", &startHeightOffset_);
		Register("rotationAmount", &rotationAmount_);
		Register("scaleBouncePower", &scaleBouncePower_);
	}

	void EnemySpawnDirectionComponent::Start(GameObject* owner)
	{
		if (!owner || isInitialized_)
		{
			return;
		}

		InitializeSpawn(owner);
	}

	void EnemySpawnDirectionComponent::Update(GameObject* owner)
	{
		if (!owner || isFinished_)
		{
			return;
		}

		if (!isInitialized_)
		{
			InitializeSpawn(owner);
		}

		const float deltaTime =
			TimeManager::GetInstance()
				.GetGameContext()
				.deltaTime;

		UpdateSpawn(owner, deltaTime);
	}

	void EnemySpawnDirectionComponent::InitializeSpawn(
		GameObject* owner)
	{
		if (!owner)
		{
			return;
		}

		// シーン側で設定された本来の状態を保存
		basePosition_ = owner->GetPosition();
		baseRotation_ = owner->GetRotation();
		baseScale_ = owner->GetScale();

		spawnTimer_ = 0.0f;

		// 登場中は当たり判定を無効化
		collider_ =
			owner->GetComponent<ICollisionComponent>().get();

		if (collider_)
		{
			collider_->SetActive(false);
		}

		// 小さく、少し下に配置
		owner->SetPosition(
			basePosition_ +
			Vector3{0.0f, startHeightOffset_, 0.0f});

		owner->SetScale({baseScale_.x * startScaleRate_,
						 baseScale_.y * startScaleRate_,
						 baseScale_.z * startScaleRate_});

		isInitialized_ = true;
	}

	void EnemySpawnDirectionComponent::UpdateSpawn(
		GameObject* owner,
		float deltaTime)
	{
		if (!owner)
		{
			return;
		}

		spawnTimer_ += deltaTime;

		float t = 1.0f;

		if (spawnDuration_ > 0.0f)
		{
			t = spawnTimer_ / spawnDuration_;
		}

		t = std::clamp(t, 0.0f, 1.0f);

		// 最初は速く、最後はゆっくり止まる
		const float inverseT = 1.0f - t;
		const float easedT =
			1.0f - inverseT * inverseT * inverseT;

		// 下から本来の位置へ浮上
		Vector3 position = basePosition_;

		position.y +=
			startHeightOffset_ *
			(1.0f - easedT);

		owner->SetPosition(position);

		// 回転しながら登場
		Vector3 rotation = baseRotation_;

		rotation.y +=
			rotationAmount_ *
			(1.0f - t);

		owner->SetRotation(rotation);

		// 小さい状態から本来の大きさへ
		float scaleRate =
			startScaleRate_ +
			(1.0f - startScaleRate_) *
				easedT;

		// 中盤で少しだけ膨らませる
		const float bounce =
			std::sin(
				t * std::numbers::pi_v<float>) *
			scaleBouncePower_;

		scaleRate += bounce;

		owner->SetScale({baseScale_.x * scaleRate,
						 baseScale_.y * scaleRate,
						 baseScale_.z * scaleRate});

		if (t >= 1.0f)
		{
			FinishSpawn(owner);
		}
	}

	void EnemySpawnDirectionComponent::FinishSpawn(
		GameObject* owner)
	{
		if (!owner)
		{
			return;
		}

		// 誤差が残らないよう本来の状態へ戻す
		owner->SetPosition(basePosition_);
		owner->SetRotation(baseRotation_);
		owner->SetScale(baseScale_);

		// 当たり判定を有効化
		if (collider_)
		{
			collider_->SetActive(true);
		}

		isFinished_ = true;
	}
} // namespace GameObjectComponent