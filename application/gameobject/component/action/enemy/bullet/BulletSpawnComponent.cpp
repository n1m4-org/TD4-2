#include "BulletSpawnComponent.h"

#include "engine/gameobject/base/GameObject.h"
#include "engine/gameobject/manager/GameObjectManager.h"

GameObject* GameObjectComponent::BulletSpawnComponent::Fire(const std::string& name, const Vector3& position, const Vector3& rotation)
{
	GameObject* bullet = GameObjectManager::GetInstance()->CreateGameObject(name, name);

	bullet->SetName(name);
	bullet->SetModel("cube");
	bullet->SetScale({1.0f, 1.0f, 1.0f});
	bullet->SetPosition(position);
	bullet->SetRotation(rotation);

	return bullet;
}
