#pragma once
#include "EnemySpawner.h"
#include "Wave.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class WaveSystem
{
	static constexpr uint32_t WAVE_FILE_COUNT = 16;
	static constexpr uint32_t WAVE_COUNT = 5;
	static constexpr float INTERVAL = 3.f;


	std::vector<std::unique_ptr<Wave>> waves_;
	uint32_t current_ = 0;

	// Wave完了後、次Waveを開始するまでの待機管理
	bool waitingInterval_ = false;
	float intervalTimer_ = 0.0f;

	EnemySpawner spawner_;
	bool started_ = false;

public:
	void Initialize(SpriteCommon* spriteCommon, Camera* camera);

	// 追跡対象を設定する(未実装のシーンではnullptrのまま)
	void SetPlayer(GameObject* player) { spawner_.SetPlayer(player); }

	// ImGuiの「Start」ボタン等から呼ぶ。最初のWaveのスポーンを開始する
	void Start();

	// ImGuiの「Restart」ボタンから呼ぶ。全Waveを未開始状態に戻す(デバッグ用)
	void Restart();

	// ImGuiの「Force Complete」ボタンから呼ぶ。クリア済み状態を強制的に立てる(デバッグ用)
	void ForceComplete();

	void Update(float deltaTime);

	// ImGuiのデバッグボタンから呼ぶ。指定した敵タイプを1体だけ即時スポーンする(デバッグ用)
	GameObject* SpawnSingleEnemy(const std::string& type);

	bool IsCompleted() const;
	bool HasStarted() const { return started_; }

	// デバッグ表示用
	uint32_t GetCurrentWaveIndex() const { return current_; }
	size_t GetWaveCount() const { return waves_.size(); }

private:
	void Load();

	std::string MakeWaveFileName(uint32_t index) const;
};