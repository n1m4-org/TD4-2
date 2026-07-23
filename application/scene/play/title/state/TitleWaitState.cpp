#include "TitleWaitState.h"
#include "application/scene/play/title/TitleScene.h"
#include "engine/scene/interface/BaseScene.h"
#include "input/Input.h"
#include "audio/Audio.h"

void TitleWaitState::OnEnter(BaseScene& scene)
{
	Audio::GetInstance()->PlayWave("TitleBGM", true);
	// BGMの音量を調整（0.0f～1.0f）
	Audio::GetInstance()->SetVolume("TitleBGM", 0.5f);
}

void TitleWaitState::OnUpdate(BaseScene& scene)
{
    // スタート待ち中の処理（ボタン点滅など）
}

void TitleWaitState::CheckTransition(BaseScene& scene)
{
    auto input = Input::GetInstance();
	bool isTriggered = false;
	#ifdef _DEBUG
	if (input && (input->TriggerKey(DIK_SPACE)))
	{
		isTriggered = true;
	}
	#else
	if (input && (input->TriggerKey(DIK_SPACE) || input->IsMouseButtonTriggered(0)))
	{
		isTriggered = true;
	}
	#endif

	if (isTriggered)
	{
		if (auto title = dynamic_cast<TitleScene*>(&scene))
		{
			title->OnDecision();
		}
		scene.ChangeState("Exit");
	}
}

const std::string& TitleWaitState::GetName() const
{
    static const std::string name = "Wait";
    return name;
}
