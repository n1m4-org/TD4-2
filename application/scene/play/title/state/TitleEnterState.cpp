#include "TitleEnterState.h"
#include "application/scene/play/title/TitleScene.h"
#include "engine/scene/interface/BaseScene.h"
#include "math/VectorColorCodes.h"

namespace
{
	constexpr float kEnterDuration = 2.0f; // 登場演出所要秒数
}

void TitleEnterState::OnEnter(BaseScene& scene)
{
	auto title = static_cast<TitleScene*>(&scene);
	title->GetTransitionEffect().SetFadeType(FadeType::FadeOut);
	title->GetTransitionEffect().SetEaseType(SceneTransitionEase::OutSine);
	title->GetTransitionEffect().SetMode(TransitionMode::TopToBottom);
	title->GetTransitionEffect().Start(kEnterDuration, VectorColorCodes::White, VectorColorCodes::White);
}

void TitleEnterState::OnUpdate(BaseScene& scene)
{
}

void TitleEnterState::CheckTransition(BaseScene& scene)
{
	if (auto title = static_cast<TitleScene*>(&scene))
	{
		if (title->GetTransitionEffect().GetState() == TransitionState::Done)
		{
			scene.ChangeState("Wait");
		}
	}
}

const std::string& TitleEnterState::GetName() const
{
	static const std::string name = "Enter";
	return name;
}
