#pragma once

#include "engine/gameobject/component/base/IActionComponent.h"
#include "jsonEditor/JsonEditableBase.h"
#include "math/Vector3.h"

class GameObject;

namespace GameObjectComponent
{
	class ICollisionComponent;

	/// <summary>
	/// 敵の生成時に回転・拡大・浮上演出を行うコンポーネント
	/// </summary>
	class EnemySpawnDirectionComponent
		: public IActionComponent
		, public JsonEditableBase
	{
	public:
		EnemySpawnDirectionComponent();

		// 敵を描画する前に、登場開始状態へ変更する
		void Start(GameObject* owner);

		// 毎フレーム更新
		void Update(GameObject* owner) override;

		// 登場演出中か
		bool IsAppearing() const
		{
			return !isFinished_;
		}

		// 登場演出が終了したか
		bool IsFinished() const
		{
			return isFinished_;
		}

	private:
		// 最初の1回だけ初期状態を保存する
		void InitializeSpawn(GameObject* owner);

		// 登場演出を更新する
		void UpdateSpawn(GameObject* owner, float deltaTime);

		// 演出終了
		void FinishSpawn(GameObject* owner);

	private:
		// 初期化済みか
		bool isInitialized_ = false;

		// 演出終了済みか
		bool isFinished_ = false;

		// 経過時間
		float spawnTimer_ = 0.0f;

		// 登場演出時間
		float spawnDuration_ = 1.0f;

		// 開始時の大きさ倍率
		float startScaleRate_ = 0.05f;

		// 下から出現する高さ
		float startHeightOffset_ = -6.0f;

		// Y軸の回転量
		float rotationAmount_ = 12.566370f;

		// 拡大時の弾む強さ
		float scaleBouncePower_ = 0.15f;

		// 元の位置
		Vector3 basePosition_ = {};

		// 元の回転
		Vector3 baseRotation_ = {};

		// 元の大きさ
		Vector3 baseScale_ = {};

		// コライダー
		ICollisionComponent* collider_ = nullptr;
	};
} // namespace GameObjectComponent 