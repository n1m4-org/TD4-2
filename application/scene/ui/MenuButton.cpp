#include "MenuButton.h"

#include "engine/graphics/2d/SpriteCommon.h"

void MenuButton::Initialize(SpriteCommon* spriteCommon, const std::string& texturePath,
							const Vector2& centerPos, const Vector2& size)
{
	center_ = centerPos;
	size_ = size;

	sprite_ = std::make_unique<Sprite>();
	sprite_->Initialize(spriteCommon, texturePath);
	sprite_->SetAnchorPoint({0.5f, 0.5f}); // 中心基準
	sprite_->SetPosition(center_);
	sprite_->SetSize(size_);
	sprite_->SetColor(normalColor_);
}

bool MenuButton::Update(const Vector2& mousePos, bool mouseTriggered)
{
	// 中心アンカーの矩形当たり判定
	const float halfW = size_.x * 0.5f;
	const float halfH = size_.y * 0.5f;

	bool previousHovered = hovered_;
	hovered_ =
		mousePos.x >= center_.x - halfW && mousePos.x <= center_.x + halfW &&
		mousePos.y >= center_.y - halfH && mousePos.y <= center_.y + halfH;

	isHoverEnter_ = !previousHovered && hovered_;

	sprite_->SetColor(hovered_ ? hoverColor_ : normalColor_);
	sprite_->SetPosition(center_);
	sprite_->SetSize(size_);
	sprite_->Update();

	return hovered_ && mouseTriggered;
}

void MenuButton::Draw()
{
	if (sprite_)
	{
		sprite_->Draw();
	}
}