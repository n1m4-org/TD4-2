#include "TitleExitState.h"
#include "application/scene/play/title/TitleScene.h"
#include "engine/scene/manager/SceneManager.h"
#include "engine/time/TimeManager.h"
#include "engine/time/TimerManager.h"
#include "engine/time/Timer.h"
#include "math/VectorColorCodes.h"

namespace
{
    constexpr float kExitDuration = 1.5f; // 次シーン遷移前演出所要秒数
}

void TitleExitState::OnEnter(BaseScene& scene)
{
	auto title = static_cast<TitleScene*>(&scene);
	title->GetTransitionEffect().SetFadeType(FadeType::FadeIn);
	title->GetTransitionEffect().SetEaseType(SceneTransitionEase::InSine);
	title->GetTransitionEffect().Start(kExitDuration, VectorColorCodes::Black, VectorColorCodes::White);
}

void TitleExitState::OnUpdate(BaseScene& scene)
{
    float dt = TimeManager::GetInstance().GetGameContext().deltaTime;
    timer_ += dt;
}

void TitleExitState::CheckTransition(BaseScene& scene)
{
	if (auto title = static_cast<TitleScene*>(&scene))
	{
		if (title->GetTransitionEffect().GetState() == TransitionState::Done)
		{
			auto timer = std::make_unique<Timer>("wait_change_scene", 0.3f, DeltaTimeType::DeltaTime);
			timer->SetOnFinish([title]()
			{
				title->GetSceneManager()->ChangeScene("Test");
			});
			TimerManager::GetInstance().AddTimer(std::move(timer));
		}
	}
}

const std::string& TitleExitState::GetName() const
{
    static const std::string name = "Exit";
    return name;
}
