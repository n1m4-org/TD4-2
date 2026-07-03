#include "HormingMoveComponent.h"

#include "engine/gameobject/base/GameObject.h"
#include "engine/time/TimeManager.h"
#include "input/Input.h"

using namespace GameObjectComponent;

HormingMoveComponent::HormingMoveComponent(GameObject* target)
	: target_(target)
{
	Register("moveDuration", &moveDuration_);
	Register("sideOffset", &sideOffset_);
	Register("heightOffset", &heightOffset_);
	Register("slideOffset", &slideOffset_);
	Register("targetFollowRate", &targetFollowRate_);
}

void HormingMoveComponent::Update(GameObject* owner)
{
	if (!owner)
	{
		return;
	}

	// Hキーを押した瞬間にスプライン移動開始
	if (Input::GetInstance()->TriggerKey(DIK_H))
	{
		StartMove(owner);
	}

	if (isMoving_)
	{
		UpdateMove(owner);
	}
}

void HormingMoveComponent::StartMove(GameObject* owner)
{
	if (!owner || !target_)
	{
		return;
	}

	startPos_ = owner->GetPosition();
	endPos_ = target_->GetPosition();

	Vector3 toTarget = endPos_ - startPos_;

	// XZ平面上の進行方向を作る
	Vector3 flatDir = {toTarget.x, 0.0f, toTarget.z};

	float lengthSq =
		flatDir.x * flatDir.x +
		flatDir.y * flatDir.y +
		flatDir.z * flatDir.z;

	if (lengthSq <= 0.001f)
	{
		flatDir = {0.0f, 0.0f, -1.0f};
	}
	else
	{
		flatDir.NormalizeSelf();
	}

	// 進行方向に対して横方向のベクトル
	sideDir_ = {-flatDir.z, 0.0f, flatDir.x};

	// 3次ベジェの制御点
	// 元のカーブ処理を維持
	controlPos1_ = startPos_ + toTarget * 0.25f + sideDir_ * sideOffset_;
	controlPos1_.y += heightOffset_;

	controlPos2_ = startPos_ + toTarget * 0.75f - sideDir_ * sideOffset_;
	controlPos2_.y += heightOffset_ * 0.5f;

	moveTimer_ = 0.0f;
	isMoving_ = true;
}

void HormingMoveComponent::UpdateMove(GameObject* owner)
{
	if (!owner || !target_)
	{
		return;
	}

	float dt = TimeManager::GetInstance().GetGameContext().deltaTime;

	if (moveDuration_ <= 0.001f)
	{
		StopMove(owner);
		return;
	}

	moveTimer_ += dt;

	float t = moveTimer_ / moveDuration_;

	if (t >= 1.0f)
	{
		StopMove(owner);
		return;
	}

	// プレイヤーが動いた場合、終点を少しずつ現在のプレイヤー位置へ寄せる
	// ここで急に endPos_ = target_->GetPosition(); にするとガクつきやすいので補間する
	Vector3 targetPos = target_->GetPosition();

	endPos_ = endPos_ + (targetPos - endPos_) * targetFollowRate_;

	// 終点が変わった分、後半の制御点も少し追従させる
	// 前半の controlPos1_ はあまり動かさないことで、最初のカーブ感を残す
	Vector3 toCurrentEnd = endPos_ - startPos_;

	Vector3 targetControlPos2 = startPos_ + toCurrentEnd * 0.75f - sideDir_ * sideOffset_;
	targetControlPos2.y += heightOffset_ * 0.5f;

	controlPos2_ = controlPos2_ + (targetControlPos2 - controlPos2_) * targetFollowRate_;

	// 元の動きと同じく、全体を3秒かけてカーブ移動
	float easedT = EaseInOut(t);

	Vector3 pos = CubicBezier(
		startPos_,
		controlPos1_,
		controlPos2_,
		endPos_,
		easedT);

	// 横方向への追加スライド
	// t=0 と t=1 では0、途中で最大になる
	float slideRate = 1.0f - ((easedT * 2.0f - 1.0f) * (easedT * 2.0f - 1.0f));

	pos += sideDir_ * slideOffset_ * slideRate;

	owner->SetPosition(pos);
}

void HormingMoveComponent::StopMove(GameObject* owner)
{
	if (!owner)
	{
		return;
	}

	isMoving_ = false;
	moveTimer_ = 0.0f;

	// 最後は、その時点のターゲット位置に着地する
	if (target_)
	{
		owner->SetPosition(target_->GetPosition());
	}
	else
	{
		owner->SetPosition(endPos_);
	}
}

Vector3 HormingMoveComponent::CubicBezier(
	const Vector3& p0,
	const Vector3& p1,
	const Vector3& p2,
	const Vector3& p3,
	float t)
{
	float invT = 1.0f - t;

	return p0 * (invT * invT * invT) +
		   p1 * (3.0f * invT * invT * t) +
		   p2 * (3.0f * invT * t * t) +
		   p3 * (t * t * t);
}

float HormingMoveComponent::EaseInOut(float t)
{
	if (t < 0.0f)
	{
		t = 0.0f;
	}
	if (t > 1.0f)
	{
		t = 1.0f;
	}

	// smoothstep
	return t * t * (3.0f - 2.0f * t);
}