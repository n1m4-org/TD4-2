#include "DashMoveComponent.h"

#include "gameobject/base/GameObject.h"
#include "gameobject/component/action/common/PhysicsComponent.h"
#ifdef USE_IMGUI
#include "manager/editor/DebugUIManager.h"
#include "externals/imgui/imgui.h"
#endif
#include "time/TimeManager.h"
#include "audio/Audio.h"

namespace GameObjectComponent
{

	DashMoveComponent::DashMoveComponent(GameObject* _player)
		:player_(_player) 
	{
#ifdef USE_IMGUI
		DebugUIManager::GetInstance()->RegisterDebugUI(this, "DashMove", [this](){ Debug(); });
#endif
	}

	void DashMoveComponent::Update(GameObject* _owner)
	{

		if (!player_) return;

		switch (state_)
		{
		case State::Idle:
			Idle(_owner);
			break;
		case State::Dash:
			Dash(_owner);
			break;
		}

	}

	void DashMoveComponent::Idle(const GameObject* _owner)
	{
		if (elapsedTime_ > triggerTime_)
		{
			elapsedTime_ = 0.f;

			Vector3 toPlayer = player_->GetPosition() - _owner->GetPosition();
			dashDirection_ = toPlayer.Normalize();
			currentSpd_ = speed_;
			Audio::GetInstance()->PlayWave("se_spawn");
			state_ = State::Dash;
			return;
		}

		elapsedTime_ += TimeManager::GetInstance().GetGameContext().deltaTime;
	}

	void DashMoveComponent::Dash(const GameObject* _owner)
	{
		auto physics = _owner->GetComponent<PhysicsComponent>();
		if (!physics) return;

		const float dt = TimeManager::GetInstance().GetGameContext().deltaTime;
		elapsedTime_ += dt;
		float brakeForce = BrakeCoefficient * elapsedTime_ * elapsedTime_;

		currentSpd_ -= brakeForce * dt;

		if (currentSpd_ <= MinSpd)
		{
			currentSpd_ = 0.f;
			elapsedTime_ = 0.f;
			state_ = State::Idle;
		}

		physics->SetMovementVelocity(dashDirection_ * currentSpd_);
	}

	void DashMoveComponent::Debug()
	{
#ifdef USE_IMGUI
		ImGui::DragFloat("#speed", &speed_);
#endif
	}
}