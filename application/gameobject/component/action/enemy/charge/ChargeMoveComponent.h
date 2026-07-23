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
	  /// 揺れ開始
	  /// </summary>
	  void StartShake();

	  /// <summary>
	  /// 揺れ処理
	  /// </summary>
	  /// <param name="owner">所有者</param>
	  void Shake(GameObject* owner);

  private:

	  // プレイヤーのポインタ
	  GameObject* player_ = nullptr; 

	  // 物理
	  PhysicsComponent* physics_ = nullptr;
	  // ステータス
	  StatusComponent* status_ = nullptr;

	  // 移動速度
	  float moveSpeed_ = 5.0f;

	  // 移動速度倍率
	  const float kMoveSpeedRate_ = 5.0f;

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

	  // チャージ開始位置
	  Vector3 chargeStartPosition_ = {0.0f, 0.0f, 0.0f};
	  // 最大後退距離
	  float maxBackDistance_ = 3.0f;

	  Vector3 shakeOffset{};

	  // 発射時間
	  const float kFireTime = 0.5f;
	  float fireTime_ = 0.0f;
	  bool hasFired_ = false;

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
	  const float bulletSpeed_ = 20.0f;

	  enum class State
	  {
		  Move,
		  Charge,
		  Fire,
		  Cooldown
	  };
	  State state_ = State::Move;

	  // 揺れフラグ
	  bool isShake_ = false;
	  // 揺れタイマー
	  float shakeTimer_ = 0.0f;
	  const float kShakeTime_ = 0.15f;
	  // 揺れの強さ
	  float shakePower_ = 0.5f;
	  // 揺れの基準位置
	  Vector3 basePosition_;
  };


} // namespace GameObjectComponent