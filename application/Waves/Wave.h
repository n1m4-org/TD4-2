#pragma once
#include <string>
#include <vector>

#include "SpawnCommand.h"

enum class WaveState
{
	Ready,
	Running,
	Completed,
};

class Wave
{
	std::vector<SpawnCommand> commands_;

	float elapsedTime_ = 0.0f;
	float duration_ = 3.0f;

	WaveState state_ = WaveState::Ready;

public:
	void Load(const std::string& fileName);

	void Start();
	void Update(float deltaTime);

	bool IsCompleted() const { return state_ == WaveState::Completed; }
	WaveState GetState() const { return state_; }
};