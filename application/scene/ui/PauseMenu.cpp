#include "PauseMenu.h"

#include "engine/graphics/2d/SpriteCommon.h"
#include "engine/time/TimeManager.h"
#include "input/Input.h"
#include "audio/Audio.h"

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

	auto IsHoveredRect = [](const Vector2& mouse, const Vector2& center, const Vector2& size)
	{
		const float halfW = size.x * 0.5f;
		const float halfH = size.y * 0.5f;
		return (mouse.x >= center.x - halfW && mouse.x <= center.x + halfW &&
				mouse.y >= center.y - halfH && mouse.y <= center.y + halfH);
	};

	const bool isResumeHoveredNow = IsHoveredRect(mousePosition, kResumeButtonPosition, kButtonSize);
	const bool isRestartHoveredNow = IsHoveredRect(mousePosition, kRestartButtonPosition, kButtonSize);
	const bool isTitleHoveredNow = IsHoveredRect(mousePosition, kTitleButtonPosition, kButtonSize);

	// ホバー開始瞬間だけ select 再生
	if (!resumeHovered_ && isResumeHoveredNow)
	{
		Audio::GetInstance()->PlayWave("select");
	}
	if (!restartHovered_ && isRestartHoveredNow)
	{
		Audio::GetInstance()->PlayWave("select");
	}
	if (!titleHovered_ && isTitleHoveredNow)
	{
		Audio::GetInstance()->PlayWave("select");
	}

	// 前回状態を更新
	resumeHovered_ = isResumeHoveredNow;
	restartHovered_ = isRestartHoveredNow;
	titleHovered_ = isTitleHoveredNow;

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
		// 効果音を鳴らす
		Audio::GetInstance()->SetVolume("check", 1.0f);
		Audio::GetInstance()->PlayWave("check");

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
		// 効果音を鳴らす
		Audio::GetInstance()->SetVolume("check", 1.0f);
		Audio::GetInstance()->PlayWave("check");

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
		// 効果音を鳴らす
		Audio::GetInstance()->SetVolume("check", 1.0f);
		Audio::GetInstance()->PlayWave("check");

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
	// ポーズ切り替え時に効果音を鳴らす
	Audio::GetInstance()->SetVolume("pause", 1.0f);
	Audio::GetInstance()->PlayWave("pause");

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
	const float halfHeight = baseSize.y * 0.5f;

	// スプライトは中心アンカーで描画しているので、判定も中心基準にそろえる
	const float left = centerPosition.x - halfWidth;
	const float right = centerPosition.x + halfWidth;

	const float top = centerPosition.y - halfHeight;
	const float bottom = centerPosition.y + halfHeight;

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