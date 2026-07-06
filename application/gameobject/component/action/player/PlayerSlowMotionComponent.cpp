#include "PlayerSlowMotionComponent.h"
#include "application/gameobject/component/action/player/PlayerInputComponent.h"
#include "engine/gameobject/base/GameObject.h"
#include "manager/scene/LightManager.h"
#include "math/Easing.h"
#include "math/VectorColorCodes.h"
#include "time/TimeManager.h"
#include "time/TimerManager.h"

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

	// 入力コンポーネントとライトマネージャーが取得できていない場合は処理を中断
	if (!input_)
	{
		return;
	}

	// 入力状態からスローモーションを行うか判定
	if (input_->IsSlowMotionTriggered() && !isCooldown_)
	{
		// クールダウンフラグを立てる
		isCooldown_ = true;

		// スローモーションのタイマーを作成
		std::unique_ptr<Timer> slowMotionTimer = std::make_unique<Timer>("slow_motion", slowMotionDuration_, DeltaTimeType::DeltaTime);

		slowMotionTimer->SetOnStart([this, owner]()
		{
			// タイムスケールをスローモーション倍率に設定
			TimeManager::GetInstance().SetGameTimeScale(slowMotionFactor_);
			// ディレクショナルライトの明るさを0,環境光を黒にして真っ暗にする
			DirectionalLight dirLight = lightManager_->GetDirectionalLight();
			dirLight.intensity = 0.0f;
			dirLight.ambient = VectorColorCodes::Black;
			lightManager_->SetDirectionalLight(dirLight);

			// スポットライトの位置更新
			lightManager_->SetSpotLightPosition("player_spot_light", owner->GetPosition() + spotLightOffset_);
			// スポットライトの明るさを4にする
			lightManager_->SetSpotLightIntensity("player_spot_light", 4.0f);
			// スポットライトの距離を50にする
			lightManager_->SetSpotLightDistance("player_spot_light", 50.0f);
		});

		slowMotionTimer->SetOnTick([this, owner](float)
		{
			// スポットライトの位置を更新
			lightManager_->SetSpotLightPosition("player_spot_light", owner->GetPosition() + spotLightOffset_);
		});

		// slowMotionTimerの終了時に新たなタイマーを作成して徐々に元のタイムスケールに戻す
		slowMotionTimer->SetOnFinish([this]()
		{
			// ディレクショナルライトの設定を元に戻す
			DirectionalLight dirLight = lightManager_->GetDirectionalLight();
			dirLight.intensity = 0.6f;
			dirLight.ambient = VectorColorCodes::White;
			lightManager_->SetDirectionalLight(dirLight);

			// 各パラメータを徐々に戻すタイマー
			std::unique_ptr<Timer> fadeBackTimer = std::make_unique<Timer>("fade_back_time_scale", 1.0f, DeltaTimeType::DeltaTime);

			Timer* fadeBackTimerPtr = fadeBackTimer.get(); // 生ポインタを保持しておく

			fadeBackTimer->SetOnTick([this, fadeBackTimerPtr](float elapsed)
			{
				// スケールを徐々に戻す
				float scale = EasingToEnd(slowMotionFactor_, 1.0f, EaseInSine<float>, fadeBackTimerPtr->GetProgress());
				TimeManager::GetInstance().SetGameTimeScale(scale);

				// ライトの設定を徐々に戻す
				DirectionalLight dirLight = lightManager_->GetDirectionalLight();
				dirLight.intensity = EasingToEnd(0.0f, 0.6f, EaseInSine<float>, fadeBackTimerPtr->GetProgress());
				dirLight.ambient = EasingToEnd(VectorColorCodes::Black, VectorColorCodes::White, EaseInSine, fadeBackTimerPtr->GetProgress());
				lightManager_->SetDirectionalLight(dirLight);

				// スポットライト設定を戻す
				float spotLightIntensity = EasingToEnd(4.0f, 0.0f, EaseInSine<float>, fadeBackTimerPtr->GetProgress());
				float spotLightDistance = EasingToEnd(50.0f, 100.0f, EaseInSine<float>, fadeBackTimerPtr->GetProgress());
				lightManager_->SetSpotLightIntensity("player_spot_light", spotLightIntensity);
				lightManager_->SetSpotLightDistance("player_spot_light", spotLightDistance);
			});

			fadeBackTimer->SetOnFinish([this]()
			{
				// クールダウンのタイマー作成
				std::unique_ptr<Timer> cooldownTimer = std::make_unique<Timer>("slow_motion_cooldown", slowMotionCooldown_, DeltaTimeType::DeltaTime);

				cooldownTimer->SetOnFinish([this]()
				{
					isCooldown_ = false;
				});
				TimerManager::GetInstance().AddTimer(std::move(cooldownTimer));
			});

			TimerManager::GetInstance().AddTimer(std::move(fadeBackTimer));
		});
		TimerManager::GetInstance().AddTimer(std::move(slowMotionTimer));
	}
}
