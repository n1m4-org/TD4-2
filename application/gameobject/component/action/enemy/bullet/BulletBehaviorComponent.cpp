#include "BulletBehaviorComponent.h"

#include "application/collision/CollisionLayer.h"
#include "application/gameobject/GameObjectTag.h"
#include "application/gameobject/component/action/common/PhysicsComponent.h"
#include "engine/gameobject/base/GameObject.h"
#include "engine/gameobject/component/base/ICollisionComponent.h"
#include "engine/time/TimeManager.h"

namespace
{
constexpr float kDirectionEpsilonSq = 0.000001f;
}

BulletBehaviorComponent::BulletBehaviorComponent(float lifetime)
	: lifetime_(lifetime)
{
}

void BulletBehaviorComponent::Update(GameObject* owner)
{
	// 反射前後で共通の寿命を使い、画面外に残り続ける弾を防ぐ。
	lifetime_ -= TimeManager::GetInstance().GetGameContext().deltaTime;
	if (lifetime_ <= 0.0f)
	{
		owner->Destroy();
	}
}

bool BulletBehaviorComponent::Reflect(GameObject* owner, const Vector3& direction, float speed)
{
	// 同じ弾が複数回コールバックを受けても、反射状態への遷移は一度だけにする。
	if (!owner || isReflected_ || direction.LengthSquared() <= kDirectionEpsilonSq)
	{
		return false;
	}

	auto physics = owner->GetComponent<GameObjectComponent::PhysicsComponent>();
	auto collider = owner->GetComponent<GameObjectComponent::ICollisionComponent>();
	if (!physics || !collider)
	{
		return false;
	}

	Vector3 normalizedDirection = direction;
	normalizedDirection.NormalizeSelf();

	// コールバック内では弾自身の移動・当たり判定・タグだけを切り替える。
	physics->SetMovementVelocity(normalizedDirection * speed);
	collider->SetCollisionLayer(CollisionLayer::PlayerBullet);
	collider->SetCollisionMask(CollisionLayer::Enemy | CollisionLayer::Bumpers);
	owner->SetTag(GameObjectTag::PlayerBullet);
	isReflected_ = true;
	return true;
}
