#pragma once
#include <vector>

#include "jsonEditor/JsonEditableBase.h"
#include "SpawnCommand.h"

/**
 * @brief 1つのwave_NN.jsonの中身をJsonEditableBase経由で読み書きするデータクラス
 *
 * JSON側のキー "wave" とC++側のメンバ名を一致させるため、
 * プロジェクトの命名規約（末尾アンダースコア）はこのメンバに限り使用しない。
 */
class WaveData : public JsonEditableBase
{
public:
	WaveData();

	const std::vector<SpawnCommand>& GetCommands() const { return wave; }

private:
	std::vector<SpawnCommand> wave;
};
