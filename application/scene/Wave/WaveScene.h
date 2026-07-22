#pragma once
#include <memory>

#include "engine/scene/interface/BaseScene.h"
#include "application/Waves/WaveSystem.h"
#include "engine/gameobject/base/GameObject.h"
#include "engine/camerawork/topdown/TopDownCamera.h"

class WaveScene : public BaseScene
{
	std::unique_ptr<WaveSystem> waveSystem_;
	// 敵のスポーン位置が分かりやすいようにTestSceneから持ってきた地面
	std::unique_ptr<GameObject> groundObject_;
	// TestSceneから持ってきたプレイヤーと追従カメラ
	std::unique_ptr<GameObject> player_;
	std::unique_ptr<TopDownCamera> topDownCamera_;
	// クリア/ゲームオーバーによるシーン遷移を一度だけ予約するためのフラグ
	bool sceneChangeRequested_ = false;
	// プレイヤーHP0(ゲームオーバー)を検知した際に立てるフラグ
	bool gameOverRequested_ = false;

	// 演出用カメラ(TestSceneのIntro演出と同様)
	enum class CameraState
	{
		Intro,
		Playing,
	};
	CameraState cameraState_ = CameraState::Intro;
	float cameraTimer_ = 0.0f;
	const float kIntroTime = 2.0f;

	void UpdateCamera();
	void UpdateIntroCamera();

public:
	void Initialize() override;
	void OnFinalize() override;
	void Draw3D() override;
	void Draw2D() override;
	void DrawShadow() override;
	void DrawGBuffer() override;

	void CommonUpdate() override;

#ifdef USE_IMGUI
	void DrawImGui();
#endif

protected:
	void CommonUpdate() override;
};
