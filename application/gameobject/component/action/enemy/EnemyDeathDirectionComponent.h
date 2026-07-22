#pragma once

#include "engine/gameobject/component/base/IActionComponent.h"
#include "jsonEditor/JsonEditableBase.h"
#include "math/Vector3.h"

#include <string>

class GameObject;

namespace GameObjectComponent
{
	class StatusComponent;
	class PhysicsComponent;
	class ICollisionComponent;

	class EnemyDeathDirectionComponent
		: public IActionComponent
		, public JsonEditableBase
	{
	public:
		explicit EnemyDeathDirectionComponent(
			const std::string& particleName = "bullet_hit");

		void Update(GameObject* owner) override;

		void StartDeath(GameObject* owner);

		// 在死亡演出中か。
		bool IsDying() const { return isDying_; }

		// 死亡演出が終了したか。
		bool IsFinished() const { return isFinished_; }

	private:
		// 必要なコンポーネントを取得する
		void InitializeComponents(GameObject* owner);

		// 死亡演出を更新する
		void UpdateDeathEffect(GameObject* owner, float deltaTime);

		// 死亡演出開始時に移動・当たり判定を止める。
		void DisableEnemyActions(GameObject* owner);

		// 死亡演出を終了する。
		void FinishDeath(GameObject* owner);

		// 0～1に収める。
		float Clamp01(float value) const;

	private:
		StatusComponent* status_ = nullptr;
		PhysicsComponent* physics_ = nullptr;
		ICollisionComponent* collider_ = nullptr;

		bool isInitialized_ = false;
		bool isDying_ = false;
		bool isFinished_ = false;

		float deathTimer_ = 0.0f;

		// 死亡開始時の情報
		Vector3 basePosition_ = {};
		Vector3 baseRotation_ = {};
		Vector3 baseScale_ = {1.0f, 1.0f, 1.0f};

		// 演出全体の時間
		float deathDuration_ = 0.55f;

		// 前半の膨張が終了する割合
		float popEndRate_ = 0.18f;

		// 膨張倍率
		float popScale_ = 1.25f;

		// 揺れ幅
		float shakePower_ = 0.18f;

		// 揺れ速度
		float shakeSpeed_ = 55.0f;

		// Y軸回転量
		float rotationAmount_ = 8.0f;

		// 浮き上がる高さ
		float riseHeight_ = 1.5f;

		// 終了時に再生するパーティクル
		std::string particleName_;
	};
} // namespace GameObjectComponent