#pragma once
#include "engine/gameobject/component/base/IActionComponent.h"
#include "jsonEditor/JsonEditableBase.h"
#include "math/Vector3.h"

class GameObject;

namespace GameObjectComponent
{

class HormingMoveComponent : public IActionComponent
	, public JsonEditableBase
{
public:
	HormingMoveComponent(GameObject* target = nullptr);

	void Update(GameObject* owner) override;

	void SetTarget(GameObject* target) { target_ = target; }

private:
	void StartMove(GameObject* owner);
	void UpdateMove(GameObject* owner);
	void StopMove(GameObject* owner);

	// 3次ベジェ曲線の計算
	Vector3 CubicBezier(
		const Vector3& p0,
		const Vector3& p1,
		const Vector3& p2,
		const Vector3& p3,
		float t);

	float EaseInOut(float t);

private:
	// 追尾対象
	GameObject* target_ = nullptr;

	// スプライン用の制御点
	Vector3 startPos_ = {0.0f, 0.0f, 0.0f};
	Vector3 controlPos1_ = {0.0f, 0.0f, 0.0f};
	Vector3 controlPos2_ = {0.0f, 0.0f, 0.0f};
	Vector3 endPos_ = {0.0f, 0.0f, 0.0f};

	// 移動時間
	float moveDuration_ = 3.0f;
	float moveTimer_ = 0.0f;

	// 曲線の膨らみ
	float sideOffset_ = 10.0f;
	float heightOffset_ = 15.0f;

	// 移動中に横へ流す追加オフセット
	float slideOffset_ = 10.0f;

	// 横方向を保持
	Vector3 sideDir_ = {0.0f, 0.0f, 0.0f};

	// ターゲット位置への追従率
	// 大きいほどプレイヤーの移動に強く追従する
	float targetFollowRate_ = 0.15	f;

	bool isMoving_ = false;
};

} // namespace GameObjectComponent