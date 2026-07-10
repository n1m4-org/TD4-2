#pragma once

#include "engine/gameobject/component/base/IActionComponent.h"
#include "engine/gameobject/manager/GameObjectManager.h"
#include "math/Vector3.h"

class BulletBehaviorComponent : public GameObjectComponent::IActionComponent
{
public:

	BulletBehaviorComponent(float lifetime);

	void Update(GameObject* owner) override;

private:
	float lifetime_;
};
