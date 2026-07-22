#include "HormingMoveComponent.h"
#include "application/gameobject/component/action/enemy/EnemyDeathDirectionComponent.h"
#include "application/gameobject/component/action/player/PlayerReflectComponent.h"

#include "application/collision/CollisionLayer.h"
#include "application/gameobject/component/action/common/StatusComponent.h"
#include "application/gameobject/GameObjectTag.h"
#include "engine/gameobject/base/GameObject.h"
#include "engine/gameobject/component/collision/AABBColliderComponent.h"
#include "engine/gameobject/component/collision/CollisionManager.h"
#include "engine/gameobject/manager/GameObjectManager.h"
#include "engine/time/TimeManager.h"
#include "input/Input.h"

#include <algorithm>
#include <string>
#include "../../common/SmashComponent.h"

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

	Register("homingReleaseDistance", &homingReleaseDistance_);
	Register("straightSpeed", &straightSpeed_);

	Register("autoFireInterval", &autoFireInterval_);
	Register("burstCount", &burstCount_);
	Register("burstInterval", &burstInterval_);
}

void HormingMoveComponent::Update(GameObject* owner)
{
	// 発射元、または追尾対象がない場合は処理しない
	if (!owner || !target_)
	{
		return;
	}

	auto deathEffect =
		owner->GetComponent<EnemyDeathDirectionComponent>();

	// 死亡演出中は、この敵が発射した弾をすべて消して処理を止める
	if (deathEffect && deathEffect->IsDying())
	{
		KillAllBullets();

		// 発射待ちのバーストも中止する
		pendingBurstCount_ = 0;
		currentBurstIndex_ = 0;
		burstTimer_ = 0.0f;

		return;
	}

	// すでに発射済みの弾を更新
	UpdateBullets();

	// 一定間隔で自動発射
	UpdateAutoFire(owner);
}

void HormingMoveComponent::UpdateAutoFire(GameObject* owner)
{
	float deltaTime = TimeManager::GetInstance().GetGameContext().deltaTime;

	// バースト発射中なら、burstInterval_ ごとに1発ずつ撃つ
	if (pendingBurstCount_ > 0)
	{
		burstTimer_ += deltaTime;

		if (burstTimer_ >= burstInterval_)
		{
			burstTimer_ = 0.0f;

			FireBullet(owner, currentBurstIndex_, burstCount_);

			currentBurstIndex_++;
			pendingBurstCount_--;
		}

		return;
	}

	// 次の攻撃開始までの時間を進める
	autoFireTimer_ += deltaTime;

	if (autoFireTimer_ >= autoFireInterval_)
	{
		autoFireTimer_ = 0.0f;

		// 複数弾の発射開始
		pendingBurstCount_ = burstCount_;
		currentBurstIndex_ = 0;
		burstTimer_ = burstInterval_;
	}
}

