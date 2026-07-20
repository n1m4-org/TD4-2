#include "HormingMoveComponent.h"
#include "application/gameobject/component/action/player/PlayerReflectComponent.h"

#include "application/gameobject/GameObjectTag.h"
#include "engine/gameobject/base/GameObject.h"
#include "engine/gameobject/manager/GameObjectManager.h"
#include "engine/time/TimeManager.h"
#include "input/Input.h"
#include "application/collision/CollisionLayer.h"
#include "engine/gameobject/component/collision/AABBColliderComponent.h"
#include "engine/gameobject/component/collision/CollisionManager.h"

#include <string>

using namespace GameObjectComponent;

HormingMoveComponent::HormingMoveComponent(GameObject* target)
	: target_(target)
{
	Register("bulletLifeTime", &bulletLifeTime_);
	Register("bulletScale", &bulletScale_);
	Register("sideOffset", &sideOffset_);
	Register("heightOffset", &heightOffset_);
	Register("slideOffset", &slideOffset_);
	Register("targetFollowRate", &targetFollowRate_);
	Register("cooldownTime", &cooldownTime_);
}

void HormingMoveComponent::Update(GameObject* owner)
{
	// 発射元、または追尾対象がない場合は処理しない
	if (!owner || !target_)
	{
		return;
	}

	float deltaTime = TimeManager::GetInstance().GetGameContext().deltaTime;

	UpdateBullets();

	// 発射クールタイムの更新
	if (isCooldown_)
	{
		cooldownTimer_ += deltaTime;
		if (cooldownTimer_ >= cooldownTime_)
		{
			cooldownTimer_ = 0.0f;
			isCooldown_ = false;
		}
	}

	// Hキーを押した瞬間にホーミング弾を発射
	if (Input::GetInstance()->TriggerKey(DIK_H) && !isCooldown_)
	{
		FireBullet(owner);
		isCooldown_ = true;
	}
}

void HormingMoveComponent::FireBullet(GameObject* owner)
{
	if (!owner || !target_)
	{
		return;
	}

	static uint32_t bulletCount = 0;
	std::string bulletName = "HomingBullet_" + std::to_string(bulletCount++);

	// 弾のGameObjectを作成
	// 生成直後から敵弾タグを付け、ロック対象の敵本体とは区別する。
	GameObject* bulletObject = GameObjectManager::GetInstance()->CreateGameObject(bulletName, GameObjectTag::EnemyBullet);

	if (!bulletObject)
	{
		return;
	}

	bulletObject->SetName(bulletName);
	bulletObject->SetModel("cube");
	bulletObject->SetScale({bulletScale_, bulletScale_, bulletScale_});

	Vector3 spawnPos = owner->GetPosition();
	spawnPos.y += 1.0f;
	bulletObject->SetPosition(spawnPos);

	// AABBコライダーの追加
	bulletObject->AddComponent("Collider", std::make_unique<AABBColliderComponent>(bulletObject));

	if (auto collider = bulletObject->GetComponent<AABBColliderComponent>())
	{
		// ホーミング弾は敵弾として扱う
		collider->SetCollisionLayer(CollisionLayer::EnemyBullet);

		// プレイヤー、バンパー、反射判定に当たるようにする
		collider->SetCollisionMask(
			CollisionLayer::Player |
			CollisionLayer::Bumpers |
			CollisionLayer::PlayerReflect);

		// ホーミング弾自身のコールバックでは、この弾の生存状態だけを変更する。
		collider->SetOnEnter([this, bulletObject](const CollisionInfo& info)
		{
			if (!info.otherCollider)
			{
				return;
			}

			// プレイヤー、バンパーに当たったら弾を消す
			if ((info.otherCollider->GetCollisionLayer() & CollisionLayer::Player) ||
				(info.otherCollider->GetCollisionLayer() & CollisionLayer::Bumpers))
			{
				KillBullet(bulletObject);
				return;
			}

			// 反射判定に当たった場合も、いったん弾を消す
			if (info.otherCollider->GetCollisionLayer() & CollisionLayer::PlayerReflect)
			{
				// 反射後の直線移動APIがないため、ホーミング弾は現状ここで消す。
				if (info.other)
				{
					GameObject* player = info.other->GetParent();
					if (auto reflect = player ? player->GetComponent<PlayerReflectComponent>() : nullptr)
					{
						reflect->NotifyReflectSucceeded();
					}
				}
				KillBullet(bulletObject);
				return;
			}
		});

		collider->SetOnStay([](const CollisionInfo& info) {});
		collider->SetOnExit([](const CollisionInfo& info) {});
	}

	HomingBullet bullet;
	bullet.object = bulletObject;
	bullet.lifeTime = bulletLifeTime_;
	bullet.timer = 0.0f;
	bullet.isDead = false;

	// ベジェ曲線用の開始点・終点・制御点を作成
	InitializeBulletCurve(bullet);

	GameObjectManager::GetInstance()->Register(bulletObject);

	bullets_.push_back(bullet);
}

