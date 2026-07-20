#include "WaveSystem.h"

#include <iomanip>
#include <random>
#include <sstream>
#include <unordered_set>

void WaveSystem::Initialize()
{
	waves_.clear();
	current_ = 0;
	waitingInterval_ = false;
	intervalTimer_ = 0.0f;

	Load();

	if (!waves_.empty())
	{
		waves_[current_]->Start();
	}
}

void WaveSystem::Update(float deltaTime)
{
	if (IsCompleted()) { return; }

	auto& current = waves_[current_];

	if (!current->IsCompleted())
	{
		current->Update(deltaTime);
		return;
	}

	if (!waitingInterval_)
	{
		waitingInterval_ = true;
		intervalTimer_ = 0.0f;
		return;
	}

	intervalTimer_ += deltaTime;
	if (intervalTimer_ < INTERVAL) { return; }

	waitingInterval_ = false;
	++current_;

	if (!IsCompleted())
	{
		waves_[current_]->Start();
	}
}

bool WaveSystem::IsCompleted() const
{
	return current_ >= waves_.size();
}

void WaveSystem::Load()
{
	static_assert(WAVE_COUNT <= WAVE_FILE_COUNT, "WAVE_COUNT must not exceed WAVE_FILE_COUNT");

	std::mt19937 engine(std::random_device{}());
	std::uniform_int_distribution<uint32_t> dist(0, WAVE_FILE_COUNT - 1);
	std::unordered_set<uint32_t> used;

	while (waves_.size() < WAVE_COUNT)
	{
		uint32_t index = dist(engine);
		if (!used.insert(index).second) { continue; } // 重複は引き直し

		auto wave = std::make_unique<Wave>();
		wave->Load(MakeWaveFileName(index));
		waves_.push_back(std::move(wave));
	}
}

std::string WaveSystem::MakeWaveFileName(uint32_t index) const
{
	std::ostringstream oss;
	oss << "wave_" << std::setw(2) << std::setfill('0') << index;
	return oss.str();
}