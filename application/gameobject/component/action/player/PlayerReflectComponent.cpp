#include "PlayerReflectComponent.h"

#include "application/collision/CollisionLayer.h"
#include "application/gameobject/GameObjectTag.h"
#include "application/gameobject/component/action/player/PlayerInputComponent.h"
#include "base/Camera.h"
#include "engine/effects/particle/ParticleManager.h"
#include "engine/gameobject/base/GameObject.h"
#include "engine/gameobject/component/collision/OBBColliderComponent.h"
#include "engine/gameobject/manager/GameObjectManager.h"
#include "engine/input/Input.h"
#include "engine/math/MathUtils.h"
#include "math/MatrixFunc.h"
#include "time/TimeManager.h"

#include <cmath>

namespace
{
constexpr float kReflectDurationSeconds = 0.2f;
constexpr float kReflectColliderOffset = 4.5f;
constexpr Vector3 kReflectColliderSize = {5.0f, 2.0f, 3.0f};
constexpr float kDirectionEpsilonSq = 0.000001f;
constexpr float kNdcMin = -1.0f;
constexpr float kNdcMax = 1.0f;
constexpr float kNdcToScreenScale = 0.5f;
constexpr float kNearNdc = 0.0f;
constexpr float kFarNdc = 1.0f;
constexpr char kLockOnMarkerTexture[] = "./Resources/uvChecker.png";
constexpr Vector2 kLockOnMarkerAnchor = {0.5f, 0.5f};
constexpr Vector2 kLockOnMarkerSize = {64.0f, 64.0f};
}

GameObjectComponent::PlayerReflectComponent::PlayerReflectComponent(
	Camera* camera, SpriteCommon* spriteCommon)
	: camera_(camera)
{
	Register("lockOnRadiusNdc", &lockOnRadiusNdc_);

	if (spriteCommon)
	{
		lockOnMarker_ = std::make_unique<Sprite>();
		lockOnMarker_->Initialize(spriteCommon, kLockOnMarkerTexture);
		lockOnMarker_->SetAnchorPoint(kLockOnMarkerAnchor);
		lockOnMarker_->SetSize(kLockOnMarkerSize);
	}
}

void GameObjectComponent::PlayerReflectComponent::Update(GameObject* owner)
{
	if (!owner || !camera_)
	{
		return;
	}

	if (!collider_)
	{
		collider_ = owner->GetComponent<OBBColliderComponent>().get();
	}

	if (!collider_)
	{
		return;
	}

	UpdateLockOnTarget();
	fallbackDirection_ = GetPlayerForward(owner);

	auto input = owner->GetComponent<PlayerInputComponent>();
	if (input && input->IsReflectTriggered() && !isReflecting_)
	{
		isReflecting_ = true;
		reflectTimer_ = kReflectDurationSeconds;
		hasReflectTarget_ = hasLockOnTarget_;
		reflectTargetPosition_ = lockOnTargetPosition_;

		collider_->SetActive(true);
		collider_->SetCollisionLayer(CollisionLayer::PlayerReflect);
		UpdateReflectCollider(owner);
		ParticleManager::GetInstance()->Play("reflect", collider_->GetOBB().center);
	}

	if (!isReflecting_)
	{
		return;
	}

	reflectTimer_ -= TimeManager::GetInstance().GetGameContext().deltaTime;
	UpdateReflectCollider(owner);

	if (reflectTimer_ <= 0.0f)
	{
		isReflecting_ = false;
		hasReflectTarget_ = false;
		collider_->SetActive(false);
		collider_->SetCollisionLayer(CollisionLayer::None);
	}
}

void GameObjectComponent::PlayerReflectComponent::Draw2D()
{
	if (hasLockOnTarget_ && lockOnMarker_)
	{
		lockOnMarker_->Draw();
	}
}

Vector3 GameObjectComponent::PlayerReflectComponent::GetReflectDirectionFrom(const Vector3& sourcePosition) const
{
	Vector3 direction = fallbackDirection_;
	if (hasReflectTarget_)
	{
		direction = reflectTargetPosition_ - sourcePosition;
		direction.y = 0.0f;
	}

	if (direction.LengthSquared() <= kDirectionEpsilonSq)
	{
		return fallbackDirection_;
	}

	direction.NormalizeSelf();
	return direction;
}

void GameObjectComponent::PlayerReflectComponent::UpdateLockOnTarget()
{
	hasLockOnTarget_ = false;

	const Vector2 mousePosition = Input::GetInstance()->GetMousePosition();
	const float clientWidth = static_cast<float>(WinApp::kClientWidth);
	const float clientHeight = static_cast<float>(WinApp::kClientHeight);
	const Vector2 mouseNdc = {
		(2.0f * mousePosition.x / clientWidth) - 1.0f,
		1.0f - (2.0f * mousePosition.y / clientHeight),
	};

	const float lockOnRadiusSq = lockOnRadiusNdc_ * lockOnRadiusNdc_;
	float nearestDistanceSq = lockOnRadiusSq;
	Vector2 markerPosition = {};

	for (GameObject* object : GameObjectManager::GetInstance()->GetGameObjects())
	{
		if (!object || object->GetTag() != GameObjectTag::Enemy ||
			!object->IsActive() || object->IsPendingDestroy())
		{
			continue;
		}

		const Vector3 enemyNdc = MathUtils::Transform(
			object->GetPosition(), camera_->GetViewProjectionMatrix());
		if (enemyNdc.x < kNdcMin || enemyNdc.x > kNdcMax ||
			enemyNdc.y < kNdcMin || enemyNdc.y > kNdcMax ||
			enemyNdc.z < kNearNdc || enemyNdc.z > kFarNdc)
		{
			continue;
		}

		const float differenceX = enemyNdc.x - mouseNdc.x;
		const float differenceY = enemyNdc.y - mouseNdc.y;
		const float distanceSq =
			(differenceX * differenceX) + (differenceY * differenceY);
		if (distanceSq > nearestDistanceSq)
		{
			continue;
		}

		nearestDistanceSq = distanceSq;
		lockOnTargetPosition_ = object->GetPosition();
		markerPosition = {
			(enemyNdc.x - kNdcMin) * kNdcToScreenScale * Sprite::kCoordinateWidth,
			(kNdcMax - enemyNdc.y) * kNdcToScreenScale * Sprite::kCoordinateHeight,
		};
		hasLockOnTarget_ = true;
	}

	if (hasLockOnTarget_ && lockOnMarker_)
	{
		lockOnMarker_->SetPosition(markerPosition);
		lockOnMarker_->Update();
	}
}

void GameObjectComponent::PlayerReflectComponent::UpdateReflectCollider(GameObject* owner)
{
	const Vector3 direction = GetReflectDirectionFrom(owner->GetPosition());
	const float yaw = std::atan2(direction.x, direction.z);

	OBB obb = collider_->GetOBB();
	obb.center = owner->GetPosition() + (direction * kReflectColliderOffset);
	obb.size = kReflectColliderSize;
	obb.rotate = MakeRotateMatrix({0.0f, yaw, 0.0f});
	collider_->SetOBB(obb);
}

Vector3 GameObjectComponent::PlayerReflectComponent::GetPlayerForward(const GameObject* owner) const
{
	const float yaw = owner->GetRotation().y;
	Vector3 forward = {std::sin(yaw), 0.0f, std::cos(yaw)};
	forward.NormalizeSelf();
	return forward;
}
