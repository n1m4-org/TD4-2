#include "Wave.h"

#include "WaveData.h"

void Wave::Load(const std::string& fileName)
{
	WaveData data;
	if (!data.LoadJson("wave/" + fileName + ".json")) { return; }

	commands_ = data.GetCommands();
	// TODO: elapsedTime_/duration_によるダミー完了判定を、commands_基準(発行済み件数+生存敵数)に置き換える。
}

void Wave::Start()
{
	elapsedTime_ = 0.0f;
	state_ = WaveState::Running;
}

void Wave::Update(float deltaTime)
{
	if (state_ != WaveState::Running) { return; }

	elapsedTime_ += deltaTime;
	if (elapsedTime_ >= duration_)
	{
		state_ = WaveState::Completed;
	}
}