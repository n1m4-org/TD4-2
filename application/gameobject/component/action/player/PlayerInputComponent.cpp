#include "PlayerInputComponent.h"
#include "input/Input.h"

namespace
{
constexpr int kLeftMouseButton = 0;
constexpr int kRightMouseButton = 2;
}

void GameObjectComponent::PlayerInputComponent::Update(GameObject* owner)
{
	// 移動方向のベクトル
	// 前後
	if (Input::GetInstance()->PushKey(DIK_W))
	{
		moveDirection_.z = 1.0f;
	}
	else if (Input::GetInstance()->PushKey(DIK_S))
	{
		moveDirection_.z = -1.0f;
	}
	else
	{
		moveDirection_.z = 0.0f;
	}

	// 左右
	if (Input::GetInstance()->PushKey(DIK_A))
	{
		moveDirection_.x = -1.0f;
	}
	else if (Input::GetInstance()->PushKey(DIK_D))
	{
		moveDirection_.x = 1.0f;
	}
	else
	{
		moveDirection_.x = 0.0f;
	}

	// 反射トリガーの判定
	if (Input::GetInstance()->TriggerKey(DIK_SPACE) ||
		Input::GetInstance()->IsMouseButtonTriggered(kLeftMouseButton))
	{
		isReflectTriggered_ = true;
	}
	else
	{
		isReflectTriggered_ = false;
	}

	// ロック対象の確定・切り替えは右クリックの立ち上がりで行う。
	if (Input::GetInstance()->IsMouseButtonTriggered(kRightMouseButton))
	{
		isLockOnTriggered_ = true;
	}
	else
	{
		isLockOnTriggered_ = false;
	}

	// スローモーショントリガーの判定
	if (Input::GetInstance()->TriggerKey(DIK_LSHIFT))
	{
		isSlowMotionTriggered_ = true;
	}
	else
	{
		isSlowMotionTriggered_ = false;
	}
}
