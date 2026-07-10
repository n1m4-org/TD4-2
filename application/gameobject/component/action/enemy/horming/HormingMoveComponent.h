#pragma once
#include "engine/gameobject/component/base/IActionComponent.h"
#include "jsonEditor/JsonEditableBase.h"
#include "math/Vector3.h"

#include <vector>

class GameObject;

namespace GameObjectComponent
{

	class HormingMoveComponent : public IActionComponent, public JsonEditableBase
	{

	public:

		// コンストラクタ
		HormingMoveComponent(GameObject* target = nullptr);

		// 更新処理
		void Update(GameObject* owner) override;

		// ホーミング対象を設定する
		void SetTarget(GameObject* target) { target_ = target; }

	private:

		// ホーミング弾の情報
		struct HomingBullet
		{
			GameObject* object = nullptr;

			Vector3 startPos = {};    // ベジェ曲線の開始位置
			Vector3 controlPos1 = {}; // ベジェ曲線の制御点1
			Vector3 controlPos2 = {}; // ベジェ曲線の制御点2
			Vector3 endPos = {};      // ベジェ曲線の終点
			Vector3 sideDir = {};     // 進行方向に対して横方向のベクトル

			// 弾が生成されてからの経過時間
			float timer = 0.0f;
			// 弾の生存時間
			float lifeTime = 3.0f;
			// 削除済みかどうか
			bool isDead = false;
		};

	private:

		// ホーミング弾を発射
		void FireBullet(GameObject* owner);

		// 生成済みのホーミング弾の更新
		void UpdateBullets();

		// 弾のベジェ曲線用の初期情報を作成する
		void InitializeBulletCurve(HomingBullet& bullet);

		// 3次ベジェ曲線上の座標を計算
		Vector3 CubicBezier(
			const Vector3& p0,
			const Vector3& p1,
			const Vector3& p2,
			const Vector3& p3,
			float t);

		float EaseInOut(float t);

	private:

		// ホーミング対象
		GameObject* target_ = nullptr;

		// ホーミング弾のリスト
		std::vector<HomingBullet> bullets_;

		// 弾の寿命
		float bulletLifeTime_ = 3.0f;

		// 弾のスケール
		float bulletScale_ = 0.6f;

		// ベジェ曲線の横方向の膨らみ
		float sideOffset_ = 10.0f;

		// ベジェ曲線の高さ方向の膨らみ
		float heightOffset_ = 15.0f;

		// 移動中にさらに横へ流す追加オフセット
		float slideOffset_ = 10.0f;

		// ターゲット位置への追従率
		float targetFollowRate_ = 0.15f;

		// 弾の発射間隔
		float cooldownTime_ = 1.0f;

		// クールタイムの経過時間
		float cooldownTimer_ = 0.0f;

		// クールタイム中かどうか
		bool isCooldown_ = false;
	};
} // namespace GameObjectComponent