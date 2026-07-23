#include "SceneEnterState.h"
#include "engine/scene/interface/BaseScene.h"
#include "math/VectorColorCodes.h"

SceneEnterState::SceneEnterState(
    SceneTransitionEffect* transitionEffect,
    const std::string& nextStateName,
    float duration,
    TransitionMode mode,
    SceneTransitionEase easeType)
    : transitionEffect_(transitionEffect)
    , nextStateName_(nextStateName)
    , duration_(duration)
    , mode_(mode)
    , easeType_(easeType)
{
}

void SceneEnterState::OnEnter(BaseScene& scene)
{
    (void)scene;
    if (transitionEffect_)
    {
        transitionEffect_->SetFadeType(FadeType::FadeOut);
        transitionEffect_->SetEaseType(easeType_);
        transitionEffect_->SetMode(mode_);
        transitionEffect_->Start(duration_, VectorColorCodes::White, VectorColorCodes::White);
    }
}

void SceneEnterState::OnUpdate(BaseScene& scene)
{
    (void)scene;
}

void SceneEnterState::CheckTransition(BaseScene& scene)
{
    if (transitionEffect_)
    {
        if (transitionEffect_->GetState() == TransitionState::Done)
        {
            if (!nextStateName_.empty())
            {
                scene.ChangeState(nextStateName_);
            }
        }
    }
    else
    {
        // トランジションエフェクトがない場合は即座に次のステートへ遷移
        if (!nextStateName_.empty())
        {
            scene.ChangeState(nextStateName_);
        }
    }
}

const std::string& SceneEnterState::GetName() const
{
    static const std::string name = "Enter";
    return name;
}
