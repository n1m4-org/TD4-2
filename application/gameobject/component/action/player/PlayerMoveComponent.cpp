#include "PlayerMoveComponent.h"
#include "PlayerInputComponent.h"
#include "../common/PhysicsComponent.h"
#include "engine/gameobject/base/GameObject.h"
#include "base/Camera.h"
#include "engine/input/Input.h"
#include "engine/math/MathUtils.h"
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

	// プレイヤーの向きをマウスカーソルの方向に向ける
	Input* inputSystem = Input::GetInstance();
	Vector2 mousePos = inputSystem->GetMousePosition();
	float width = static_cast<float>(WinApp::kClientWidth);
	float height = static_cast<float>(WinApp::kClientHeight);

	// スクリーン座標をNDC座標に変換
	float mouseNdcX = (2.0f * mousePos.x / width) - 1.0f;
	float mouseNdcY = 1.0f - (2.0f * mousePos.y / height);

	// プレイヤーのワールド座標をNDC座標に変換
	Vector3 playerNdc = MathUtils::Transform(owner->GetPosition(), camera_->GetViewProjectionMatrix());

	// プレイヤーからマウスへの方向ベクトル（NDC空間）
	float diffX = mouseNdcX - playerNdc.x;
	float diffY = mouseNdcY - playerNdc.y;

	if (std::abs(diffX) > 1e-6f || std::abs(diffY) > 1e-6f)
	{
		//目標角度(Yaw)を計算
		float cameraYaw = camera_->GetRotate().y;
		float targetYaw = std::atan2(diffX, diffY) + cameraYaw;

		// 現在の回転を取得して最短で目標の向きに旋回させる
		float currentYaw = owner->GetRotation().y;
		float diff = targetYaw - currentYaw;

		// 最短角度補正 (-PI 〜 PI)
		while (diff < -std::numbers::pi_v<float>) diff += 2.0f * std::numbers::pi_v<float>;
		while (diff > std::numbers::pi_v<float>) diff -= 2.0f * std::numbers::pi_v<float>;

		// 旋回速度を適用して徐々に向きを変える
		float nextYaw = currentYaw + diff * turnSpeed_;
		owner->SetRotation({0.0f, nextYaw, 0.0f});
	}
	
	// 入力に基づいて移動方向を取得、カメラの基準方向を計算（Y軸回転のみを考慮）
	float yaw = camera_->GetRotate().y;
	const Vector3& moveDirection = input_->GetMoveDirection();
	Vector3 forward = {std::sin(yaw), 0.0f, std::cos(yaw)};
	Vector3 right = {std::cos(yaw), 0.0f, -std::sin(yaw)};

	// 入力方向をカメラ基準に変換
	Vector3 transformedDirection = (moveDirection.x * right) + (moveDirection.z * forward);

	// 移動入力がある場合のみ処理
	float lengthSq = transformedDirection.LengthSquared();
	if (lengthSq > 0.001f)
	{
		// 移動速度を適用
		transformedDirection.NormalizeSelf();		
		transformedDirection *= moveSpeed_;
		physics_->SetMovementVelocity(transformedDirection);
	}
	else
	{
		// 入力がない時は移動速度を0にする
		physics_->SetMovementVelocity({0.0f, 0.0f, 0.0f});
	}
}