#include "TitleExitState.h"
#include "application/scene/play/title/TitleScene.h"
#include "engine/scene/manager/SceneManager.h"
#include "engine/time/TimeManager.h"

namespace
{
    constexpr float kExitDuration = 1.0f; // 次シーン遷移前演出所要秒数
}

void TitleExitState::OnEnter(BaseScene& scene)
{
    timer_ = 0.0f;
}

void TitleExitState::OnUpdate(BaseScene& scene)
{
    float dt = TimeManager::GetInstance().GetGameContext().deltaTime;
    timer_ += dt;
}

void TitleExitState::CheckTransition(BaseScene& scene)
{
    if (timer_ >= kExitDuration)
    {
        auto& titleScene = static_cast<TitleScene&>(scene);
        if (auto mgr = titleScene.GetSceneManager())
        {
            mgr->ChangeScene("Test");
        }
    }
}

const std::string& TitleExitState::GetName() const
{
    static const std::string name = "Exit";
    return name;
}
