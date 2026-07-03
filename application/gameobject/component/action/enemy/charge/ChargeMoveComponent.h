#pragma once

#include "engine/gameobject/component/base/IActionComponent.h"

namespace GameObjectComponent
{

	// 前方宣言
	class PhysicsComponent;

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


  private:

	  // プレイヤーのポインタ
	  GameObject* player_ = nullptr; 

	  // 物理
	  PhysicsComponent* physics_ = nullptr;

	  // 移動速度
	  float moveSpeed_ = 5.0f;

	  // チャージ開始距離
	  float chargeStartDistance_ = 30.0f;
	  // チャージ開始フラグ
	  bool isChargeStart_ = false;
	  // チャージ時間
	  const float kChargeTime = 1.0f;
	  float chargeTime_ = 0.0f;

	  // 攻撃クールタイム
	  const float kCoolTime = 2.0f;
	  float coolTime_ = 0.0f;

	  // 攻撃フラグ
	  bool isAttacking_ = false;

	  enum class State
	  {
		  Move,
		  Charge,
		  Fire,
		  Cooldown
	  };
	  State state_ = State::Move;

  };


} // namespace GameObjectComponent