#include "EnemySpawner.h"

void EnemySpawner::Initialize(SpriteCommon* spriteCommon, Camera* camera)
{
	factory_.Initialize(spriteCommon, camera);
}

GameObject* EnemySpawner::Spawn(const SpawnCommand& command)
{
	return factory_.Create(command, player_);
}
