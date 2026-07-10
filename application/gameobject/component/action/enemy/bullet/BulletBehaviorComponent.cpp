#include "BulletBehaviorComponent.h"

#include "engine/gameobject/base/GameObject.h"
#include "engine/time/TimeManager.h"

BulletBehaviorComponent::BulletBehaviorComponent(float lifetime)
	: lifetime_(lifetime)
{
}

void BulletBehaviorComponent::Update(GameObject* owner)
{
	float deltaTime = TimeManager::GetInstance().GetGameContext().deltaTime;

	// 寿命による消滅
	lifetime_ -= deltaTime;
	if (lifetime_ <= 0.0f)
	{
		// オブジェクトを非アクティブ化
		owner->Destroy();
	}
}