void HormingMoveComponent::FireBullet(GameObject* owner, int32_t bulletIndex, int32_t bulletCount)
{
	if (!owner || !target_)
	{
		return;
	}

	static uint32_t bulletCountForName = 0;
	std::string bulletName = "HomingBullet_" + std::to_string(bulletCountForName++);

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

	// 跳ね返した時のエフェクト再生コンポーネント
	bulletObject->AddComponent("smash", std::make_unique<SmashComponent>());

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

			// 反射判定に当たったら、ロックオン方向へ跳ね返す
			if (info.other &&
				(info.otherCollider->GetCollisionLayer() & CollisionLayer::PlayerReflect))
			{
				// PlayerReflect のコライダーは ReflectHand についているので、
				// 親をたどってプレイヤー本体を取得する
				GameObject* player = info.other->GetParent();

				auto reflect = player ? player->GetComponent<PlayerReflectComponent>() : nullptr;

				if (!reflect)
				{
					return;
				}

				// チャージ弾と同じく、ロックオン方向を取得する
				const Vector3 direction =
					reflect->GetReflectDirectionFrom(bulletObject->GetPosition());

				// 反射方向から対象を特定し、プレイヤー弾としてホーミングさせる
				ReflectBullet(bulletObject, direction);

				// 反射成功演出
				reflect->NotifyReflectSucceeded();

				// 吹っ飛ばしエフェクトを再生
				if (auto smash = bulletObject->GetComponent<SmashComponent>())
				{
					smash->Play(bulletObject);
				}

				return;
			}

			// プレイヤー、バンパーに当たったら弾を消す
			if ((info.otherCollider->GetCollisionLayer() & CollisionLayer::Player) ||
				(info.otherCollider->GetCollisionLayer() & CollisionLayer::Bumpers))
			{
				KillBullet(bulletObject);
				return;
			}

			// 反射後に敵へ当たったら弾を消す
			if (info.otherCollider->GetCollisionLayer() & CollisionLayer::Enemy)
			{
				// 反射後に敵へ当たったため弾を消す
				KillBullet(bulletObject);
				return;
			}
		});

		collider->SetOnStay([](const CollisionInfo& info) {});
		collider->SetOnExit([](const CollisionInfo& info) {});
	}

	HomingBullet bullet;
	bullet.object = bulletObject;

	// 最初はプレイヤーを狙う
	bullet.target = target_;
	bullet.useVirtualTarget = false;
	bullet.virtualTargetPosition = {};

	bullet.lifeTime = bulletLifeTime_;
	bullet.timer = 0.0f;
	bullet.isStraight = false;
	bullet.straightDir = {};
	bullet.isReflected = false;
	bullet.isDead = false;

	// 複数弾の横方向差を作る
	float center = static_cast<float>(bulletCount - 1) * 0.5f;
	float offsetIndex = static_cast<float>(bulletIndex) - center;

	// 横方向の膨らみ倍率
	bullet.sidePower = 1.0f + offsetIndex * 0.45f;

	// 偶数・奇数で左右の流れを変え、同じ軌道になりすぎないようにする
	if (bulletIndex % 2 == 1)
	{
		bullet.sidePower *= -1.0f;
	}

	// ベジェ曲線用の開始点・終点・制御点を作成
	InitializeBulletCurve(bullet);

	GameObjectManager::GetInstance()->Register(bulletObject);

	bullets_.push_back(bullet);
}

