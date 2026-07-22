#pragma once

#include "scene/interface/BaseScene.h"
#include "camerawork/debug/DebugCamera.h"
#include "engine/gameobject/base/GameObject.h"
#include "engine/camerawork/topdown/TopDownCamera.h"
#include "engine/gameobject/component/collision/SphereColliderComponent.h"

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

	// クリア演出
	void StartClearDirection();
	void UpdateClearDirection();

	// ディレクショナルライト設定
	static constexpr Vector3 kLightDirection = { -0.2f, -1.0f, 0.3f };
	static constexpr float kLightIntensity = 0.6f;
	static constexpr Vector3 kBombEnemyPosition = { 50.0f, 2.0f, 0.0f };
	static constexpr Vector3 kBombEnemyScale = { 2.0f, 2.0f, 2.0f };
	static constexpr Vector3 kReflectHandLocalPosition = {0.0f, 0.0f, 1.25f};
	static constexpr Vector3 kReflectHandLocalScale = {1.5f, 0.35f, 0.35f};

	// デバッグカメラ
	std::unique_ptr<DebugCamera> debugCamera_;
	// 追従カメラ
	std::unique_ptr<TopDownCamera> topDownCamera_;

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
};
