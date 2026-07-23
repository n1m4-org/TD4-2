#include "TitleWaitState.h"
#include "engine/scene/interface/BaseScene.h"
#include "input/Input.h"

void TitleWaitState::OnEnter(BaseScene& scene)
{
}

void TitleWaitState::OnUpdate(BaseScene& scene)
{
    // スタート待ち中の処理（ボタン点滅など）
}

void TitleWaitState::CheckTransition(BaseScene& scene)
{
    auto input = Input::GetInstance();
	#ifdef _DEBUG
	if (input && (input->TriggerKey(DIK_SPACE)))
	{
		scene.ChangeState("Exit");
	}
	#else
	if (input && (input->TriggerKey(DIK_SPACE) || input->IsMouseButtonTriggered(1)))
	{
		scene.ChangeState("Exit");
	}
	#endif
}

const std::string& TitleWaitState::GetName() const
{
    static const std::string name = "Wait";
    return name;
}
