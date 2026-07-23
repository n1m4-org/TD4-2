#pragma once
#include "engine/gameobject/component/base/IActionComponent.h"
#include "math/Vector3.h"
#include "graphics/2d/Sprite.h"

class SpriteCommon;

namespace GameObjectComponent
{
	class PlayerInputComponent : public IActionComponent
	{
	public:

		~PlayerInputComponent() = default;
		void Update(GameObject* owner) override;

		void Draw2D() override;

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


		// UIの初期化
		void InitializeUI(SpriteCommon* spriteCommon);

	private:

		// UIの作成
		std::unique_ptr<Sprite> CreateSprite(const std::string& tex);


	private:

		SpriteCommon* spriteCommon_ = nullptr;

		// 移動方向ベクトル
		Vector3 moveDirection_ = {0.0f, 0.0f, 0.0f};
		// 反射トリガーが発生したかどうか
		bool isReflectTriggered_ = false;
		// ロックオン入力が発生したフレームだけtrueになる。
		bool isLockOnTriggered_ = false;
		// スローモーショントリガーが発生したかどうか
		bool isSlowMotionTriggered_ = false;

		struct PlayUI
		{
			std::unique_ptr<Sprite> background;

			std::unique_ptr<Sprite> wBlack;
			std::unique_ptr<Sprite> aBlack;
			std::unique_ptr<Sprite> sBlack;
			std::unique_ptr<Sprite> dBlack;

			std::unique_ptr<Sprite> shiftBlack;
			std::unique_ptr<Sprite> spaceBlack;

			std::unique_ptr<Sprite> mouseLeft;
			std::unique_ptr<Sprite> mouseRight;
		};

		PlayUI ui_;

		// UIを非表示にするタイマー
		float uiHideTimer_ = 0.0f;

		// UIを非表示中か
		bool isHideUI_ = false;

	};

} // namespace GameObjectComponent
