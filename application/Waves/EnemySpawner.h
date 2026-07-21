#pragma once
#include "EnemyFactory.h"
#include "SpawnCommand.h"

class Object3dCommon;
class LightManager;
class SpriteCommon;
class Camera;
class GameObject;

/**
 * @brief SpawnCommandを受け取り、EnemyFactoryへ生成を委譲するクラス
 */
class EnemySpawner
{
public:
	void Initialize(SpriteCommon* spriteCommon, Camera* camera);

	// 追跡対象を設定する(未実装のシーンではnullptrのまま)
	void SetPlayer(GameObject* player) { player_ = player; }

	/**
	 * @return 生成されたGameObject(GameObjectManagerが所有。失敗時はnullptr)
	 */
	GameObject* Spawn(const SpawnCommand& command);

private:
	EnemyFactory factory_;
	GameObject* player_ = nullptr;
};
