#pragma once

#include "engine/gameobject/component/base/IActionComponent.h"
#include "math/Vector3.h"

#include "../bullet/BulletSpawnComponent.h"

namespace GameObjectComponent
{

    // 前方宣言
	class PhysicsComponent;
	class StatusComponent;

  /// <summary>
  /// チャージ移動コンポーネント
  /// </summary>
  class ChargeMoveComponent : public IActionComponent
  {
  public:

	  // コンストラクタ
	  ChargeMoveComponent(GameObject* _player);

	  // デストラクタ
	  ~ChargeMoveComponent() override = default;
	  
	  /// <summary>
	  /// 毎フレームの更新処理
	  /// </summary>
	  /// <param name="owner">所有者</param>
      void Update(GameObject* owner) override;

	  void Move(GameObject* owner);

	  /// <summary>
	  /// チャージ
	  /// </summary>
	  /// <param name="owner">所有者</param>
	  void Charge(GameObject* owner);

	  /// <summary>
	  /// クールダウン
	  /// </summary>
	  /// <param name="owner">所有者</param>
	  void Cooldown(GameObject* owner);

	  /// <summary>
	  /// 発射
	  /// </summary>
	  /// <param name="owner">所有者</param>
	  void Fire(GameObject* owner);

	  /// <summary>
	  /// 弾生成
	  /// </summary>
	  /// <param name="owner">所有者</param>
	  void BulletInitialize(GameObject* owner);

	  /// <summary>
	  /// 左右移動移動
	  /// </summary>
	  /// <param name="owner"></param>
	  void StrafeMove(GameObject* owner);

	  /// <summary>
	  /// 乱数生成
	  /// </summary>
	  /// <param name="min">最小値</param>
	  /// <param name="max">最大値</param>
	  float Random(float min, float max);

	  /// <summary>
	  /// コンポーネント破棄時の処理
	  /// </summary>
	  /// <param name="owner"></param>
	  void Destroy(GameObject* owner);

  private:

	  // プレイヤーのポインタ
	  GameObject* player_ = nullptr; 

	  // 物理
	  PhysicsComponent* physics_ = nullptr;
	  // ステータス
	  StatusComponent* status_ = nullptr;

	  // 移動速度
	  float moveSpeed_ = 5.0f;

	  // 回転速度
	  float rotationSpeed_ = 60.0f;

	  // チャージ開始距離
	  float chargeStartDistance_ = 30.0f;
	  // チャージ開始フラグ
	  bool isChargeStart_ = false;
	  // チャージ時間
	  const float kChargeTime = 1.0f;
	  float chargeTime_ = 0.0f;

	  // 攻撃クールタイム
	  const float kCoolTime = 5.0f;
	  float coolTime_ = 0.0f;

	   // 左右移動のタイマー
	  float strafeTimer_ = 0.0f;
	  // 左右移動の切り替え時間
	  float changeTime_ = 0.5f;
	  // 左右移動フラグ
	  bool moveRight_ = true;
	  // 左右移動速度
	  float currentStrafeSpeed_ = 5.0f;

	  // 攻撃フラグ
	  bool isAttacking_ = false;

	  // 弾spawnコンポーネント
	  std::unique_ptr<BulletSpawnComponent> bulletSpawnComponent_;

	  // 弾の向き
	  Vector3 bulletDirection_ = {0.0f, 0.0f, 1.0f};

	  enum class State
	  {
		  Move,
		  Charge,
		  Fire,
		  Cooldown
	  };
	  State state_ = State::Move;

	  // 死亡アニメーションフラグ
	  bool isDeadAnimation_ = false;
	  // 死亡アニメーション時間
	  float deathTimer_ = 0.0f;
	  // 死亡アニメーションの展開時間
	  const float kExpandTime = 0.08f;
	  const float kShrinkTime = 0.15f;

  };


} // namespace GameObjectComponent