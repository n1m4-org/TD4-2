#include "BulletSpawnComponent.h"

#include "application/gameobject/GameObjectTag.h"
#include "engine/gameobject/base/GameObject.h"
#include "engine/gameobject/manager/GameObjectManager.h"

GameObjectComponent::BulletSpawnComponent::BulletSpawnComponent()
{
}

GameObject* GameObjectComponent::BulletSpawnComponent::Fire(const std::string& name, const Vector3& position, const Vector3& rotation)
{
	// 敵本体のロック候補と区別するため、生成時点では敵弾タグを設定する。
	GameObject* bullet = GameObjectManager::GetInstance()->CreateGameObject(name, GameObjectTag::EnemyBullet);

	bullet->SetName(name);
	bullet->SetModel("cube");
	bullet->SetScale({1.0f, 1.0f, 1.0f});
	bullet->SetPosition(position);
	bullet->SetRotation(rotation);

	return bullet;
}
