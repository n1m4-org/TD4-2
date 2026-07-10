#pragma once

#include "engine/gameobject/component/base/IActionComponent.h"

#include "math/Vector3.h"

namespace GameObjectComponent
{

	class BulletSpawnComponent : public IActionComponent
	{
	public:

	   GameObject* Fire(const std::string& name,  const Vector3& position, const Vector3& rotation); 

	};

} // namespace GameObjectComponent