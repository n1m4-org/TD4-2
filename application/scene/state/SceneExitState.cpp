#include "SceneExitState.h"
#include "engine/scene/interface/BaseScene.h"
#include "engine/scene/manager/SceneManager.h"
#include "math/VectorColorCodes.h"

SceneExitState::SceneExitState(
    SceneTransitionEffect* transitionEffect,
    const std::string& nextSceneName,
    float duration,
    TransitionMode mode,
    SceneTransitionEase easeType)
    : transitionEffect_(transitionEffect)
    , nextSceneName_(nextSceneName)
    , duration_(duration)
    , mode_(mode)
    , easeType_(easeType)
{
}

void SceneExitState::OnEnter(BaseScene& scene)
{
    (void)scene;
    sceneChanged_ = false;
    if (transitionEffect_)
    {
        transitionEffect_->SetFadeType(FadeType::FadeIn);
        transitionEffect_->SetEaseType(easeType_);
        transitionEffect_->SetMode(mode_);
        transitionEffect_->Start(duration_, VectorColorCodes::White, VectorColorCodes::White);
    }
}

void SceneExitState::OnUpdate(BaseScene& scene)
{
    (void)scene;
}

void SceneExitState::CheckTransition(BaseScene& scene)
{
    if (sceneChanged_)
    {
        return;
    }

    bool shouldChange = false;
    if (transitionEffect_)
    {
        if (transitionEffect_->GetState() == TransitionState::Done)
        {
            shouldChange = true;
        }
    }
    else
    {
        shouldChange = true;
    }

    if (shouldChange)
    {
        sceneChanged_ = true;
        if (auto mgr = scene.GetSceneManager())
        {
            mgr->ChangeScene(nextSceneName_);
        }
    }
}

const std::string& SceneExitState::GetName() const
{
    static const std::string name = "Exit";
    return name;
}
