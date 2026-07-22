#include "Wave.h"

#include "EnemySpawner.h"
#include "WaveData.h"

bool Wave::Load(const std::string& fileName)
{
	WaveData data;
	if (!data.LoadJson("wave/" + fileName + ".json")) { return false; }

	commands_ = data.GetCommands();
	return true;
}

void Wave::Start()
{
	elapsedTime_ = 0.0f;
	state_ = WaveState::Running;
	spawned_.assign(commands_.size(), false);
}

void Wave::Update(float deltaTime, EnemySpawner* spawner)
{
	if (state_ != WaveState::Running) { return; }

	elapsedTime_ += deltaTime;

	bool allSpawned = true;
	for (size_t i = 0; i < commands_.size(); ++i)
	{
		if (spawned_[i]) { continue; }

		if (elapsedTime_ >= commands_[i].timing)
		{
			if (spawner) { spawner->Spawn(commands_[i]); }
			spawned_[i] = true;
		}
		else
		{
			allSpawned = false;
		}
	}

	if (allSpawned)
	{
		state_ = WaveState::Completed;
	}
}