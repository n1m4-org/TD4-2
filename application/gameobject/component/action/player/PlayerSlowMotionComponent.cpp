#include "PlayerSlowMotionComponent.h"
#include "application/gameobject/component/action/player/PlayerInputComponent.h"
#include "engine/gameobject/base/GameObject.h"
#include "manager/scene/LightManager.h"
#include "time/TimeManager.h"
#include "time/TimerManager.h"
#include "math/Easing.h"

GameObjectComponent::PlayerSlowMotionComponent::PlayerSlowMotionComponent(LightManager* lightManager)
	: lightManager_(lightManager)
{
}

void GameObjectComponent::PlayerSlowMotionComponent::Update(GameObject* owner)
{
	// 初回更新時のみ行う
	if (!input_)
	{
		input_ = owner->GetComponent<PlayerInputComponent>().get();
	}
	
	// 入力コンポーネントが取得できていないなら終了する
	if (!input_)
	{
		return;
	}

	// 入力状態からスローモーションを行うか判定
	if (input_->IsSlowMotionTriggered() && !isCooldown_)
	{
		// クールダウンフラグを立てる
		isCooldown_ = true;

		// タイマー作成
		std::unique_ptr<Timer> slowMotionTimer = std::make_unique<Timer>("slow_motion", slowMotionDuration_, DeltaTimeType::DeltaTime);

		slowMotionTimer->SetOnStart([this]() {
			// タイムスケールをスローモーション倍率に設定
			TimeManager::GetInstance().SetGameTimeScale(slowMotionFactor_);
		});
		
		// slowMotionTimerの終了時に新たなタイマーを作成して徐々に元のタイムスケールに戻す
		slowMotionTimer->SetOnFinish([this]() {
			std::unique_ptr<Timer> fadeBackTimer = std::make_unique<Timer>("fade_back_time_scale", 1.0f, DeltaTimeType::DeltaTime);

			Timer* fadeBackTimerPtr = fadeBackTimer.get(); // 生ポインタを保持しておく

			fadeBackTimer->SetOnTick([this, fadeBackTimerPtr](float elapsed) {
				float scale = EasingToEnd(slowMotionFactor_, 1.0f, EaseInSine<float>, fadeBackTimerPtr->GetProgress());
				TimeManager::GetInstance().SetGameTimeScale(scale);
			});
			
			fadeBackTimer->SetOnFinish([this]() {
				// クールダウンのタイマー作成
				std::unique_ptr<Timer> cooldownTimer = std::make_unique<Timer>("slow_motion_cooldown", slowMotionCooldown_, DeltaTimeType::DeltaTime);

				cooldownTimer->SetOnFinish([this]() {
					isCooldown_ = false;
				});
				TimerManager::GetInstance().AddTimer(std::move(cooldownTimer));
			});
			
			TimerManager::GetInstance().AddTimer(std::move(fadeBackTimer));
		});
		TimerManager::GetInstance().AddTimer(std::move(slowMotionTimer));
	}
}
