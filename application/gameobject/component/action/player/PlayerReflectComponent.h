#pragma once

#include "engine/gameobject/component/base/IActionComponent.h"
#include "engine/graphics/2d/Sprite.h"
#include "jsonEditor/JsonEditableBase.h"
#include "math/Vector3.h"

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

	private:
		void UpdateLockOnTarget();
		void UpdateReflectCollider(GameObject* owner);
		Vector3 GetPlayerForward(const GameObject* owner) const;

		// ownerが所有する。PlayerReflectComponentより先に破棄されない前提。
		OBBColliderComponent* collider_ = nullptr;
		// CameraManagerが所有する。シーン中は有効な前提。
		Camera* camera_ = nullptr;
		// PlayerReflectComponentが所有するロックオン表示。
		std::unique_ptr<Sprite> lockOnMarker_;

		bool isReflecting_ = false;
		float reflectTimer_ = 0.0f;
		float lockOnRadiusNdc_ = 5.0f;

		bool hasLockOnTarget_ = false;
		Vector3 lockOnTargetPosition_ = {};
		bool hasReflectTarget_ = false;
		Vector3 reflectTargetPosition_ = {};
		Vector3 fallbackDirection_ = {0.0f, 0.0f, 1.0f};
	};
} // namespace GameObjectComponent
