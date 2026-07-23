#pragma once
#include "engine/graphics/2d/Sprite.h"

class SpriteCommon;

class PauseMenu
{
public:

	// ポーズメニューからシーン側へ返す操作
	enum class Result
	{
		None,
		Restart,
		GoToTitle,
	};

	// 初期化
	void Initialize(SpriteCommon* spriteCommon);

	// ポーズメニューの更新、切り替え
	Result Update();

	// 描画
	void Draw();

	// ポーズ状態かどうか
	bool IsPaused() const { return isPaused_; }

private:

	// ポーズ状態の切り替え
	void TogglePause();

	// 指定したボタンのマウス判定と表示サイズ更新
	bool UpdateButton(
		Sprite* sprite,
		const Vector2& centerPosition,
		const Vector2& baseSize,
		const Vector2& mousePosition,
		bool mouseTriggered);

private:

	// ポーズ状態かどうか
	bool isPaused_ = false;

	// 背景スプライト
	std::unique_ptr<Sprite> pauseBackground_;

	// ポーズスプライト
	std::unique_ptr<Sprite> pauseText_;

	// ゲーム再開ボタン
	std::unique_ptr<Sprite> resumeButton_;

	// リスタートボタン
	std::unique_ptr<Sprite> restartButton_;

	// タイトルへ戻るボタン
	std::unique_ptr<Sprite> titleButton_;

	// ボタンのホバー状態
	bool resumeHovered_ = false;
	bool restartHovered_ = false;
	bool titleHovered_ = false;

	// ポーズのテキストの色
	Vector4 pauseTextColor_ = {1.0f, 1.0f, 1.0f, 1.0f};
	// ポーズのテキストの座標
	Vector2 pauseTextPosition_ = {960.0f, 200.0f};

};