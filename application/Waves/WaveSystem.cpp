#include "WaveSystem.h"

#include <iomanip>
#include <random>
#include <sstream>
#include <unordered_set>

#include "base/Logger.h"

void WaveSystem::Initialize(SpriteCommon* spriteCommon, Camera* camera)
{
	waves_.clear();
	current_ = 0;
	waitingInterval_ = false;
	intervalTimer_ = 0.0f;
	started_ = false;

	spawner_.Initialize(spriteCommon, camera);

	Load();
}

void WaveSystem::Start()
{
	if (started_ || waves_.empty()) { return; }

	started_ = true;
	waves_[current_]->Start();
}

void WaveSystem::Restart()
{
	current_ = 0;
	waitingInterval_ = false;
	intervalTimer_ = 0.0f;
	started_ = false;

	for (auto& wave : waves_)
	{
		wave->Reset();
	}
}

void WaveSystem::ForceComplete()
{
	current_ = static_cast<uint32_t>(waves_.size());
	waitingInterval_ = false;
	intervalTimer_ = 0.0f;
}

void WaveSystem::Update(float deltaTime)
{
	if (!started_) { return; }
	if (IsCompleted()) { return; }

	auto& current = waves_[current_];

	if (!current->IsCompleted())
	{
		current->Update(deltaTime, &spawner_);
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
	std::unordered_set<uint32_t> tried;

	// tried.size() < WAVE_FILE_COUNT で全候補を試し切ったら打ち切るため、
	// 読み込みに失敗するファイルがあっても無限ループしない。
	while (waves_.size() < WAVE_COUNT && tried.size() < WAVE_FILE_COUNT)
	{
		uint32_t index = dist(engine);
		if (!tried.insert(index).second) { continue; } // 試行済みは引き直し

		auto wave = std::make_unique<Wave>();
		if (!wave->Load(MakeWaveFileName(index)))
		{
			Logger::Log("[WaveSystem] Failed to load " + MakeWaveFileName(index) + ", trying another wave.\n");
			continue; // 読み込み失敗したwaveは使わず、別のインデックスを引き直す
		}

		waves_.push_back(std::move(wave));
	}

	if (waves_.size() < WAVE_COUNT)
	{
		Logger::Log("[WaveSystem] Only loaded " + std::to_string(waves_.size()) + "/" + std::to_string(WAVE_COUNT) + " waves.\n");
	}
}

std::string WaveSystem::MakeWaveFileName(uint32_t index) const
{
	std::ostringstream oss;
	oss << "wave_" << std::setw(2) << std::setfill('0') << index;
	return oss.str();
}