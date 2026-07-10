#pragma once
#include "engine/gameobject/component/base/IActionComponent.h"
#include "jsonEditor/JsonEditableBase.h"

// プレイヤーの前方宣言
class GameObject;

namespace GameObjectComponent
{
	// 物理挙動コンポーネントの前方宣言
	class PhysicsComponent;

	class BombMoveComponent : public IActionComponent
		, public JsonEditableBase
	{
	public:
		// プレイヤーのポインタを受け取るコンストラクタ
		BombMoveComponent(GameObject* player);

		// 更新処理
		void Update(GameObject* owner) override;


	public: // Setter / Getter
		// 反射されたら
		void OnReflected(const Vector3& direction, float speed)
		{
			// すでに反射済み、爆発済みならスルー
			if (isReflected_ || hasExploded_)
			{
				return;
			}
			// 各種フラグを設定
			isReflected_ = true;
			isDashing_ = false;
			reflectedDirection_ = direction.Normalize();
			reflectedVelocity_ = reflectedDirection_ * speed;
			reflectedLifespan_ = lifespan_; 
		}


	private:
		// プレイヤーのポインタを保持
		GameObject* player_ = nullptr;

		// プレイヤーの座標を保持
		Vector3 playerPosition_ = {0.0f, 0.0f, 0.0f};

		// 突進中かどうかのフラグ
		bool isDashing_ = false;

		// 突進する方向
		Vector3 dashDirection_ = {0.0f, 0.0f, 0.0f};

		// 物理挙動コンポーネントのポインタを保持
		PhysicsComponent* physics_ = nullptr;

		// 追尾開始距離
		float chaseRange_ = 30.0f;

		// 追尾速度
		float chaseSpeed_ = 7.0f;

		// 点火から爆発までの時間
		const float ignitionTime_ = 300.0f;


		// 爆発までの残り時間
		float lifespan_ = ignitionTime_;

		// 爆発済みかどうかのフラグ
		bool hasExploded_ = false;


		// リフレクト後のフラグ
		bool isReflected_ = false;

		// 爆発までの残り時間（リフレクト後） 60は仮
		float reflectedLifespan_ = 60.0f;

		// プレイヤー側から取得する反射後の方向と速度
		Vector3 reflectedDirection_ = {0.0f, 0.0f, 0.0f};
		Vector3 reflectedVelocity_ = {0.0f, 0.0f, 0.0f};
	};

} // namespace GameObjectComponent