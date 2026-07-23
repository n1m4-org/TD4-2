#include "TrailComponent.h"

#include "engine/effects/particle/ParticleEmitter.h"
#include "engine/effects/particle/ParticleManager.h"
#include "engine/effects/particle/renderer/IRenderer.h"
#include "engine/gameobject/base/GameObject.h"

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

			// SetColorで明示的に色指定されている場合のみ再適用する。
			// ParticleManagerのプールは前の所有者が変更した色を保持したまま返ってくることがあるため、
			// 色指定ありのTrailComponentが古い色を引き継がないようにする。
			// 指定が無い場合はプリセット（JSON）側の色をそのまま活かす。
			if (hasCustomColor_)
			{
				ApplyBaseColor();
			}
		}
	}

	void TrailComponent::Play(GameObject* owner)
	{
		if (!owner)
		{
			return;
		}

		// 既に再生中なら何もしない（多重トリガー防止）
		if (currentEffect_ && currentEffect_->IsPlaying())
		{
			return;
		}

		// ParticleManager経由でワンショット再生
		currentEffect_ = ParticleManager::GetInstance()->Play(effectName_, owner->GetPosition());

		if (hasCustomColor_)
		{
			ApplyBaseColor();
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
		// 以後、再生し直されてもこの色を維持できるよう基準色として保持する
		baseColor_ = color;
		hasCustomColor_ = true;
		ApplyBaseColor();
	}

	void TrailComponent::ApplyBaseColor()
	{
		if (!currentEffect_)
		{
			return;
		}

		for (size_t i = 0; i < currentEffect_->GetEmitterCount(); ++i)
		{
			if (auto emitter = currentEffect_->GetEmitter(i))
			{
				if (auto renderer = emitter->GetRenderer())
				{
					renderer->SetTintColor(baseColor_);
				}
			}
		}
	}
} // namespace GameObjectComponent
