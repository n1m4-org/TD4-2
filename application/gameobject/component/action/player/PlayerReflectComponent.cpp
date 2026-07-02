#include "PlayerReflectComponent.h"
#include "application/collision/CollisionLayer.h"
#include "application/gameobject/component/action/player/PlayerInputComponent.h"
#include "engine/effects/particle/ParticleManager.h"
#include "engine/gameobject/base/GameObject.h"
#include "engine/gameobject/component/base/ICollisionComponent.h"
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
			reflectTimer_ = 0.2f;		// 0.2秒間反射判定を出し続ける
			collider_->SetActive(true); // 反射判定を有効化

			// 先にコライダーの位置を正面に更新する
			float yaw = owner->GetRotation().y;
			Vector3 forward = {std::sin(yaw), 0.0f, std::cos(yaw)};
			forward.Normalize();
			Vector3 targetCenter = owner->GetPosition() + (forward * 3.0f);

			auto* sphereCollider = static_cast<SphereColliderComponent*>(collider_);
			Sphere s = sphereCollider->GetSphere();
			s.center = targetCenter;
			s.radius = 2.0f;
			sphereCollider->SetSphere(s);

			// 更新されたコライダーの位置を取得してエフェクトを再生する
			ParticleManager::GetInstance()->Play("reflect", sphereCollider->GetSphere().center);
		}
	}

	if (isReflecting_)
	{
		float deltaTime = TimeManager::GetInstance().GetGameContext().deltaTime;
		reflectTimer_ -= deltaTime;

		// 反射中はレイヤーを Player にして当たり判定を有効にする
		collider_->SetCollisionLayer(CollisionLayer::Player);

		// 反射中のみ、プレイヤーの正面にオフセットした位置を計算してコライダーを追従させる
		float yaw = owner->GetRotation().y;
		Vector3 forward = {std::sin(yaw), 0.0f, std::cos(yaw)};
		forward.Normalize();

		Vector3 targetCenter = owner->GetPosition() + (forward * 3.0f);

		// 球体コライダーの座標とサイズを上書き更新
		auto* sphereCollider = static_cast<SphereColliderComponent*>(collider_);
		Sphere s = sphereCollider->GetSphere();
		s.center = targetCenter;
		s.radius = 2.0f; // 反射判定の半径
		sphereCollider->SetSphere(s);

		if (reflectTimer_ <= 0.0f)
		{
			isReflecting_ = false;
			collider_->SetActive(false); // 反射終了時に非アクティブ化
			collider_->SetCollisionLayer(CollisionLayer::None);
		}
	}
}
