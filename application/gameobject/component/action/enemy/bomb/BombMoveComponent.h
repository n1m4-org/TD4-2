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
		/** @brief ボムの行動段階。反射後は通常の追跡へ戻さない。 */
		enum class State
		{
			Idle,
			Chasing,
			Reflected,
			Exploded,
		};

		/**
		 * @brief ownerが所有する依存コンポーネントと衝突コールバックを初期化する。
		 * @param owner このコンポーネントを所有するボム
		 * @return 更新可能な状態ならtrue
		 */
		bool InitializeComponents(GameObject* owner);

		/**
		 * @brief 待機中にプレイヤーが追跡範囲へ入ったか確認する。
		 * @param owner このコンポーネントを所有するボム
		 */
		void UpdateIdle(GameObject* owner);

		/**
		 * @brief 制限時間内だけプレイヤーを追跡する。
		 * @param owner このコンポーネントを所有するボム
		 * @param deltaTime 経過秒
		 */
		void UpdateChasing(GameObject* owner, float deltaTime);

		/**
		 * @brief 反射時に確定した速度を維持し、寿命切れで爆発する。
		 * @param deltaTime 経過秒
		 */
		void UpdateReflected(float deltaTime);

		/** @brief 爆発演出を再生し、ボム本体とコライダーを無効化する。 */
		void Explode();

		/**
		 * @brief 残り時間の割合に応じて赤点滅させる。
		 * @param remainRatio 残り時間の割合（1.0=開始直後、0.0=爆発直前）
		 */
		void UpdateBlink(float remainRatio);

		/**
		 * @brief 衝突相手のレイヤーに応じて自身の状態を遷移させる。
		 * @param info 衝突情報
		 */
		void HandleCollision(const CollisionInfo& info);

		/**
		 * @brief 地形とのめり込みと接地時の落下速度を補正する。
		 * @param info 衝突情報
		 */
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

		// ドリフト用: 現在の進行方向
		Vector3 dashDirection_ = {0.0f, 0.0f, 1.0f};
		// 旋回の追従率（小さいほど曲がりにくく、ドリフトが大きくなる）
		float turnRate_ = 0.04f;

		// 赤点滅の位相（累積値）
		float blinkPhase_ = 0.0f;
		// 点滅速度（1フレームあたりの位相の進み）: 開始直後〜爆発直前
		float blinkSpeedMin_ = 0.08f;
		float blinkSpeedMax_ = 0.7f;
	};
} // namespace GameObjectComponent