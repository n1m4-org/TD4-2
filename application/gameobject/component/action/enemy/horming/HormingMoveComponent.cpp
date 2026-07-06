#include "HormingMoveComponent.h"

#include "application/gameobject/component/action/common/PhysicsComponent.h"
#include "engine/gameobject/base/GameObject.h"
#include "input/Input.h"

using namespace GameObjectComponent;

HormingMoveComponent::HormingMoveComponent(GameObject* target)
	: target_(target)
{
	Register("moveSpeed", &moveSpeed_);
	Register("stopDistance", &stopDistance_);
}

void HormingMoveComponent::Update(GameObject* owner)
{
	if (!owner)
	{
		return;
	}

	// 初回だけPhysicsComponentを取得
	if (!physics_)
	{
		physics_ = owner->GetComponent<PhysicsComponent>().get();
	}

	if (!physics_)
	{
		return;
	}

	// Spaceキーを押した瞬間に、ターゲット位置へ移動開始
	if (Input::GetInstance()->TriggerKey(DIK_RETURN))
	{
		StartMove(owner);
	}

	// 移動中なら目的地へ向かって進む
	if (isMoving_)
	{
		UpdateMove(owner);
	}
}

void HormingMoveComponent::StartMove(GameObject* owner)
{
	if (!owner || !target_ || !physics_)
	{
		return;
	}

	// Spaceを押した瞬間のターゲット位置を目的地にする
	destination_ = target_->GetPosition();

	// 飛んでいる間は重力を切る
	prevUseGravity_ = physics_->GetUseGravity();
	physics_->SetUseGravity(false);

	isMoving_ = true;
}

void HormingMoveComponent::UpdateMove(GameObject* owner)
{
	if (!owner || !physics_)
	{
		return;
	}

	Vector3 currentPos = owner->GetPosition();
	Vector3 toDestination = destination_ - currentPos;

	float distanceSq =
		toDestination.x * toDestination.x +
		toDestination.y * toDestination.y +
		toDestination.z * toDestination.z;

	float stopDistanceSq = stopDistance_ * stopDistance_;

	// 目的地に十分近づいたら停止
	if (distanceSq <= stopDistanceSq)
	{
		StopMove(owner);
		return;
	}

	// 目的地方向へ進む
	toDestination.NormalizeSelf();

	Vector3 velocity = toDestination * moveSpeed_;
	physics_->SetMovementVelocity(velocity);
}

void HormingMoveComponent::StopMove(GameObject* owner)
{
	if (!owner || !physics_)
	{
		return;
	}

	isMoving_ = false;

	// 目的地にぴったり合わせる
	owner->SetPosition(destination_);

	// 速度を止める
	physics_->SetMovementVelocity({0.0f, 0.0f, 0.0f});
	physics_->SetExternalVelocity({0.0f, 0.0f, 0.0f});

	// 重力設定を戻す
	physics_->SetUseGravity(prevUseGravity_);
}