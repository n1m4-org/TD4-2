#pragma once

#include "engine/gameobject/component/base/IActionComponent.h"
#include "engine/gameobject/manager/GameObjectManager.h"
#include "math/Vector3.h"

class BulletBehaviorComponent : public GameObjectComponent::IActionComponent
{
public:

	BulletBehaviorComponent(const Vector3& velocity, float lifetime);

	void Update(GameObject* owner) override;

	void SetVelocity(const Vector3& velocity) { velocity_ = velocity; }
	const Vector3& GetVelocity() const { return velocity_; }

private:
	Vector3 velocity_;
	float lifetime_ = 3.0f;
};
