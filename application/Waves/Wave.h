#pragma once
#include <string>
#include <vector>

#include "SpawnCommand.h"

class EnemySpawner;

enum class WaveState
{
	Ready,
	Running,
	Completed,
};

class Wave
{
	std::vector<SpawnCommand> commands_;
	// commands_[i]が発行済みかどうか(commands_と添字対応)
	std::vector<bool> spawned_;

	float elapsedTime_ = 0.0f;

	WaveState state_ = WaveState::Ready;

public:
	bool Load(const std::string& fileName);

	void Start();
	void Update(float deltaTime, EnemySpawner* spawner);

	// Ready状態に戻し、スポーン済みフラグをクリアする(デバッグ用の再スタートに使用)
	void Reset();

	bool IsCompleted() const { return state_ == WaveState::Completed; }
	WaveState GetState() const { return state_; }
};