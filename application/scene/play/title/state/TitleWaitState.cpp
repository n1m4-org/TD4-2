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
    if (input && (input->TriggerKey(DIK_SPACE) || input->TriggerKey(DIK_RETURN)))
    {
        scene.ChangeState("Exit");
    }
}

const std::string& TitleWaitState::GetName() const
{
    static const std::string name = "Wait";
    return name;
}
