#include "TitleEnterState.h"
#include "engine/scene/interface/BaseScene.h"
#include "engine/time/TimeManager.h"

namespace
{
    constexpr float kEnterDuration = 1.0f; // 登場演出所要秒数
}

void TitleEnterState::OnEnter(BaseScene& scene)
{
    timer_ = 0.0f;
}

void TitleEnterState::OnUpdate(BaseScene& scene)
{
    float dt = TimeManager::GetInstance().GetGameContext().deltaTime;
    timer_ += dt;
}

void TitleEnterState::CheckTransition(BaseScene& scene)
{
    if (timer_ >= kEnterDuration)
    {
        scene.ChangeState("Wait");
    }
}

const std::string& TitleEnterState::GetName() const
{
    static const std::string name = "Enter";
    return name;
}
