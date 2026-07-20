#include "EnemySpawner.h"

#include "base/Logger.h"
#include "engine/gameobject/base/GameObject.h"
#include "engine/gameobject/manager/GameObjectManager.h"
#include "gameobject/component/action/common/PhysicsComponent.h"
#include "gameobject/component/action/common/StatusComponent.h"
#include "gameobject/component/action/enemy/bomb/BombMoveComponent.h"
#include "gameobject/component/action/enemy/charge/ChargeMoveComponent.h"
#include "gameobject/component/action/enemy/dash/DashMoveComponent.h"
#include "gameobject/component/action/enemy/horming/HormingMoveComponent.h"

using namespace GameObjectComponent;

void EnemySpawner::Initialize(Object3dCommon* object3dCommon, LightManager* lightManager)
{
	object3dCommon_ = object3dCommon;
	lightManager_ = lightManager;
}

GameObject* EnemySpawner::Spawn(const SpawnCommand& command) const
{
	if (!object3dCommon_ || !lightManager_)
	{
		Logger::Log("[EnemySpawner] Not initialized.\n");
		return nullptr;
	}

	GameObject* enemy = GameObjectManager::GetInstance()->CreateGameObject(command.type + "Enemy", "Enemy");
	if (!enemy) { return nullptr; }

	// マネージャーのキャッシュに依存せず、確実に初期化する
	enemy->Initialize(object3dCommon_, lightManager_);
	enemy->SetModel("cube");
	enemy->SetPosition(command.position);
	enemy->SetScale({2.0f, 2.0f, 2.0f});

	enemy->AddComponent("Status", std::make_unique<StatusComponent>(enemy));

	auto physics = std::make_unique<PhysicsComponent>(enemy);
	// 地形が無いため、重力を無効にして生成位置に留まらせる
	physics->SetUseGravity(false);
	enemy->AddComponent("Physics", std::move(physics));

	// プレイヤーが未実装のため、Move系コンポーネントにはnullptrを渡す(各実装はnullチェック済みで安全)
	if (command.type == "Dash")
	{
		enemy->AddComponent("Move", std::make_unique<DashMoveComponent>(nullptr));
	}
	else if (command.type == "Bomb")
	{
		enemy->AddComponent("Move", std::make_unique<BombMoveComponent>(nullptr));
	}
	else if (command.type == "Charge")
	{
		enemy->AddComponent("Move", std::make_unique<ChargeMoveComponent>(nullptr));
	}
	else if (command.type == "Homing")
	{
		enemy->AddComponent("Move", std::make_unique<HormingMoveComponent>(nullptr));
	}
	else
	{
		Logger::Log("[EnemySpawner] Unknown enemy type: " + command.type + "\n");
	}

	return enemy;
}
