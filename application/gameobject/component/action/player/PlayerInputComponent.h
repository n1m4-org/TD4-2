#pragma once
#include "engine/gameobject/component/base/IActionComponent.h"
#include "math/Vector3.h"

namespace GameObjectComponent
{
	class PlayerInputComponent : public IActionComponent
	{
	public:
		~PlayerInputComponent() = default;
		void Update(GameObject* owner) override;

		// 移動方向ベクトルを取得
		const Vector3& GetMoveDirection() const { return moveDirection_; }

		// 反射トリガーが発生したかどうかを取得
		bool IsReflectTriggered() const { return isReflectTriggered_; }

		/**
		 * @brief ロックオン入力が発生したか取得する。
		 * @return 右クリックされたフレームならtrue
		 */
		bool IsLockOnTriggered() const { return isLockOnTriggered_; }

		// スローモーショントリガーが発生したかどうかを取得
		bool IsSlowMotionTriggered() const { return isSlowMotionTriggered_; }

	private:
		// 移動方向ベクトル
		Vector3 moveDirection_ = {0.0f, 0.0f, 0.0f};
		// 反射トリガーが発生したかどうか
		bool isReflectTriggered_ = false;
		// ロックオン入力が発生したフレームだけtrueになる。
		bool isLockOnTriggered_ = false;
		// スローモーショントリガーが発生したかどうか
		bool isSlowMotionTriggered_ = false;
	};

} // namespace GameObjectComponent
