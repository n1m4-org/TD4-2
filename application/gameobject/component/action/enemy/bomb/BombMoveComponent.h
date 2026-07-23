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
	class SphereColliderComponent;

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
			Ignition,
			Chasing,
			Reflected,
			Exploding,
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

		/**
		 * @brief 爆発判定の球を時間経過で拡大し、終了後に本体を無効化する。
		 * @param deltaTime 経過秒
		 */
		void UpdateExploding(float deltaTime);

		/** @brief 反射された弾に撃たれた際の即死処理。爆発演出を出して自身を破棄する。 */
		void Die();

		/**
		 * @brief 着火モーションを再生し、終了後に追跡へ移行する。
		 * @param deltaTime 経過秒
		 */
		void UpdateIgnition(float deltaTime);

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
		float chaseSpeed_ = 10.0f;
		float chaseLifetimeSeconds_ = 5.0f;
		float reflectedSpeed_ = 25.0f;
		float reflectedLifetimeSeconds_ = 1.0f;

		// ドリフト用: 現在の進行方向
		Vector3 dashDirection_ = {0.0f, 0.0f, 1.0f};
		// 旋回の追従率（小さいほど曲がりにくく、ドリフトが大きくなる）
		float turnRate_ = 0.04f;

		// 赤点滅の位相（累積値）
		float blinkPhase_ = 0.0f;
		// 点滅速度（1フレームあたりの位相の進み）
		float blinkSpeedMin_ = 0.08f;
		float blinkSpeedMax_ = 0.7f;


		// ownerが所有する爆発判定用の球コライダー
		SphereColliderComponent* explosionCollider_ = nullptr;

		// 爆発判定の持続時間・最大半径・ダメージ
		float explosionDurationSeconds_ = 0.4f;
		float explosionMaxRadius_ = 6.0f;
		int explosionDamage_ = 20;
		// 爆発開始からの経過秒
		float explosionElapsedSeconds_ = 0.0f;

				// 着火モーションの長さと経過秒
		float ignitionDurationSeconds_ = 0.4f;
		float ignitionElapsedSeconds_ = 0.0f;
		// モーションで変形させる前の基準スケール
		Vector3 baseScale_ = {1.0f, 1.0f, 1.0f};
	};
} // namespace GameObjectComponent