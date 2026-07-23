#include "EnemyFactory.h"

#include "base/Logger.h"
#include "engine/gameobject/base/GameObject.h"
#include "engine/gameobject/manager/GameObjectManager.h"
#include "engine/gameobject/component/collision/AABBColliderComponent.h"
#include "engine/gameobject/component/collision/SphereColliderComponent.h"
#include "application/collision/CollisionLayer.h"
#include "application/gameobject/GameObjectTag.h"
#include "application/gameobject/component/action/common/PhysicsComponent.h"
#include "application/gameobject/component/action/common/TrailComponent.h"
#include "application/gameobject/component/action/common/StatusComponent.h"
#include "application/gameobject/component/action/common/UIComponent.h"
#include "application/gameobject/component/action/enemy/bomb/BombMoveComponent.h"
#include "application/gameobject/component/action/enemy/charge/ChargeMoveComponent.h"
#include "application/gameobject/component/action/enemy/dash/DashMoveComponent.h"
#include "application/gameobject/component/action/enemy/horming/HormingMoveComponent.h"
#include "application/gameobject/component/action/enemy/EnemyDeathDirectionComponent.h"
#include "math/VectorColorCodes.h"
#include "audio/Audio.h"

using namespace GameObjectComponent;

namespace
{
	// 敵の生成位置は地面(y=0)ぎりぎりの高さに固定する
	constexpr float kEnemySpawnPositionY = 5.0f;
	constexpr Vector3 kEnemyScale = { 2.0f, 2.0f, 2.0f };
	// HPバーは頭上少し上に固定
	constexpr Vector3 kUIOffset = { 0.0f, 6.0f, 0.0f };

	// Terrain, Bumpersとの押し戻し・接地処理
	void HandleTerrainCollision(GameObject* enemy, const CollisionInfo& info)
	{
		if (!info.otherCollider) { return; }
		if (!(info.otherCollider->GetCollisionLayer() & (CollisionLayer::Terrain | CollisionLayer::Bumpers))) { return; }

		Vector3 pos = enemy->GetPosition();
		pos += info.normal * info.depth;
		enemy->SetPosition(pos);

		auto physics = enemy->GetComponent<PhysicsComponent>();
		if (!physics) { return; }

		if (info.normal.y > 0.0f)
		{
			physics->SetGrounded(true);
			Vector3 vel = physics->GetExternalVelocity();
			if (vel.y < 0.0f)
			{
				vel.y = 0.0f;
				physics->SetExternalVelocity(vel);
			}
		}
	}

	// プレイヤーの反射弾(PlayerBullet)に当たったらダメージを受ける
	void HandleBulletDamage(GameObject* enemy, const CollisionInfo& info)
	{
		if (!info.otherCollider) { return; }
		if (!(info.otherCollider->GetCollisionLayer() & CollisionLayer::PlayerBullet)) { return; }

		auto status = enemy->GetComponent<StatusComponent>();
		if (status) 
		{
			status->ApplyDamage(10); 
			Audio::GetInstance()->PlayWave("se_damage");
		
		}
	}

	void SetupCommonCollider(GameObject* enemy)
	{
		enemy->AddComponent("Collider", std::make_unique<AABBColliderComponent>(enemy));
		auto collider = enemy->GetComponent<AABBColliderComponent>();
		if (!collider) { return; }

		collider->SetCollisionLayer(CollisionLayer::Enemy);
		collider->SetCollisionMask(CollisionLayer::Player | CollisionLayer::PlayerReflect | CollisionLayer::PlayerBullet | CollisionLayer::Terrain | CollisionLayer::Bumpers);

		collider->SetOnEnter([enemy](const CollisionInfo& info)
		{
			HandleTerrainCollision(enemy, info);
			HandleBulletDamage(enemy, info);
		});
		collider->SetOnStay([enemy](const CollisionInfo& info)
		{
			HandleTerrainCollision(enemy, info);
		});
	}
}

EnemyFactory::EnemyFactory()
{
	RegisterDefaultEnemies();
}

