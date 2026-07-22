#include "SmashComponent.h"

#include "engine/effects/particle/ParticleManager.h"
#include "engine/gameobject/base/GameObject.h"

namespace GameObjectComponent
{
	SmashComponent::SmashComponent()
	{
		Register("effectName", &effectName_);
	}

	SmashComponent::~SmashComponent()
	{
		Stop();
	}

	void SmashComponent::Update(GameObject* owner)
	{
		// 再生中エフェクトの生存状態を追跡
		if (currentEffect_ && currentEffect_->IsPlaying())
		{
			currentEffect_->SetPosition(owner->GetPosition());
		}
	}

	void SmashComponent::Play(GameObject* owner)
	{
		// 位置が指定されていない場合、所有者の位置を使用
		if (owner)
		{
			// ParticleManager経由でワンショット再生
			currentEffect_ = ParticleManager::GetInstance()->Play(effectName_, owner->GetPosition());
		}
	}

	void SmashComponent::Stop()
	{
		if (currentEffect_)
		{
			currentEffect_->Stop();
		}
	}

	bool SmashComponent::IsPlaying() const
	{
		return currentEffect_ && currentEffect_->IsPlaying();
	}
} // namespace GameObjectComponent
