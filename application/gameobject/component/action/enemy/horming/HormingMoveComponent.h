#pragma once
#include "engine/gameobject/component/base/IActionComponent.h"
#include "jsonEditor/JsonEditableBase.h"
#include "math/Vector3.h"

class GameObject;

namespace GameObjectComponent
{

class PhysicsComponent;

class HormingMoveComponent : public IActionComponent, public JsonEditableBase
{

public:
	
	HormingMoveComponent(GameObject* target = nullptr);

	void Update(GameObject* owner) override;

	void SetTarget(GameObject* target) { target_ = target; }

private:

	// 動き始め
	void StartMove(GameObject* owner);
	// 動きの更新
	void UpdateMove(GameObject* owner);
	// 動きの停止
	void StopMove(GameObject* owner);

private:
	// 追尾対象のGameObject
	GameObject* target_ = nullptr;
	PhysicsComponent* physics_ = nullptr;

	// Spaceを押した瞬間のターゲット位置
	Vector3 destination_ = {0.0f, 0.0f, 0.0f};

	// 移動速度
	float moveSpeed_ = 25.0f;
	//  止まるまでの距離
	float stopDistance_ = 0.2f;

	// 移動中かどうかのフラグ
	bool isMoving_ = false;
	// 
	bool prevUseGravity_ = true;

};
} // namespace GameObjectComponent