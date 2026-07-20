#include "BombMoveComponent.h"

#include "application/collision/CollisionLayer.h"
#include "application/gameobject/component/action/common/PhysicsComponent.h"
#include "application/gameobject/component/action/player/PlayerReflectComponent.h"
#include "application/gameobject/GameObjectTag.h"
#include "engine/effects/particle/ParticleManager.h"
#include "engine/gameobject/base/GameObject.h"
#include "engine/gameobject/component/base/ICollisionComponent.h"
#include "engine/time/TimeManager.h"
#include <cmath>

namespace
{
	constexpr float kDirectionEpsilonSq = 0.000001f;
}

GameObjectComponent::BombMoveComponent::BombMoveComponent(GameObject* player)
	: player_(player)
{
	Register("chaseRange", &chaseRange_);
	Register("chaseSpeed", &chaseSpeed_);
	Register("chaseLifetimeSeconds", &chaseLifetimeSeconds_);
	Register("reflectedSpeed", &reflectedSpeed_);
	Register("reflectedLifetimeSeconds", &reflectedLifetimeSeconds_);
	Register("turnRate", &turnRate_);
}

void GameObjectComponent::BombMoveComponent::Update(GameObject* owner)
{
	if (!InitializeComponents(owner))
	{
		return;
	}

	const float deltaTime = TimeManager::GetInstance().GetGameContext().deltaTime;
	// 行動を状態ごとに分け、追跡中と反射後の速度更新が混ざらないようにする。
	switch (state_)
	{
	case State::Idle:
		UpdateIdle(owner);
		break;
	case State::Chasing:
		UpdateChasing(owner, deltaTime);
		break;
	case State::Reflected:
		UpdateReflected(deltaTime);
		break;
	case State::Exploded:
		physics_->SetMovementVelocity({0.0f, 0.0f, 0.0f});
		break;
	}
}

bool GameObjectComponent::BombMoveComponent::Reflect(const Vector3& direction)
{
	// 多重反射と爆発後の再利用を拒否し、状態遷移を一度だけ行う。
	if (!owner_ || !physics_ || !collider_ ||
		state_ == State::Reflected || state_ == State::Exploded ||
		direction.LengthSquared() <= kDirectionEpsilonSq)
	{
		return false;
	}

	Vector3 normalizedDirection = direction;
	normalizedDirection.y = 0.0f;
	if (normalizedDirection.LengthSquared() <= kDirectionEpsilonSq)
	{
		return false;
	}

	normalizedDirection.NormalizeSelf();
	reflectedVelocity_ = normalizedDirection * reflectedSpeed_;
	remainingLifetimeSeconds_ = reflectedLifetimeSeconds_;
	state_ = State::Reflected;

	// 反射後はプレイヤー側の攻撃として敵へ当たるレイヤーに切り替える。
	physics_->SetMovementVelocity(reflectedVelocity_);
	collider_->SetCollisionLayer(CollisionLayer::PlayerBullet);
	collider_->SetCollisionMask(
		CollisionLayer::Enemy |
		CollisionLayer::Terrain |
		CollisionLayer::Bumpers);
	owner_->SetTag(GameObjectTag::PlayerBullet);
	return true;
}

bool GameObjectComponent::BombMoveComponent::InitializeComponents(GameObject* owner)
{
	if (!owner || !player_)
	{
		return false;
	}

	if (physics_ && collider_)
	{
		return true;
	}

	owner_ = owner;
	// 依存コンポーネントはownerが所有する。このコンポーネントは更新中だけ参照する。
	physics_ = owner->GetComponent<PhysicsComponent>().get();
	collider_ = owner->GetComponent<ICollisionComponent>().get();
	if (!physics_ || !collider_)
	{
		return false;
	}

	collider_->SetCollisionLayer(CollisionLayer::Enemy);
	collider_->SetCollisionMask(
		CollisionLayer::Player |
		CollisionLayer::Terrain |
		CollisionLayer::Bumpers |
		CollisionLayer::PlayerReflect);
	// コールバックではボム自身の状態と移動だけを変更する。
	collider_->SetOnEnter([this](const CollisionInfo& info)
	{
		HandleCollision(info);
	});
	collider_->SetOnStay([this](const CollisionInfo& info)
	{
		ResolveTerrainCollision(info);
	});
	collider_->SetOnExit([](const CollisionInfo&) {});
	return true;
}

void GameObjectComponent::BombMoveComponent::UpdateIdle(GameObject* owner)
{
	physics_->SetMovementVelocity({0.0f, 0.0f, 0.0f});

	const Vector3 toPlayer = player_->GetPosition() - owner->GetPosition();
	const float chaseRangeSq = chaseRange_ * chaseRange_;
	if (toPlayer.LengthSquared() <= chaseRangeSq)
	{
		state_ = State::Chasing;
		remainingLifetimeSeconds_ = chaseLifetimeSeconds_;

		// 追尾開始時の方向で走り出す（ドリフト用）
		Vector3 dir = toPlayer;
		dir.y = 0.0f;
		if (dir.LengthSquared() > kDirectionEpsilonSq)
		{
			dir.NormalizeSelf();
			dashDirection_ = dir;
		}
	}
}

