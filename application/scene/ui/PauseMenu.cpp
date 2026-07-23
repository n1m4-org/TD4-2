#include "PauseMenu.h"

#include "engine/graphics/2d/SpriteCommon.h"
#include "engine/time/TimeManager.h"
#include "input/Input.h"

namespace
{
	constexpr char kPauseBackgroundTexturePath[] = "./Resources/white1x1.png";

	// ボタンの基本サイズ
	constexpr Vector2 kButtonSize =
		{360.0f, 120.0f};

	// ボタンのY座標
	constexpr float kButtonY = 540.0f;

	// 3つのボタン位置
	constexpr Vector2 kResumeButtonPosition =
		{500.0f, kButtonY};

	constexpr Vector2 kRestartButtonPosition =
		{960.0f, kButtonY};

	constexpr Vector2 kTitleButtonPosition =
		{1420.0f, kButtonY};

	// ホバー時の拡大倍率
	constexpr float kHoverScale = 1.08f;
} // namespace

void PauseMenu::Initialize(SpriteCommon* spriteCommon)
{
	pauseBackground_ = std::make_unique<Sprite>();
	pauseBackground_->Initialize(spriteCommon, kPauseBackgroundTexturePath);

	// ClearSceneと同じく左上基準で画面全体に広げる
	pauseBackground_->SetPosition({0.0f, 0.0f});

	pauseBackground_->SetSize({Sprite::kCoordinateWidth,
							   Sprite::kCoordinateHeight});

	pauseBackground_->SetColor({0.0f, 0.0f, 0.0f, 0.5f}); // 半透明の黒

	// ゲーム再開ボタン
	resumeButton_ = std::make_unique<Sprite>();
	resumeButton_->Initialize(
		spriteCommon,
		kPauseBackgroundTexturePath);

	resumeButton_->SetAnchorPoint({0.5f, 0.5f});
	resumeButton_->SetPosition(kResumeButtonPosition);
	resumeButton_->SetSize(kButtonSize);
	resumeButton_->SetColor({0.15f, 0.55f, 0.25f, 0.95f});

	// リスタートボタン
	restartButton_ = std::make_unique<Sprite>();
	restartButton_->Initialize(
		spriteCommon,
		kPauseBackgroundTexturePath);

	restartButton_->SetAnchorPoint({0.5f, 0.5f});
	restartButton_->SetPosition(kRestartButtonPosition);
	restartButton_->SetSize(kButtonSize);
	restartButton_->SetColor({0.15f, 0.55f, 0.25f, 0.95f});

	// タイトルへ戻るボタン
	titleButton_ = std::make_unique<Sprite>();
	titleButton_->Initialize(
		spriteCommon,
		kPauseBackgroundTexturePath);

	titleButton_->SetAnchorPoint({0.5f, 0.5f});
	titleButton_->SetPosition(kTitleButtonPosition);
	titleButton_->SetSize(kButtonSize);
	titleButton_->SetColor({0.15f, 0.55f, 0.25f, 0.95f});
}

PauseMenu::Result PauseMenu::Update()
{
	// ESCを押した瞬間にポーズ状態を切り替える
	if (Input::GetInstance()->TriggerKey(DIK_ESCAPE))
	{
		TogglePause();
	}

	// ポーズ中でなければボタン判定は一切行わない
	// （ゲーム中の射撃・反射クリックでリスタートやタイトル遷移が誤発動するのを防ぐ）
	if (!isPaused_)
	{
		return Result::None;
	}

	// ポーズ中だけSpriteを更新
	if (pauseBackground_)
	{
		pauseBackground_->Update();
	}

	const Vector2 mousePosition =
		Input::GetInstance()->GetMousePosition();

	const bool mouseTriggered =
		Input::GetInstance()->IsMouseButtonTriggered(0);

	// 左：ゲーム再開
	if (resumeButton_ &&
		UpdateButton(
			resumeButton_.get(),
			kResumeButtonPosition,
			kButtonSize,
			mousePosition,
			mouseTriggered))
	{
		TogglePause();
		return Result::None;
	}

	// 中央：リスタート
	if (restartButton_ &&
		UpdateButton(
			restartButton_.get(),
			kRestartButtonPosition,
			kButtonSize,
			mousePosition,
			mouseTriggered))
	{
		return Result::Restart;
	}

	// 右：タイトルへ戻る
	if (titleButton_ &&
		UpdateButton(
			titleButton_.get(),
			kTitleButtonPosition,
			kButtonSize,
			mousePosition,
			mouseTriggered))
	{
		return Result::GoToTitle;
	}

	return Result::None;
}

void PauseMenu::Draw()
{
	if (!isPaused_)
	{
		return;
	}

	// 半透明背景を描画
	if (pauseBackground_)
	{
		pauseBackground_->Draw();
	}

	// その上に3つのボタンを描画
	if (resumeButton_)
	{
		resumeButton_->Draw();
	}

	if (restartButton_)
	{
		restartButton_->Draw();
	}

	if (titleButton_)
	{
		titleButton_->Draw();
	}
}


void PauseMenu::TogglePause()
{
	isPaused_ = !isPaused_;

	if (isPaused_)
	{
		TimeManager::GetInstance().Pause();
	}
	else
	{
		TimeManager::GetInstance().Resume();
	}
}

bool PauseMenu::UpdateButton(
	Sprite* sprite,
	const Vector2& centerPosition,
	const Vector2& baseSize,
	const Vector2& mousePosition,
	bool mouseTriggered)
{
	if (!sprite)
	{
		return false;
	}

	const float halfWidth = baseSize.x * 0.5f;

	// 横は今までどおり中央基準
	const float left = centerPosition.x - halfWidth;
	const float right = centerPosition.x + halfWidth;

	// 縦は実際に表示されている位置に合わせる
	const float top = centerPosition.y;
	const float bottom = centerPosition.y + baseSize.y;

	const bool isHovered =
		mousePosition.x >= left &&
		mousePosition.x <= right &&
		mousePosition.y >= top &&
		mousePosition.y <= bottom;

	Vector2 drawSize = baseSize;

	if (isHovered)
	{
		drawSize.x *= kHoverScale;
		drawSize.y *= kHoverScale;
	}

	sprite->SetPosition(centerPosition);
	sprite->SetSize(drawSize);
	sprite->Update();

	return isHovered && mouseTriggered;
}