#pragma once
#include "engine/gameobject/component/base/IActionComponent.h"
#include "jsonEditor/JsonEditableBase.h"

// プレイヤーの前方宣言
class GameObject;

namespace GameObjectComponent
{
// 物理挙動コンポーネントの前方宣言
class PhysicsComponent;

class BombMoveComponent : public IActionComponent
	, public JsonEditableBase
{
public:
	// プレイヤーのポインタを受け取るコンストラクタ
	BombMoveComponent(GameObject* player);

	// 更新処理
	void Update(GameObject* owner) override;

	
public:	// Setter / Getter

	// プレイヤーの座標
	Vector3 SetPlayerPosition(const Vector3& position)
	{
		playerPosition_ = position;
		return playerPosition_;
	}


private:
	// プレイヤーのポインタを保持
	GameObject* player_ = nullptr;

	// プレイヤーの座標を保持
	Vector3 playerPosition_ = {0.0f, 0.0f, 0.0f};

	// 移動速度
	float moveSpeed_ = 10.0f;

	// 突進中かどうかのフラグ
	bool isDashing_ = false;

	// 突進を始めるまでの距離
	float dashRange_ = 5.0f;

	// 突進速度
	float dashSpeed_ = 10.0f;

	// 突進する方向
	Vector3 dashDirection_ = {0.0f, 0.0f, 0.0f};

	// 物理挙動コンポーネントのポインタを保持
	PhysicsComponent* physics_ = nullptr;

	// 追尾開始距離
	float chaseRange_ = 30.0f;

	// 追尾速度
	float chaseSpeed_ = 7.0f;

	// 点火から爆発までの時間
	const float ignitionTime_ = 300.0f;

	// 爆発までの残り時間
	float lifespan_ = ignitionTime_;
};

} // namespace GameObjectComponent