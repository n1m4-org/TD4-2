#pragma once
#include "gameobject/component/base/IActionComponent.h"
#include "math/Vector3.h"

namespace GameObjectComponent
{
	class ICollisionComponent;
	class PhysicsComponent;
	struct CollisionInfo;

	class DashMoveComponent : public IActionComponent
	{
		GameObject* player_ = nullptr;
		// owner自身。GameObjectがこのコンポーネントを所有する。
		GameObject* owner_ = nullptr;
		// ownerが所有する。DashMoveComponentより先に破棄されない前提。
		PhysicsComponent* physics_ = nullptr;
		// ownerが所有する。DashMoveComponentより先に破棄されない前提。
		ICollisionComponent* collider_ = nullptr;

		float elapsedTime_ = 0.f;
		float triggerTime_ = 3.6f;

		Vector3 dashDirection_ = {};
		float speed_ = 50.f;

		// 予測線演出: triggerTime_をダッシュまでの4拍に見立て、最初の3拍で透明な予測オブジェクトを飛ばし、
		// 4拍目のタイミングで実際にダッシュする。
		static constexpr int32_t PREDICTION_COUNT = 3;
		int32_t predictionsFired_ = 0;
		float predictionSpeed_ = 20.f;
		float predictionLifetime_ = 0.6f;

		/** @brief ダッシュ敵の行動段階。反射後は通常のダッシュへ戻さない。 */
		enum class State { Idle, Dash, Reflected };
		State state_ = State::Idle;

		float currentSpd_ = 0.f;
		const float Decay = 0.05f;
		const float MinSpd = 0.5f;

		float BrakeCoefficient = 600.f;

		// 反射後の速度・寿命
		Vector3 reflectedVelocity_ = {};
		float reflectedSpeed_ = 40.f;
		float reflectedLifetimeSeconds_ = 3.f;
		float remainingReflectedTime_ = 0.f;

	public:
		DashMoveComponent(GameObject* _player);
		void Update(GameObject* _owner) override;

		/**
		 * @brief ダッシュ中の敵をプレイヤーの弾として指定方向へ反射する。
		 * @param direction 反射方向
		 * @return 反射を受け付けた場合はtrue
		 */
		bool Reflect(const Vector3& direction);

	private:
		/** @brief 依存コンポーネントの取得と衝突コールバックの初期化を行う。 */
		void InitializeComponents(GameObject* _owner);

		void Idle(const GameObject* _owner);
		void Dash(const GameObject* _owner);

		/** @brief 予測線用の透明オブジェクトをプレイヤー方向へ飛ばす。 */
		void SpawnPredictionObject(const GameObject* _owner);

		/** @brief 反射後に確定した速度を維持し、寿命切れで消滅する。 */
		void UpdateReflected(float deltaTime);

		/** @brief 衝突相手のレイヤーに応じて状態と生存を変更する。 */
		void HandleCollision(const CollisionInfo& info);

		/** @brief 地形とのめり込みと接地時の落下速度を補正する。 */
		void ResolveTerrainCollision(const CollisionInfo& info);
	};

}