#include "BulletBehaviorComponent.h"

#include "engine/gameobject/base/GameObject.h"
#include "engine/time/TimeManager.h"

BulletBehaviorComponent::BulletBehaviorComponent(const Vector3& velocity, float lifetime)
	: velocity_(velocity)
	, lifetime_(lifetime)
{
}

void BulletBehaviorComponent::Update(GameObject* owner)
{
	float deltaTime = TimeManager::GetInstance().GetGameContext().deltaTime;

	// 移動処理
	Vector3 pos = owner->GetPosition();
	pos += velocity_ * deltaTime; // deltaTimeはゲームタイマー等から取得
	owner->SetPosition(pos);

	 // 寿命による消滅
	lifetime_ -= deltaTime;
	if (lifetime_ <= 0.0f)
	{
		// オブジェクトを非アクティブ化
		owner->Destroy();

	}
}
