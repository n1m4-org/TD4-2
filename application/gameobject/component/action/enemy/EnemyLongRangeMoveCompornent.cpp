#include "EnemyLongRangeMoveCompornent.h"

#include "engine/gameobject/base/GameObject.h"
#include "engine/time/TimeManager.h"
#include "math/Vector3.h"

GameObjectComponent::EnemyLongRangeMoveCompornent::EnemyLongRangeMoveCompornent(GameObject* _player)
	: player_(_player)
{
}

void GameObjectComponent::EnemyLongRangeMoveCompornent::Update(GameObject* owner)
{
	// タイマー更新
	strafeTimer_ += TimeManager::GetInstance().GetGameContext().deltaTime;

	if (strafeTimer_ >= changeTime_)
	{
		moveRight_ = !moveRight_;

		changeTime_ = Random(1.0f, 2.5f);

		strafeTimer_ = 0.0f;
	}

	// プレイヤーへの方向
	Vector3 toPlayer = player_->GetPosition() - owner->GetPosition();
	float distance = toPlayer.Length();
	toPlayer.NormalizeSelf();

	// 横方向
	Vector3 side = { -toPlayer.z, 0.0f, toPlayer.x};

	// 左右移動
	if (!moveRight_)
	{
		side *= -1.0f;
	}

	owner->SetPosition(owner->GetPosition() + side * moveSpeed_ * TimeManager::GetInstance().GetGameContext().deltaTime);
}

float GameObjectComponent::EnemyLongRangeMoveCompornent::Random(float min, float max)
{
	return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}
