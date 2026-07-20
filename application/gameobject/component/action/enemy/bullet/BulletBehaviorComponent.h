#pragma once

#include "engine/gameobject/component/base/IActionComponent.h"
#include "math/Vector3.h"

/**
 * @brief 弾の寿命と反射状態を管理する。
 */
class BulletBehaviorComponent : public GameObjectComponent::IActionComponent
{
public:
	/**
	 * @brief 弾の寿命を設定する。
	 * @param lifetime 寿命（秒）
	 */
	explicit BulletBehaviorComponent(float lifetime);

	/**
	 * @brief 弾の寿命を更新する。
	 * @param owner このコンポーネントを所有する弾
	 */
	void Update(GameObject* owner) override;

	/**
	 * @brief 弾を指定方向へ反射する。
	 * @param owner このコンポーネントを所有する弾
	 * @param direction 反射方向
	 * @param speed 反射後の速度
	 * @return 反射を受け付けた場合はtrue
	 */
	bool Reflect(GameObject* owner, const Vector3& direction, float speed);

	/**
	 * @brief 反射済みか取得する。
	 * @return 反射済みならtrue
	 */
	bool IsReflected() const { return isReflected_; }

private:
	// 反射後も引き継ぐ残り寿命。
	float lifetime_ = 0.0f;
	// 衝突コールバックによる多重反射を防ぐ。
	bool isReflected_ = false;
};
