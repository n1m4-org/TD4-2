#pragma once
#include <functional>
#include <string>
#include <unordered_map>

#include "SpawnCommand.h"

class GameObject;
class Object3dCommon;
class LightManager;
class SpriteCommon;
class Camera;

/**
 * @brief SpawnCommand::typeごとの敵生成関数を登録し、生成を仲介するファクトリ
 */
class EnemyFactory
{
public:
	// (command, player) -> 生成されたGameObject
	using Creator = std::function<GameObject*(const SpawnCommand&, GameObject* player)>;

	EnemyFactory();

	void Initialize(SpriteCommon* spriteCommon, Camera* camera);

	void Register(const std::string& type, Creator creator);

	/**
	 * @return 生成されたGameObject(GameObjectManagerが所有。未登録typeや失敗時はnullptr)
	 */
	GameObject* Create(const SpawnCommand& command, GameObject* player) const;

private:
	void RegisterDefaultEnemies();

	GameObject* CreateBaseEnemy(const SpawnCommand& command, const std::string& modelName) const;

	SpriteCommon* spriteCommon_ = nullptr;
	Camera* camera_ = nullptr;

	std::unordered_map<std::string, Creator> registry_;
};
