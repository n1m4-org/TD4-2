#include "UIComponent.h"
#include "StatusComponent.h"
#include "engine/gameobject/base/GameObject.h"
#include "base/Camera.h"
#include "engine/math/MathUtils.h"
#include "engine/graphics/2d/SpriteCommon.h"

namespace
{
	// 使用するテクスチャ。1x1の白画像
	constexpr char kDefaultTexturePath[] = "./Resources/white1x1.png";
}

namespace GameObjectComponent
{
	UIComponent::UIComponent(::GameObject* owner, SpriteCommon* spriteCommon, Camera* camera, const Vector3& offset)
		: camera_(camera), offset3D_(offset)
	{
		InitializeSprites(spriteCommon);
	}

	UIComponent::~UIComponent() = default;

	void UIComponent::InitializeSprites(SpriteCommon* spriteCommon)
	{
		if (!spriteCommon)
		{
			return;
		}

		// 背景スプライト初期化
		hpBarBg_ = std::make_unique<Sprite>();
		hpBarBg_->Initialize(spriteCommon, kDefaultTexturePath);
		hpBarBg_->SetSize(barSize_);
		hpBarBg_->SetColor({ 0.2f, 0.2f, 0.2f, 0.8f }); // 暗いグレー

		// 残量スプライト初期化
		hpBarFill_ = std::make_unique<Sprite>();
		hpBarFill_->Initialize(spriteCommon, kDefaultTexturePath);
		hpBarFill_->SetSize(barSize_);
		hpBarFill_->SetColor({ 0.0f, 1.0f, 0.0f, 1.0f }); // 緑
	}

	void UIComponent::Update(::GameObject* owner)
	{
		if (!owner || !camera_)
		{
			return;
		}

		// 必要なコンポーネントをキャッシュ
		if (!cachedStatus_)
		{
			cachedStatus_ = owner->GetComponent<StatusComponent>();
		}

		if (!cachedStatus_)
		{
			return;
		}

		// HP割合の算出
		float hpRatio = 0.0f;
		if (cachedStatus_->GetMaxHp() > 0)
		{
			hpRatio = static_cast<float>(cachedStatus_->GetHp()) / cachedStatus_->GetMaxHp();
			hpRatio = std::clamp(hpRatio, 0.0f, 1.0f);
		}

		// 3D空間上の追従位置をNDC座標に変換
		Vector3 worldPos = owner->GetPosition() + offset3D_;
		Vector3 ndc = MathUtils::Transform(worldPos, camera_->GetViewProjectionMatrix());

		// 画面外（カメラ背面など）は描画しない
		if (ndc.z < 0.0f || ndc.z > 1.0f || ndc.x < -1.0f || ndc.x > 1.0f || ndc.y < -1.0f || ndc.y > 1.0f)
		{
			hpBarBg_->SetColor({ 0.0f, 0.0f, 0.0f, 0.0f });
			hpBarFill_->SetColor({ 0.0f, 0.0f, 0.0f, 0.0f });
			return;
		}

		// 表示する色を設定
		hpBarBg_->SetColor({ 0.2f, 0.2f, 0.2f, 0.8f });
		hpBarFill_->SetColor({ 0.0f, 1.0f, 0.0f, 1.0f });

		// NDC座標を画面のピクセル座標系（基準解像度）へ変換
		Vector2 screenPos = {
			(ndc.x - (-1.0f)) * 0.5f * Sprite::kCoordinateWidth,
			(1.0f - ndc.y) * 0.5f * Sprite::kCoordinateHeight
		};

		// 背景スプライトは中央揃え
		const Vector2 anchor = { 0.5f, 0.5f };
		hpBarBg_->SetAnchorPoint(anchor);
		hpBarBg_->SetPosition(screenPos);
		hpBarBg_->Update();

		// 残量バーは左端から右に向かって伸縮させる
		const Vector2 fillAnchor = { 0.0f, 0.5f };
		Vector2 fillPos = { screenPos.x - barSize_.x * 0.5f, screenPos.y };

		hpBarFill_->SetAnchorPoint(fillAnchor);
		hpBarFill_->SetPosition(fillPos);
		hpBarFill_->SetSize({ barSize_.x * hpRatio, barSize_.y });
		hpBarFill_->Update();
	}

	void UIComponent::Draw2D()
	{
		// 非生存時、またはステータスがない場合は描画しない
		if (cachedStatus_ && !cachedStatus_->IsAlive())
		{
			return;
		}

		if (hpBarBg_)
		{
			hpBarBg_->Draw();
		}
		if (hpBarFill_)
		{
			hpBarFill_->Draw();
		}
	}
}
