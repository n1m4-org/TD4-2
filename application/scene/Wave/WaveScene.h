#pragma once
#include <memory>

#include "engine/scene/interface/BaseScene.h"
#include "application/Waves/WaveSystem.h"

class WaveScene : public BaseScene
{
	std::unique_ptr<WaveSystem> waveSystem_;

public:
	void Initialize() override;
	void Finalize() override;
	void Draw2D() override;

#ifdef USE_IMGUI
	void DrawImGui();
#endif

protected:
	void OnUpdatePlaying() override;
};
