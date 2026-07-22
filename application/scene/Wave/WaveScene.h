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
	// 全Wave完了+敵全滅(疑似ゲームクリア)によるシーン遷移を一度だけ予約するためのフラグ
	bool clearSceneRequested_ = false;

public:
	void Initialize() override;
	void Finalize() override;
	void Draw3D() override;
	void Draw2D() override;
	void DrawShadow() override;
	void DrawGBuffer() override;

#ifdef USE_IMGUI
	void DrawImGui();
#endif

protected:
	void OnUpdatePlaying() override;
};