void EnemyFactory::Initialize(SpriteCommon* spriteCommon, Camera* camera)
{
	spriteCommon_ = spriteCommon;
	camera_ = camera;
}

void EnemyFactory::Register(const std::string& type, Creator creator)
{
	registry_[type] = std::move(creator);
}

GameObject* EnemyFactory::Create(const SpawnCommand& command, GameObject* player) const
{
	auto it = registry_.find(command.type);
	if (it == registry_.end())
	{
		Logger::Log("[EnemyFactory] Unknown enemy type: " + command.type + "\n");
		return nullptr;
	}
	// 敵生成時のSE再生
	Audio::GetInstance()->PlayWave("se_spawn");

	return it->second(command, player);
}

GameObject* EnemyFactory::CreateBaseEnemy(const SpawnCommand& command, const std::string& modelName) const
{
	GameObject* enemy = GameObjectManager::GetInstance()->CreateGameObject(command.type + "Enemy", GameObjectTag::Enemy);
	if (!enemy) { return nullptr; }

	enemy->SetModel(modelName);
	enemy->SetPosition({ command.position.x, kEnemySpawnPositionY, command.position.z });
	enemy->SetScale(kEnemyScale);

	enemy->AddComponent("Status", std::make_unique<StatusComponent>(enemy));
	enemy->AddComponent("Physics", std::make_unique<PhysicsComponent>(enemy));
	// HPが尽きたら死亡演出を再生してDestroyする(TestSceneのChargeEnemy/HormingTestCubeと同様)
	enemy->AddComponent("DeathDirection", std::make_unique<EnemyDeathDirectionComponent>("bullet_hit"));

	if (spriteCommon_ && camera_)
	{
		enemy->AddComponent("UI", std::make_unique<UIComponent>(enemy, spriteCommon_, camera_, kUIOffset));
	}

	return enemy;
}

void EnemyFactory::RegisterDefaultEnemies()
{
	Register("Dash", [this](const SpawnCommand& command, GameObject* player) -> GameObject*
	{
		GameObject* enemy = CreateBaseEnemy(command, "cube");
		if (!enemy) { return nullptr; }
		enemy->AddComponent("Move", std::make_unique<DashMoveComponent>(player));
		// 跳ね返した時に出すエフェクトコンポーネント
		auto trail = std::make_unique<TrailComponent>();
		trail->SetColor(VectorColorCodes::Red);
		enemy->AddComponent("trail", move(trail));
		SetupCommonCollider(enemy);
		return enemy;
	});

	Register("Charge", [this](const SpawnCommand& command, GameObject* player) -> GameObject*
	{
		GameObject* enemy = CreateBaseEnemy(command, "chargeEnemy");
		if (!enemy) { return nullptr; }
		enemy->AddComponent("Move", std::make_unique<ChargeMoveComponent>(player));
		SetupCommonCollider(enemy);
		return enemy;
	});

	Register("Homing", [this](const SpawnCommand& command, GameObject* player) -> GameObject*
	{
		GameObject* enemy = CreateBaseEnemy(command, "cube");
		if (!enemy) { return nullptr; }
		enemy->AddComponent("Move", std::make_unique<HormingMoveComponent>(player));
		SetupCommonCollider(enemy);
		// ホーミング敵は3発で倒れる(TestSceneの調整値を踏襲)
		if (auto status = enemy->GetComponent<StatusComponent>())
		{
			status->SetHp(3);
		}
		return enemy;
	});

	Register("Bomb", [this](const SpawnCommand& command, GameObject* player) -> GameObject*
	{
		GameObject* enemy = CreateBaseEnemy(command, "bombenemy");
		if (!enemy) { return nullptr; }
		enemy->AddComponent("Move", std::make_unique<BombMoveComponent>(player));
		SetupCommonCollider(enemy);
		// 爆発判定用の球コライダー(設定はBombMoveComponentが内部で行う)
		enemy->AddComponent("ExplosionCollider", std::make_unique<SphereColliderComponent>(enemy));
		return enemy;
	});
}
