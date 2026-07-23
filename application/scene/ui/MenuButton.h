#pragma once
#include <memory>
#include <string>

#include "engine/graphics/2d/Sprite.h"
#include "math/Vector2.h"
#include "math/Vector4.h"

class SpriteCommon;

/**
 * @brief クリック可能なUIボタン（画像未使用時は単色矩形で代用）。
 *        アンカーは中心。マウス座標との矩形判定でホバー/クリックを検知する。
 */
class MenuButton
{
public:
	void Initialize(SpriteCommon* spriteCommon, const std::string& texturePath,
					const Vector2& centerPos, const Vector2& size);

	/**
	 * @brief 状態更新。ホバー中にクリックされたら true を返す。
	 * @param mousePos       マウス座標（Sprite座標系＝1920x1080）
	 * @param mouseTriggered 左クリックした瞬間か
	 */
	bool Update(const Vector2& mousePos, bool mouseTriggered);

	/**
	 * @brief ホバー開始した瞬間かどうかを取得
	 */
	bool IsHoveredEnter() const { return isHoverEnter_; }

	void Draw();

	void SetColors(const Vector4& normal, const Vector4& hovered)
	{
		normalColor_ = normal;
		hoverColor_ = hovered;
	}

	

private:
	std::unique_ptr<Sprite> sprite_;
	Vector2 center_{};
	Vector2 size_{};
	Vector4 normalColor_{0.3f, 0.3f, 0.3f, 0.9f}; // 通常：暗いグレー
	Vector4 hoverColor_{0.9f, 0.8f, 0.2f, 1.0f};  // ホバー：黄色
	bool hovered_ = false;
	bool isHoverEnter_ = false; // ホバー開始フラグ
};