void HormingMoveComponent::InitializeBulletCurve(HomingBullet& bullet)
{
	if (!bullet.object)
	{
		return;
	}

	if (!bullet.target && !bullet.useVirtualTarget)
	{
		return;
	}


	// ベジェ曲線の開始点と終点を決める
	bullet.startPos = bullet.object->GetPosition();

	// 終点を決める
	if (bullet.useVirtualTarget)
	{
		bullet.endPos = bullet.virtualTargetPosition;
	}
	else
	{
		bullet.endPos = bullet.target->GetPosition();
	}

	// 開始点からターゲットへの方向
	Vector3 toTarget = bullet.endPos - bullet.startPos;

	// 見下ろし視点なので、XZ平面上で横方向を計算する
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
	bullet.sideDir = {-flatDir.z, 0.0f, flatDir.x};

	float sidePowerOffset = sideOffset_ * bullet.sidePower;
	float slidePowerOffset = slideOffset_ * bullet.sidePower;

	// 横方向を強めにして、見下ろし視点でも避けやすい軌道にする
	bullet.controlPos1 =
		bullet.startPos +
		toTarget * 0.20f +
		bullet.sideDir * sidePowerOffset;

	bullet.controlPos1.y += heightOffset_;

	bullet.controlPos2 =
		bullet.startPos +
		toTarget * 0.70f -
		bullet.sideDir * sidePowerOffset;

	bullet.controlPos2.y += heightOffset_ * 0.5f;

	// slideOffset_も弾ごとに差を出したいので、sideDir側に反映する
	bullet.sideDir = bullet.sideDir * slidePowerOffset;
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

		// この弾が狙う対象がない場合は削除する
		if (!bullet.target && !bullet.useVirtualTarget)
		{
			KillBullet(bullet.object);
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

		// 現在の弾位置とターゲット位置
		Vector3 bulletPos = bullet.object->GetPosition();

		Vector3 targetPos = {};
		if (bullet.useVirtualTarget)
		{
			targetPos = bullet.virtualTargetPosition;
		}
		else
		{
			targetPos = bullet.target->GetPosition();
		}

		// まだホーミング中なら、一定距離以内でホーミング解除
		if (!bullet.isStraight && !bullet.isReflected)
		{
			Vector3 toTarget = targetPos - bulletPos;
			float distance = toTarget.Length();

			if (distance <= homingReleaseDistance_)
			{
				// この瞬間のターゲット方向を保存して、以降はその方向に直進する
				if (distance > 0.001f)
				{
					toTarget.NormalizeSelf();
					bullet.straightDir = toTarget;
				}
				else
				{
					// ほぼ重なっている場合の保険
					bullet.straightDir = {0.0f, 0.0f, 1.0f};
				}

				bullet.isStraight = true;
			}
		}

		// ホーミング解除後は、その時点の方向へまっすぐ進む
		if (bullet.isStraight)
		{
			Vector3 pos = bullet.object->GetPosition();
			pos += bullet.straightDir * straightSpeed_ * deltaTime;

			if (!bullet.object || bullet.isDead)
			{
				continue;
			}

			bullet.object->SetPosition(pos);
			continue;
		}

		// ホーミング処理

		// 終点をターゲットの現在位置へ少しずつ寄せる
		if (!bullet.useVirtualTarget)
		{
			bullet.endPos = bullet.endPos + (targetPos - bullet.endPos) * targetFollowRate_;
		}

		Vector3 toCurrentEnd = bullet.endPos - bullet.startPos;

		// 後半の制御点だけターゲット側へ少し追従させる
		Vector3 targetControlPos2 =
			bullet.startPos +
			toCurrentEnd * 0.70f -
			bullet.sideDir;

		targetControlPos2.y += heightOffset_ * 0.5f;

		bullet.controlPos2 =
			bullet.controlPos2 +
			(targetControlPos2 - bullet.controlPos2) * targetFollowRate_;

		float easedT = EaseBullet(t);

		// ベジェ曲線上の位置を計算
		Vector3 pos = CubicBezier(
			bullet.startPos,
			bullet.controlPos1,
			bullet.controlPos2,
			bullet.endPos,
			easedT);

		// 中盤だけ横に流す（見下ろし視点で軌道が分かりやすくなるようにする）
		float slideRate =
			1.0f - ((easedT * 2.0f - 1.0f) * (easedT * 2.0f - 1.0f));

		pos += bullet.sideDir * slideRate;

		// 衝突などで途中で消えていた場合の保険
		if (!bullet.object || bullet.isDead)
		{
			continue;
		}

		bullet.object->SetPosition(pos);
	}
}

