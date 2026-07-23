#pragma once
#include <memory>

#include "engine/scene/interface/BaseScene.h"
#include "application/Waves/WaveSystem.h"
#include "engine/gameobject/base/GameObject.h"
#include "engine/camerawork/topdown/TopDownCamera.h"
#include "application/transition/SceneTransitionEffect.h"
#include "application/scene/ui/PauseMenu.h"
#include "application/scene/ui/MenuButton.h"
#include "engine/graphics/2d/Sprite.h"

class WaveScene : public BaseScene
{
	std::unique_ptr<WaveSystem> waveSystem_;
	// 敵のスポーン位置が分かりやすいようにTestSceneから持ってきた地面
	std::unique_ptr<GameObject> groundObject_;
	// TestSceneから持ってきたプレイヤーと追従カメラ
	std::unique_ptr<GameObject> player_;
	std::unique_ptr<TopDownCamera> topDownCamera_;

	// 演出用カメラ(TestSceneと同様)
	enum class CameraState
	{
		Intro,
		Playing,
		Clear,
		GameOver,
	};
	CameraState cameraState_ = CameraState::Intro;
	float cameraTimer_ = 0.0f;
	const float kIntroTime = 2.0f;
	const float kGameOverTime = 2.5f;

	void UpdateCamera();
	void UpdateIntroCamera();
	void UpdateFollowCamera();
	void UpdateGameOverCamera();

	// ゲームオーバー演出
	void GameOverDirection();

	// クリア演出
	void StartClearDirection();
	void UpdateClearDirection();

	// 結果UI（クリア／ゲームオーバー演出終了後に表示するオーバーレイ）
	void InitializeResultUI();
	void UpdateResultUI();
	void DrawResultUI(Sprite* titleSprite);

	// エフェクトタイマー
	float effectTimer_ = 0.0f;
	// RGBシフトの強さ
	float rgbShiftStrength_ = 15.0f;

	// --------- クリア演出用 --------- //
	// カメラが正面へ移動する時間
	const float kClearCameraMoveTime = 0.8f;
	// プレイヤーが回転・ジャンプする時間
	const float kClearPlayerActionTime = 1.2f;
	// カメラの正面距離
	const float kClearCameraDistance = 30.0f;
	// カメラの高さ
	const float kClearCameraHeight = 6.0f;
	// ジャンプの最大高さ
	const float kClearJumpHeight = 5.0f;

	// クリア開始時の状態
	Vector3 clearStartCameraPosition_ = {};
	Vector3 clearStartCameraRotation_ = {};
	Vector3 clearPlayerBasePosition_ = {};
	Vector3 clearPlayerBaseRotation_ = {};
	bool isClearDirectionStarted_ = false;

	// --------- 結果UI用（クリア／ゲームオーバー共通） --------- //
	// 背景の暗幕（両方で共通利用）
	std::unique_ptr<Sprite> resultBackground_;
	// 「クリア！」の帯（仮画像・後で文字画像へ差し替え予定）
	std::unique_ptr<Sprite> clearTitleSprite_;
	// 「ゲームオーバー」の帯（仮画像・後で文字画像へ差し替え予定）
	std::unique_ptr<Sprite> gameOverTitleSprite_;
	// もう一度（Waveを再読み込み。両方で共通利用）
	std::unique_ptr<MenuButton> retryButton_;
	// ゲームを終了（アプリを閉じる。両方で共通利用）
	std::unique_ptr<MenuButton> quitButton_;
	// クリアUIを表示中かどうか（クリア演出終了で true）
	bool isClearUIVisible_ = false;
	// ゲームオーバーUIを表示中かどうか（ゲームオーバー演出終了で true）
	bool isGameOverUIVisible_ = false;

	// UIレイアウト（Sprite座標系 1920x1080 基準。画面内で大きく見えるよう調整）
	// 画面中央のX
	static constexpr float kResultUICenterX = Sprite::kCoordinateWidth * 0.5f;
	// タイトル画像の中心位置
	static constexpr Vector2 kResultTitlePos = { kResultUICenterX, 330.0f };
	// タイトル画像の表示高さ（幅は元画像のアスペクト比を保って算出する）
	static constexpr float kResultTitleHeight = 420.0f;
	// クリア画像 (ui/clear.png : 256x181)
	static constexpr Vector2 kClearTitleSize = { kResultTitleHeight * (256.0f / 181.0f), kResultTitleHeight };
	// ゲームオーバー画像 (ui/gameover.png : 483x181)
	static constexpr Vector2 kGameOverTitleSize = { kResultTitleHeight * (483.0f / 181.0f), kResultTitleHeight };
	// ボタン (ui/onemore.png, ui/end.png : 342x181 ≒ 1.89:1)。タイトル拡大に合わせて大きめに
	static constexpr Vector2 kResultButtonSize = { 460.0f, 243.0f };
	// ボタン行の中心Y
	static constexpr float kResultButtonRowY = 800.0f;
	// ボタン間の隙間
	static constexpr float kResultButtonGap = 160.0f;

	// ポーズメニュー
	std::unique_ptr<PauseMenu> pauseMenu_;

	// シーン遷移時にフェード演出
	SceneTransitionEffect transitionEffect_;
	// 退出演出ステート参照（所有しない）
	class SceneExitState* exitState_ = nullptr;

public:
	void Initialize() override;
	void OnFinalize() override;
	void Draw3D() override;
	void Draw2D() override;
	void DrawShadow() override;
	void DrawGBuffer() override;

#ifdef USE_IMGUI
	void DrawImGui();
#endif

protected:
	void CommonUpdate() override;
};
