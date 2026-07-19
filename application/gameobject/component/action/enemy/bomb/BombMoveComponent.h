#pragma once

#include "engine/gameobject/component/base/IActionComponent.h"
#include "jsonEditor/JsonEditableBase.h"
#include "math/Vector3.h"

class GameObject;

namespace GameObjectComponent
{
	class ICollisionComponent;
	class PhysicsComponent;
	struct CollisionInfo;

	/**
	 * @brief ボムの待機、追跡、反射、爆発を制御する。
	 */
	class BombMoveComponent : public IActionComponent
		, public JsonEditableBase
	{
	public:
		/**
		 * @brief 追跡対象のプレイヤーを設定する。
		 * @param player TestSceneが所有するプレイヤー
		 */
		explicit BombMoveComponent(GameObject* player);

		/**
		 * @brief 現在の状態に応じてボムを更新する。
		 * @param owner このコンポーネントを所有するボム
		 */
		void Update(GameObject* owner) override;

		/**
		 * @brief ボムを指定方向へ反射する。
		 * @param direction 反射方向
		 * @return 反射を受け付けた場合はtrue
		 */
		bool Reflect(const Vector3& direction);

	private:
		enum class State
		{
			Idle,
			Chasing,
			Reflected,
			Exploded,
		};

		bool InitializeComponents(GameObject* owner);
		void UpdateIdle(GameObject* owner);
		void UpdateChasing(GameObject* owner, float deltaTime);
		void UpdateReflected(float deltaTime);
		void Explode();
		void HandleCollision(const CollisionInfo& info);
		void ResolveTerrainCollision(const CollisionInfo& info);

		// TestSceneが所有する。ボムより先に破棄されない前提。
		GameObject* player_ = nullptr;
		// owner自身。GameObjectがこのコンポーネントを所有する。
		GameObject* owner_ = nullptr;
		// ownerが所有する。BombMoveComponentより先に破棄されない前提。
		PhysicsComponent* physics_ = nullptr;
		// ownerが所有する。BombMoveComponentより先に破棄されない前提。
		ICollisionComponent* collider_ = nullptr;

		State state_ = State::Idle;
		Vector3 reflectedVelocity_ = {};
		float remainingLifetimeSeconds_ = 0.0f;

		float chaseRange_ = 30.0f;
		float chaseSpeed_ = 7.0f;
		float chaseLifetimeSeconds_ = 5.0f;
		float reflectedSpeed_ = 10.0f;
		float reflectedLifetimeSeconds_ = 1.0f;
	};
} // namespace GameObjectComponent
