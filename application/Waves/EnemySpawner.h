#pragma once
#include "SpawnCommand.h"

class Object3dCommon;
class LightManager;
class GameObject;

/**
 * @brief SpawnCommandから実際のGameObject(敵)を生成するクラス
 */
class EnemySpawner
{
public:
	void Initialize(Object3dCommon* object3dCommon, LightManager* lightManager);

	/**
	 * @brief SpawnCommandに従って敵を生成する
	 * @return 生成されたGameObject(GameObjectManagerが所有。失敗時はnullptr)
	 */
	GameObject* Spawn(const SpawnCommand& command) const;

private:
	Object3dCommon* object3dCommon_ = nullptr;
	LightManager* lightManager_ = nullptr;
};
