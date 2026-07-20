#pragma once
#include "engine/gameobject/component/base/IActionComponent.h"
#include "jsonEditor/JsonEditableBase.h"
#include "math/Vector3.h"

#include <vector>

class GameObject;

namespace GameObjectComponent
{
	class HormingMoveComponent : public IActionComponent
		, public JsonEditableBase
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

			Vector3 startPos = {};	  // ベジェ曲線の開始位置
			Vector3 controlPos1 = {}; // ベジェ曲線の制御点1
			Vector3 controlPos2 = {}; // ベジェ曲線の制御点2
			Vector3 endPos = {};	  // ベジェ曲線の終点
			Vector3 sideDir = {};	  // 進行方向に対して横方向のベクトル

			// 横方向の広がり倍率
			float sidePower = 1.0f;

			// 弾が生成されてからの経過時間
			float timer = 0.0f;

			// 弾の生存時間
			float lifeTime = 3.0f;

			// ホーミングを解除して直進中かどうか
			bool isStraight = false;

			// ホーミング解除後に進む方向
			Vector3 straightDir = {};

			// 削除済みかどうか
			bool isDead = false;
		};

	private:
		// ホーミング弾を発射
		void FireBullet(GameObject* owner, int32_t bulletIndex, int32_t bulletCount);

		// 生成済みのホーミング弾の更新
		void UpdateBullets();

		// 一定間隔での自動発射処理
		void UpdateAutoFire(GameObject* owner);

		// 指定した弾を削除する
		void KillBullet(GameObject* bulletObject);

		// 弾のベジェ曲線用の初期情報を作成する
		void InitializeBulletCurve(HomingBullet& bullet);

		// 3次ベジェ曲線上の座標を計算
		Vector3 CubicBezier(
			const Vector3& p0,
			const Vector3& p1,
			const Vector3& p2,
			const Vector3& p3,
			float t);

		// キレを出すための補間
		float EaseBullet(float t);

	private:
		// ホーミング対象
		GameObject* target_ = nullptr;

		// ホーミング弾のリスト
		std::vector<HomingBullet> bullets_;

		// 弾の寿命
		float bulletLifeTime_ = 2.2f;

		// 弾のスケール
		float bulletScale_ = 0.6f;

		// ベジェ曲線の横方向の膨らみ
		float sideOffset_ = 22.0f;

		// 見下ろし視点用に高さ方向は控えめ
		float heightOffset_ = 10.0f;

		// 移動中にさらに横へ流す追加オフセット
		float slideOffset_ = 18.0f;

		// ターゲット位置への追従率
		float targetFollowRate_ = 0.045f;

		// この距離以内に近づいたらホーミングをやめて直進する
		float homingReleaseDistance_ = 12.0f;

		// ホーミング解除後の直進速度
		float straightSpeed_ = 28.0f;

		// 自動発射の間隔
		float autoFireInterval_ = 2.0f;

		// 自動発射用タイマー
		float autoFireTimer_ = 0.0f;

		// 一度の攻撃で撃つ弾数
		int32_t burstCount_ = 3;

		// 複数弾を少しずつずらして撃つ間隔
		float burstInterval_ = 0.18f;

		// バースト発射中のタイマー
		float burstTimer_ = 0.0f;

		// 残り発射数
		int32_t pendingBurstCount_ = 0;

		// 今のバースト内で何発目か
		int32_t currentBurstIndex_ = 0;
	};
} // namespace GameObjectComponent