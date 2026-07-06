#pragma once
#include "engine/gameobject/component/base/IActionComponent.h"

class LightManager;

namespace GameObjectComponent
{
	class PlayerInputComponent;

	class PlayerSlowMotionComponent : public IActionComponent
	{
	public:
		PlayerSlowMotionComponent(LightManager* lightManager);

		void Update(GameObject* owner) override;

	private:
		// ライトマネージャーのポインタを保持
		LightManager* lightManager_ = nullptr;

		// プレイヤーの入力状態コンポーネントのポインタを保持
		PlayerInputComponent* input_ = nullptr;

		// スローモーションの継続時間
		float slowMotionDuration_ = 2.0f;

		// スローモーションの倍率
		float slowMotionFactor_ = 0.5f;
	};
} // namespace GameObjectComponent