void HormingMoveComponent::InitializeBulletCurve(HomingBullet& bullet)
{
	if (!bullet.object || !target_)
	{
		return;
	}

	// ベジェ曲線の開始点と終点を決める
	bullet.startPos = bullet.object->GetPosition();
	bullet.endPos = target_->GetPosition();

	// 開始点からターゲットへの方向
	Vector3 toTarget = bullet.endPos - bullet.startPos;

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

	bullet.sideDir = {-flatDir.z, 0.0f, flatDir.x};

	// ベジェ曲線の制御点1
	// 序盤の曲がり方を決める
	bullet.controlPos1 = bullet.startPos + toTarget * 0.25f + bullet.sideDir * sideOffset_;
	bullet.controlPos1.y += heightOffset_;

	// ベジェ曲線の制御点2
	// 終盤の曲がり方を決める
	bullet.controlPos2 = bullet.startPos + toTarget * 0.75f - bullet.sideDir * sideOffset_;
	bullet.controlPos2.y += heightOffset_ * 0.5f;
}

void HormingMoveComponent::UpdateBullets()
{
	if (!target_)
	{
		return;
	}

	float deltaTime = TimeManager::GetInstance().GetGameContext().deltaTime;

	for (HomingBullet& bullet : bullets_)
	{
		if (!bullet.object || bullet.isDead)
		{
			continue;
		}

		bullet.timer += deltaTime;

		float t = bullet.timer / bullet.lifeTime;

		// 寿命が切れたら削除
		if (t >= 1.0f)
		{
			KillBullet(bullet.object);
			continue;
		}

		// 現在のターゲット位置を取得
		Vector3 targetPos = target_->GetPosition();

		// 終点をターゲットの現在位置へ少しずつ寄せる
		bullet.endPos = bullet.endPos + (targetPos - bullet.endPos) * targetFollowRate_;

		Vector3 toCurrentEnd = bullet.endPos - bullet.startPos;

		Vector3 targetControlPos2 =
			bullet.startPos + toCurrentEnd * 0.75f - bullet.sideDir * sideOffset_;

		targetControlPos2.y += heightOffset_ * 0.5f;

		bullet.controlPos2 =
			bullet.controlPos2 + (targetControlPos2 - bullet.controlPos2) * targetFollowRate_;

		float easedT = EaseInOut(t);

		// ベジェ曲線上の位置を計算
		Vector3 pos = CubicBezier(
			bullet.startPos,
			bullet.controlPos1,
			bullet.controlPos2,
			bullet.endPos,
			easedT);

		float slideRate =
			1.0f - ((easedT * 2.0f - 1.0f) * (easedT * 2.0f - 1.0f));

		pos += bullet.sideDir * slideOffset_ * slideRate;

		bullet.object->SetPosition(pos);
	}
}

void HormingMoveComponent::KillBullet(GameObject* bulletObject)
{
	if (!bulletObject)
	{
		return;
	}

	for (HomingBullet& bullet : bullets_)
	{
		if (bullet.object == bulletObject)
		{
			bullet.isDead = true;

			// Destroyは一回だけ呼ぶ
			bullet.object->Destroy();

			// 以降UpdateBulletsで触らないようにする
			bullet.object = nullptr;
			return;
		}
	}
}

Vector3 HormingMoveComponent::CubicBezier(
	const Vector3& p0,
	const Vector3& p1,
	const Vector3& p2,
	const Vector3& p3,
	float t)
{
	// 3次ベジェ曲線
	// p0: 開始点
	// p1: 制御点1
	// p2: 制御点2
	// p3: 終点
	// t : 進行度 0.0f ～ 1.0f

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

	return t * t * (3.0f - 2.0f * t);
}
