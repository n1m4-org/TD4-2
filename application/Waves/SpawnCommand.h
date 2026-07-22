#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include "math/Vector3.h"

/**
 * @brief 指定時刻に指定座標へ敵を1体生成する命令
 *
 * timingはWave開始からの絶対時刻。同時刻に複数体出す場合はSpawnCommandを複数並べる。
 */
struct SpawnCommand
{
	float timing = 0.0f;
	std::string type;
	Vector3 position{};
};

void to_json(nlohmann::json& j, const SpawnCommand& command);
void from_json(const nlohmann::json& j, SpawnCommand& command);
