#pragma once

#include "engine/gameobject/component/base/IActionComponent.h"

namespace GameObjectComponent
{

	class EnemyLongRangeMoveCompornent : public IActionComponent
	{
	public:

		EnemyLongRangeMoveCompornent(GameObject* _player);

		void Update(GameObject* owner) override;

	private: // 内部関数

		// 乱数生成
		float Random(float min, float max);

	private:

		 // プレイヤーのポインタ
		GameObject* player_ = nullptr;

		// 左右移動のタイマー
		float strafeTimer_;
		// 左右移動の切り替え時間
		float changeTime_;
		// 左右移動フラグ
		bool moveRight_;

		// 移動速度
		float moveSpeed_ = 5.0f;

	};

} // namespace GameObjectComponent