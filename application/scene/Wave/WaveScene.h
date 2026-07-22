#pragma once
#include <memory>

#include "engine/scene/interface/BaseScene.h"
#include "application/Waves/WaveSystem.h"
#include "engine/gameobject/base/GameObject.h"

class WaveScene : public BaseScene
{
	std::unique_ptr<WaveSystem> waveSystem_;
	// 敵のスポーン位置が分かりやすいようにTestSceneから持ってきた地面
	std::unique_ptr<GameObject> groundObject_;

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
