#include "PlayerMoveComponent.h"
#include "PlayerInputComponent.h"
#include "../common/PhysicsComponent.h"
#include "engine/gameobject/base/GameObject.h"
#include "base/Camera.h"
#include <numbers>
#include <cmath>

GameObjectComponent::PlayerMoveComponent::PlayerMoveComponent(Camera* camera)
	: camera_(camera)
{
	// メンバ変数をJSONエディタ/シリアライズ用に登録
	Register("moveSpeed", &moveSpeed_);
	Register("turnSpeed", &turnSpeed_);
}

void GameObjectComponent::PlayerMoveComponent::Update(GameObject* owner)
{
	// 初回Update時に必要なコンポーネントのポインタを取得
	if (!input_)
	{
		input_ = owner->GetComponent<PlayerInputComponent>().get();
	}
	if (!physics_)
	{
		physics_ = owner->GetComponent<PhysicsComponent>().get();
	}

	// 入力コンポーネントと物理コンポーネントが取得できていない場合は処理を中断
	if (!input_ || !physics_)
	{
		return;
	}

	// 入力に基づいて移動方向を取得
	const Vector3& moveDirection = input_->GetMoveDirection();

	// カメラのY軸回転を取得
	float yaw = camera_->GetRotate().y;

	// カメラの基準方向を計算（Y軸回転のみを考慮）
	Vector3 forward = {std::sin(yaw), 0.0f, std::cos(yaw)};
	Vector3 right = {std::cos(yaw), 0.0f, -std::sin(yaw)};

	// 入力方向をカメラ基準に変換
	Vector3 transformedDirection = (moveDirection.x * right) + (moveDirection.z * forward);

	// 移動入力がある場合のみ処理
	float lengthSq = transformedDirection.LengthSquared();
	if (lengthSq > 0.001f)
	{
		transformedDirection.NormalizeSelf(); // 正規化して方向ベクトルにする

		// 進行方向に基づいて目標の向き（Y軸回転）を計算
		float targetYaw = std::atan2(transformedDirection.x, transformedDirection.z);

		// 現在の回転を取得して最短で目標の向きに旋回させる
		float currentYaw = owner->GetRotation().y;
		float diff = targetYaw - currentYaw;

		// 最短角度補正 (-PI 〜 PI)
		while (diff < -std::numbers::pi_v<float>) diff += 2.0f * std::numbers::pi_v<float>;
		while (diff > std::numbers::pi_v<float>) diff -= 2.0f * std::numbers::pi_v<float>;

		// 旋回速度を適用して徐々に向きを変える
		float nextYaw = currentYaw + diff * turnSpeed_;
		owner->SetRotation({0.0f, nextYaw, 0.0f});

		// 移動速度を適用
		transformedDirection *= moveSpeed_;
		physics_->SetMovementVelocity(transformedDirection);
	}
	else
	{
		// 入力がない時は移動速度を0にする
		physics_->SetMovementVelocity({0.0f, 0.0f, 0.0f});
	}
}