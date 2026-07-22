#pragma once
#include "gameobject/component/base/IActionComponent.h"
#include "math/Vector3.h"

namespace GameObjectComponent
{

	class DashMoveComponent : public IActionComponent
	{
		GameObject* player_ = nullptr;

		float elapsedTime_ = 0.f;
		float triggerTime_ = 3.f;

		Vector3 dashDirection_ = {};
		float speed_ = 50.f;

		enum class State { Idle, Dash };
		State state_ = State::Idle;

		float currentSpd_ = 0.f;
		const float Decay = 0.05f;
		const float MinSpd = 0.5f;

		float BrakeCoefficient = 600.f;

	public:
		DashMoveComponent(GameObject* _player);
		void Update(GameObject* _owner) override;

	private:
		void Idle(const GameObject* _owner);
		void Dash(const GameObject* _owner);
		void Debug();
	};

}