void GameObjectComponent::BombMoveComponent::UpdateChasing(GameObject* owner, float deltaTime)
{
	remainingLifetimeSeconds_ -= deltaTime;
	if (remainingLifetimeSeconds_ <= 0.0f)
	{
		Explode();
		return;
	}

	Vector3 direction = player_->GetPosition() - owner->GetPosition();
	direction.y = 0.0f;
	if (direction.LengthSquared() <= kDirectionEpsilonSq)
	{
		physics_->SetMovementVelocity({0.0f, 0.0f, 0.0f});
		return;
	}
	direction.NormalizeSelf();

	// 進行方向を毎フレーム少しずつプレイヤー方向へ寄せる（ドリフト挙動）
	dashDirection_ += (direction - dashDirection_) * turnRate_;
	dashDirection_.NormalizeSelf();
	physics_->SetMovementVelocity(dashDirection_ * chaseSpeed_);

	// 進行方向を正面に向かせる
	owner->SetRotation({0.0f, std::atan2(dashDirection_.x, dashDirection_.z), 0.0f});

	// 残り時間に応じて赤点滅
	UpdateBlink(remainingLifetimeSeconds_ / chaseLifetimeSeconds_);
}

void GameObjectComponent::BombMoveComponent::UpdateReflected(float deltaTime)
{
	remainingLifetimeSeconds_ -= deltaTime;
	if (remainingLifetimeSeconds_ <= 0.0f)
	{
		Explode();
		return;
	}

	physics_->SetMovementVelocity(reflectedVelocity_);

	// 反射後も残り時間に応じて赤点滅
	UpdateBlink(remainingLifetimeSeconds_ / reflectedLifetimeSeconds_);
}

void GameObjectComponent::BombMoveComponent::Explode()
{
	if (state_ == State::Exploded)
	{
		return;
	}

	state_ = State::Exploded;
	physics_->SetMovementVelocity({0.0f, 0.0f, 0.0f});
	ParticleManager::GetInstance()->Play("bomber", owner_->GetPosition());
	collider_->SetActive(false);
	owner_->SetActive(false);
}

void GameObjectComponent::BombMoveComponent::UpdateBlink(float remainRatio)
{
	// 残りが減るほど位相の進みを速くする
	blinkPhase_ += blinkSpeedMin_ + (blinkSpeedMax_ - blinkSpeedMin_) * (1.0f - remainRatio);

	// sinが正の間だけ赤くする（パキッと切り替わる点滅）
	bool redOn = std::sin(blinkPhase_) > 0.0f;
	owner_->SetColor(redOn ? Vector4{1.0f, 0.2f, 0.2f, 1.0f}
						   : Vector4{1.0f, 1.0f, 1.0f, 1.0f});
}

void GameObjectComponent::BombMoveComponent::HandleCollision(const CollisionInfo& info)
{
	if (!info.otherCollider)
	{
		return;
	}

	const auto otherLayer = info.otherCollider->GetCollisionLayer();
	if (otherLayer & CollisionLayer::Terrain)
	{
		ResolveTerrainCollision(info);
	}

	if ((otherLayer & CollisionLayer::PlayerReflect) && info.other)
	{
		// プレイヤーから行き先だけ取得し、ボム自身のReflect APIで状態を切り替える。
		GameObject* player = info.other->GetParent();
		auto reflect = player ? player->GetComponent<PlayerReflectComponent>() : nullptr;
		if (reflect)
		{
			Reflect(reflect->GetReflectDirectionFrom(owner_->GetPosition()));
		}
		return;
	}

	if (state_ == State::Reflected && (otherLayer & CollisionLayer::Enemy))
	{
		// 反射されたボムが敵へ届いた時点で爆発させる。
		Explode();
	}
}

void GameObjectComponent::BombMoveComponent::ResolveTerrainCollision(const CollisionInfo& info)
{
	if (!info.otherCollider ||
		!(info.otherCollider->GetCollisionLayer() & CollisionLayer::Terrain))
	{
		return;
	}

	Vector3 position = owner_->GetPosition();
	position += info.normal * info.depth;
	owner_->SetPosition(position);

	if (info.normal.y <= 0.0f)
	{
		return;
	}

	physics_->SetGrounded(true);
	Vector3 velocity = physics_->GetExternalVelocity();
	if (velocity.y < 0.0f)
	{
		velocity.y = 0.0f;
		physics_->SetExternalVelocity(velocity);
	}
}