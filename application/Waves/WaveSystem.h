#pragma once
#include "Wave.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class WaveSystem
{
	static constexpr uint32_t WAVE_FILE_COUNT = 16; 
	static constexpr uint32_t WAVE_COUNT = 5;
	static constexpr float INTERVAL = 1.f;


	std::vector<std::unique_ptr<Wave>> waves_;
	uint32_t current_ = 0;

	// Wave完了後、次Waveを開始するまでの待機管理
	bool waitingInterval_ = false;
	float intervalTimer_ = 0.0f;

public:
	void Initialize();
	void Update(float deltaTime);

	bool IsCompleted() const;

	// デバッグ表示用
	uint32_t GetCurrentWaveIndex() const { return current_; }
	size_t GetWaveCount() const { return waves_.size(); }

private:
	void Load();

	std::string MakeWaveFileName(uint32_t index) const;
};