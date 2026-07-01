#include "PlayerReflectComponent.h"
#include "application/collision/CollisionLayer.h"
#include "application/gameobject/component/action/player/PlayerInputComponent.h"
#include "engine/gameobject/base/GameObject.h"
#include "engine/gameobject/component/collision/SphereColliderComponent.h"
#include "time/TimeManager.h"
#include <cmath>

void GameObjectComponent::PlayerReflectComponent::Update(GameObject* owner)
{
	if (!collider_)
	{
		// 初回Update時にコライダーコンポーネントのポインタを取得
		collider_ = owner->GetComponent<SphereColliderComponent>().get();
	}

	// コライダーコンポーネントが取得できていない場合は処理を中断
	if (!collider_)
	{
		return;
	}

	// 反射入力とタイマー処理
	auto input = owner->GetComponent<PlayerInputComponent>();
	if (input)
	{
		if (input->IsReflectTriggered() && !isReflecting_)
		{
			isReflecting_ = true;
			reflectTimer_ = 0.2f; // 0.2秒間反射判定を出し続ける
		}
	}

	if (isReflecting_)
	{
		float deltaTime = TimeManager::GetInstance().GetGameContext().deltaTime;
		reflectTimer_ -= deltaTime;

		// 反射中はレイヤーを Player (または反射用) にして当たり判定を有効にする
		collider_->SetCollisionLayer(CollisionLayer::Player);

		if (reflectTimer_ <= 0.0f)
		{
			isReflecting_ = false;
			// 通常時はNone にして誰とも当たらないようにする
			collider_->SetCollisionLayer(CollisionLayer::None);
		}
	}


	// プレイヤーの回転（Y軸）から正面方向ベクトルを算出
	float yaw = owner->GetRotation().y;
	Vector3 forward = {std::sin(yaw), 0.0f, std::cos(yaw)};
	forward.Normalize();

	// プレイヤーの正面にオフセットした位置を計算
	float offsetDistance = 2.5f; // プレイヤーの2.5m正面
	Vector3 targetCenter = owner->GetPosition() + (forward * offsetDistance);

	// 球体コライダーの座標とサイズを上書き更新
	auto* sphereCollider = static_cast<SphereColliderComponent*>(collider_);
	Sphere s = sphereCollider->GetSphere();
	s.center = targetCenter;
	s.radius = 2.0f; // 反射判定の半径
	sphereCollider->SetSphere(s);
}
