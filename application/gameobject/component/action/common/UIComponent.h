#pragma once
#include "engine/gameobject/component/base/IActionComponent.h"
#include "engine/graphics/2d/Sprite.h"
#include "jsonEditor/JsonEditableBase.h"
#include "math/Vector2.h"
#include "math/Vector3.h"
#include <memory>

class SpriteCommon;
class Camera;

namespace GameObjectComponent
{
	class StatusComponent;

	/**
	 * @brief キャラクターの3D位置に追従してHPバーを表示するUIコンポーネント
	 */
	class UIComponent : public IActionComponent, public JsonEditableBase
	{
	public:
		/**
		 * @brief コンストラクタ
		 * @param owner このコンポーネントを所有するGameObject
		 * @param spriteCommon スプライト共通設定
		 * @param camera 3D空間からスクリーン座標への変換用カメラ
		 * @param offset 3Dオブジェクト位置からの表示オフセット
		 */
		UIComponent(::GameObject* owner, SpriteCommon* spriteCommon, Camera* camera, const Vector3& offset = { 0.0f, 2.0f, 0.0f });

		/**
		 * @brief デストラクタ
		 */
		~UIComponent() override;

		/**
		 * @brief 毎フレームの更新処理
		 * @param owner このコンポーネントを所有するGameObject
		 */
		void Update(::GameObject* owner) override;

		/**
		 * @brief 2D描画処理
		 */
		void Draw2D() override;

	public: // ゲッター・セッター
		void SetOffset3D(const Vector3& offset) { offset3D_ = offset; }
		void SetBarSize(const Vector2& size) { barSize_ = size; }
		void SetVisible(bool visible) { isVisible_ = visible; }
		const Vector3& GetOffset3D() const { return offset3D_; }
		const Vector2& GetBarSize() const { return barSize_; }
		bool IsVisible() const { return isVisible_; }

	private:
		// スプライトの初期化
		void InitializeSprites(SpriteCommon* spriteCommon);

	private:
		// HPバー背景スプライト（UIComponentが所有）
		std::unique_ptr<Sprite> hpBarBg_;
		// HPバー残量スプライト（UIComponentが所有）
		std::unique_ptr<Sprite> hpBarFill_;

		// 表示フラグ
		bool isVisible_ = true;

		// 描画カメラ。CameraManagerが所有し、シーン中は有効な前提
		Camera* camera_ = nullptr;

		// 3Dオブジェクトからのオフセット座標
		Vector3 offset3D_ = { 0.0f, 2.0f, 0.0f };
		// バーの最大サイズ
		Vector2 barSize_ = { 100.0f, 10.0f };

		// 同一オブジェクト内のStatusComponentへのキャッシュ。ownerと同じ寿命
		std::shared_ptr<StatusComponent> cachedStatus_ = nullptr;
	};
}
