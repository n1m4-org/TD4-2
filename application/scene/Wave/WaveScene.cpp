#include "WaveScene.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#include "manager/editor/DebugUIManager.h"
#endif

void WaveScene::Initialize()
{
	waveSystem_ = std::make_unique<WaveSystem>();
	waveSystem_->Initialize();

#ifdef USE_IMGUI
	DebugUIManager::GetInstance()->RegisterDebugUI(this, "Wave Scene", [this]() { this->DrawImGui(); }, DebugUIArea::Console);
#endif

	StartState(SceneState::Playing);
}

void WaveScene::Finalize()
{
#ifdef USE_IMGUI
	if (DebugUIManager::HasInstance())
	{
		DebugUIManager::GetInstance()->UnregisterDebugUI(this);
	}
#endif
}

void WaveScene::Draw2D()
{
}

void WaveScene::OnUpdatePlaying()
{
	waveSystem_->Update(1.0f / 60.0f);
}

#ifdef USE_IMGUI
void WaveScene::DrawImGui()
{
	ImGui::Text("Wave: %u / %zu", waveSystem_->GetCurrentWaveIndex() + 1, waveSystem_->GetWaveCount());
	ImGui::Text("Completed: %s", waveSystem_->IsCompleted() ? "true" : "false");
}
#endif
