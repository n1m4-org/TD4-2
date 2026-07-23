#pragma once

#include "scene/interface/BaseScene.h"
#include "camerawork/debug/DebugCamera.h"
#include "engine/gameobject/base/GameObject.h"
#include "engine/camerawork/topdown/TopDownCamera.h"
#include "engine/gameobject/component/collision/SphereColliderComponent.h"
#include "application/scene/ui/MenuButton.h"
#include "engine/graphics/2d/Sprite.h"
#include "application/scene/ui/PauseMenu.h"


/**
 * @brief ゲームオブジェクトの動作確認を行うデバッグ用シーン。
 */
class TestScene : public BaseScene
{
public:
	void Initialize() override;
	void Draw3D() override;
	void Draw2D() override;
	void DrawShadow() override;
	void DrawGBuffer() override;

#ifdef USE_IMGUI
	void DrawImGui();
#endif

protected:
	void CommonUpdate() override;
	void OnFinalize() override;

private:
	/**
	 * @brief ボムエネミーを生成して必要なコンポーネントを設定する。
	 */
	void InitializeBombEnemy();

	// 演出用カメラ
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

	// ディレクショナルライト設定
	static constexpr Vector3 kLightDirection = { -0.2f, -1.0f, 0.3f };
	static constexpr float kLightIntensity = 0.6f;
	static constexpr Vector3 kBombEnemyPosition = { 50.0f, 2.0f, 0.0f };
	static constexpr Vector3 kBombEnemyScale = { 2.0f, 2.0f, 2.0f };
	static constexpr Vector3 kReflectHandLocalPosition = {0.0f, 0.0f, 1.25f};
	static constexpr Vector3 kReflectHandLocalScale = { 0.5f, 0.5f, 0.5f};

	// デバッグカメラ
	std::unique_ptr<DebugCamera> debugCamera_;
	// 追従カメラ
	std::unique_ptr<TopDownCamera> topDownCamera_;

	// ポーズメニュー
	std::unique_ptr<PauseMenu> pauseMenu_;

	// テスト用のゲームオブジェクト
	std::unique_ptr<GameObject> player_;
	std::unique_ptr<GameObject> groundObject_;
	std::unique_ptr<GameObject> targetObject_;
	std::unique_ptr<GameObject> bumper_;
	std::unique_ptr<GameObject> chargeEnemy_;
	std::unique_ptr<GameObject> bombEnemy_;
	std::unique_ptr<GameObject> hormingTest_;

	// 演出用
	enum class CameraState
	{
		Intro,
		Playing,
		Clear,
		GameOver,
	};
	CameraState cameraState_ = CameraState::Intro;
	float cameraTimer_ = 0.0f;
	// 演出時間
	const float kIntroTime = 2.0f;
	const float kGameOverTime = 2.5f;
	// 演出中かどうかのフラグ
	bool isIntroPlaying_ = true;
	bool isGameOverPlaying_ = false;

	// エフェクトタイマー
	float effectTimer_ = 0.0f;
	// RGBシフトの強さ
	float rgbShiftStrength_ = 15.0f;

	// --------- クリア演出用 --------- //
	// クリア演出全体の時間
	const float kClearDirectionTime = 2.2f;

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
	// もう一度（TestSceneを再読み込み。両方で共通利用）
	std::unique_ptr<MenuButton> retryButton_;
	// ゲームを終了（アプリを閉じる。両方で共通利用）
	std::unique_ptr<MenuButton> quitButton_;
	// クリアUIを表示中かどうか（クリア演出終了で true）
	bool isClearUIVisible_ = false;
	// ゲームオーバーUIを表示中かどうか（ゲームオーバー演出終了で true）
	bool isGameOverUIVisible_ = false;

	// UIレイアウト（Sprite座標系 1920x1080 基準）
	// 画面中央のX
	static constexpr float kResultUICenterX = Sprite::kCoordinateWidth * 0.5f;
	// タイトル帯
	static constexpr Vector2 kResultTitlePos = { kResultUICenterX, 300.0f };
	static constexpr Vector2 kResultTitleSize = { 640.0f, 180.0f };
	// ボタン
	static constexpr Vector2 kResultButtonSize = { 380.0f, 130.0f };
	// ボタン行の中心Y
	static constexpr float kResultButtonRowY = 640.0f;
	// ボタン間の隙間
	static constexpr float kResultButtonGap = 140.0f;
};
