#include "TrailComponent.h"

#include "engine/effects/particle/ParticleEmitter.h"
#include "engine/effects/particle/ParticleManager.h"
#include "engine/effects/particle/renderer/IRenderer.h"
#include "engine/gameobject/base/GameObject.h"
#include "math/VectorColorCodes.h"

namespace GameObjectComponent
{
	TrailComponent::TrailComponent(const std::string& effectName)
		: effectName_(effectName)
	{
		Register("effectName", &effectName_);
	}

	TrailComponent::~TrailComponent()
	{
		Stop();
	}

	void TrailComponent::Update(GameObject* owner)
	{
		// 再生中エフェクトの生存状態を追跡
		if (currentEffect_ && currentEffect_->IsPlaying())
		{
			currentEffect_->SetPosition(owner->GetPosition());
		}
		else
		{
			// エフェクトの再生を行う
			currentEffect_ = ParticleManager::GetInstance()->Play(effectName_, owner->GetPosition());
			SetColor(VectorColorCodes::Red);
		}
	}

	void TrailComponent::Play(GameObject* owner)
	{
		// 位置が指定されていない場合、所有者の位置を使用
		if (owner && currentEffect_ && !currentEffect_->IsPlaying())
		{
			// ParticleManager経由でワンショット再生
			currentEffect_ = ParticleManager::GetInstance()->Play(effectName_, owner->GetPosition());
			SetColor(VectorColorCodes::Red);
		}
	}

	void TrailComponent::Stop()
	{
		if (currentEffect_)
		{
			currentEffect_->Stop();
		}
	}

	bool TrailComponent::IsPlaying() const
	{
		return currentEffect_ && currentEffect_->IsPlaying();
	}

	void TrailComponent::SetColor(const Vector4& color)
	{
		if (currentEffect_)
		{
			for (size_t i = 0; i < currentEffect_->GetEmitterCount(); ++i)
			{
				if (auto emitter = currentEffect_->GetEmitter(i))
				{
					if (auto renderer = emitter->GetRenderer())
					{
						renderer->SetTintColor(color);
					}
				}
			}
		}
	}
} // namespace GameObjectComponent
