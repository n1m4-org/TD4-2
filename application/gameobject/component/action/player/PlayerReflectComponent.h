#pragma once

#include "engine/gameobject/component/base/IActionComponent.h"
#include "engine/graphics/2d/Sprite.h"
#include "jsonEditor/JsonEditableBase.h"
#include "math/Vector3.h"
#include "engine/effects/particle/ParticleEffect.h"

#include <memory>

class Camera;
class SpriteCommon;

namespace GameObjectComponent
{
	class OBBColliderComponent;

	/**
	 * @brief プレイヤーのロックオンと反射判定を制御する。
	 */
	class PlayerReflectComponent : public IActionComponent
		, public JsonEditableBase
	{
	public:
		/**
		 * @brief 反射処理に使用するカメラを設定する。
		 * @param camera CameraManagerが所有するアクティブカメラ
		 * @param spriteCommon SceneManagerが所有するスプライト共通設定
		 */
		PlayerReflectComponent(Camera* camera, SpriteCommon* spriteCommon);

		/**
		 * @brief ロックオンと反射判定を更新する。
		 * @param owner このコンポーネントを所有するプレイヤー
		 */
		void Update(GameObject* owner) override;

		/**
		 * @brief ロック中の対象位置へマーカーを描画する。
		 */
		void Draw2D() override;

		/**
		 * @brief 指定位置から今回の反射先へ向かう方向を取得する。
		 * @param sourcePosition 反射されるオブジェクトの現在位置
		 * @return 水平面上で正規化した反射方向
		 */
		Vector3 GetReflectDirectionFrom(const Vector3& sourcePosition) const;

		/**
		 * @brief 反射成立時のリアクションとヒットストップを開始する。
		 */
		void NotifyReflectSucceeded();

	private:
		/**
		 * @brief カーソル付近の敵を探し、入力時に行き先として保持する。
		 * @param isLockOnTriggered ロックオン入力が発生したか
		 */
		void UpdateLockOnTarget(bool isLockOnTriggered);

		/**
		 * @brief プレイヤーのY軸回転から水平な正面方向を取得する。
		 * @param owner このコンポーネントを所有するプレイヤー
		 * @return 正規化した正面方向
		 */
		Vector3 GetPlayerForward(const GameObject* owner) const;

		/**
		 * @brief 手を振るアニメーションを更新する。
		 * @param deltaTime ゲーム時間の経過秒
		 */
		void UpdateHandAnimation(float deltaTime);

		// hand_が所有する。hand_はownerの子なので、このコンポーネントより後に破棄される。
		OBBColliderComponent* collider_ = nullptr;
		// ownerが所有する子オブジェクト。ownerと同じ期間だけ有効。
		GameObject* hand_ = nullptr;
		// 横振りの基準になる手のローカル位置
		Vector3 handBasePosition_ = {};
		// プレイヤーを中心とした横振り円弧の半径
		float handArcRadius_ = 0.0f;
		// CameraManagerが所有する。シーン中は有効な前提。
		Camera* camera_ = nullptr;
		// PlayerReflectComponentが所有するロックオン表示。
		std::unique_ptr<Sprite> lockOnMarker_;
		// GameObjectManagerが管理するロック対象。登録中だけ参照する（所有しない）。
		GameObject* lockOnTarget_ = nullptr;
		// パーティクルのポインタ
		ParticleEffect* handEffect_ = nullptr;

		bool isReflecting_ = false;
		float reflectTimer_ = 0.0f;
		float activationAnimationTimer_ = 0.0f;
		float lockOnRadiusNdc_ = 0.2f;
		static constexpr float kDefaultActivationAnimationDuration = 0.28f;
		static constexpr float kDefaultHitStopDuration = 0.08f;
		static constexpr float kDefaultWindUpArcRadians = 0.8f;
		static constexpr float kDefaultSwingArcRadians = -0.8f;
		static constexpr float kDefaultHandRadialOffset = 0.5f;
		static constexpr float kDefaultCameraShakeIntensity = 0.35f;
		static constexpr float kDefaultCameraShakeDuration = 0.16f;
		static constexpr float kDefaultCameraZoomFovOffset = -0.08f;
		static constexpr float kDefaultCameraZoomDuration = 0.3f;
		float activationAnimationDuration_ = kDefaultActivationAnimationDuration;
		float hitStopDuration_ = kDefaultHitStopDuration;
		float windUpArcRadians_ = kDefaultWindUpArcRadians;
		float swingArcRadians_ = kDefaultSwingArcRadians;
		float handRadialOffset_ = kDefaultHandRadialOffset;
		float cameraShakeIntensity_ = kDefaultCameraShakeIntensity;
		float cameraShakeDuration_ = kDefaultCameraShakeDuration;
		float cameraZoomFovOffset_ = kDefaultCameraZoomFovOffset;
		float cameraZoomDuration_ = kDefaultCameraZoomDuration;

		bool hasLockOnTarget_ = false;
		Vector3 lockOnTargetPosition_ = {};
		bool hasReflectTarget_ = false;
		Vector3 reflectTargetPosition_ = {};
		Vector3 fallbackDirection_ = {0.0f, 0.0f, 1.0f};
	};
} // namespace GameObjectComponent