void HormingMoveComponent::ReflectBullet(
	GameObject* bulletObject,
	const Vector3& reflectDirection)
{
	if (!bulletObject)
	{
		return;
	}

	for (HomingBullet& bullet : bullets_)
	{
		if (bullet.object != bulletObject)
		{
			continue;
		}

		if (bullet.isDead || bullet.isReflected)
		{
			return;
		}

		Vector3 direction = reflectDirection;
		direction.y = 0.0f;

		if (direction.LengthSquared() <= 0.000001f)
		{
			return;
		}

		direction.NormalizeSelf();

		// プレイヤーから渡された方向と一致する敵を探す
		GameObject* reflectTarget =
			FindReflectTarget(
				bulletObject->GetPosition(),
				direction);

		bullet.isReflected = true;
		bullet.isStraight = false;
		bullet.straightDir = {};

		if (reflectTarget)
		{
			// ロックオン方向と一致する敵が見つかった場合
			// その敵を実際のホーミング対象にする
			bullet.target = reflectTarget;
			bullet.useVirtualTarget = false;
			bullet.virtualTargetPosition = {};
		}
		else
		{
			// ロックオン対象を復元できなかった場合は、プレイヤーから渡された方向へ曲線移動する
			bullet.target = nullptr;
			bullet.useVirtualTarget = true;

			// 35のような固定値ではなく、
			// 直進速度×寿命を曲線の到達距離にする
			const float travelDistance =
				straightSpeed_ * bulletLifeTime_;

			bullet.virtualTargetPosition =
				bulletObject->GetPosition() +
				direction * travelDistance;
		}

		// 反射地点から、敵が撃った時と同じ曲線を作り直す
		bullet.timer = 0.0f;
		bullet.lifeTime = bulletLifeTime_;

		InitializeBulletCurve(bullet);

		if (auto collider =
				bullet.object->GetComponent<AABBColliderComponent>())
		{
			collider->SetCollisionLayer(
				CollisionLayer::PlayerBullet);

			collider->SetCollisionMask(
				CollisionLayer::Enemy |
				CollisionLayer::Bumpers);
		}

		bullet.object->SetTag(GameObjectTag::PlayerBullet);

		return;
	}
}

GameObject* HormingMoveComponent::FindReflectTarget(
	const Vector3& sourcePosition,
	const Vector3& reflectDirection) const
{
	Vector3 normalizedReflectDirection = reflectDirection;
	normalizedReflectDirection.y = 0.0f;

	if (normalizedReflectDirection.LengthSquared() <= 0.000001f)
	{
		return nullptr;
	}

	normalizedReflectDirection.NormalizeSelf();

	const auto& gameObjects =
		GameObjectManager::GetInstance()->GetGameObjects();

	GameObject* bestTarget = nullptr;
	float bestDot = -1.0f;

	for (GameObject* object : gameObjects)
	{
		if (!object)
		{
			continue;
		}

		// 敵本体だけを候補にする
		if (object->GetTag() != GameObjectTag::Enemy)
		{
			continue;
		}

		if (!object->IsActive() || object->IsPendingDestroy())
		{
			continue;
		}

		Vector3 toEnemy = object->GetPosition() - sourcePosition;
		toEnemy.y = 0.0f;

		if (toEnemy.LengthSquared() <= 0.000001f)
		{
			continue;
		}

		toEnemy.NormalizeSelf();

		// Vector3のDot関数に依存しないように直接計算
		const float dot =
			normalizedReflectDirection.x * toEnemy.x +
			normalizedReflectDirection.y * toEnemy.y +
			normalizedReflectDirection.z * toEnemy.z;

		if (dot > bestDot)
		{
			bestDot = dot;
			bestTarget = object;
		}
	}

	// 方向が十分一致している敵だけをロックオン対象として採用する
	if (bestDot < 0.999f)
	{
		return nullptr;
	}

	return bestTarget;
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

void HormingMoveComponent::KillAllBullets()
{
	for (HomingBullet& bullet : bullets_)
	{
		if (!bullet.object || bullet.isDead)
		{
			continue;
		}

		// KillBullet()はbullets_を再度検索するので、
		// 削除対象のポインタを一度保存してから呼ぶ
		GameObject* bulletObject = bullet.object;
		KillBullet(bulletObject);
	}
}

Vector3 HormingMoveComponent::CubicBezier(const Vector3& p0, const Vector3& p1,
										  const Vector3& p2, const Vector3& p3,
										  float t)
{
	const float invT = 1.0f - t;

	return p0 * (invT * invT * invT) + p1 * (3.0f * invT * invT * t) +
		   p2 * (3.0f * invT * t * t) + p3 * (t * t * t);
}

float HormingMoveComponent::EaseBullet(float t)
{
	t = std::clamp(t, 0.0f, 1.0f);

	// 発射直後に速く進み、終点へ近づくほど緩やかになる
	const float invT = 1.0f - t;
	return 1.0f - invT * invT * invT;
}