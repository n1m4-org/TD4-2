#pragma once
#include "engine/gameobject/component/collision/CollisionLayer.h"

namespace CollisionLayer
{
using GameObjectComponent::ColliderLayer;

static constexpr ColliderLayer None = 0;
static constexpr ColliderLayer Player = 1 << 0;		  // プレイヤー
static constexpr ColliderLayer PlayerBullet = 1 << 1; // プレイヤーの弾
static constexpr ColliderLayer Enemy = 1 << 2;		  // 敵
static constexpr ColliderLayer EnemyBullet = 1 << 3;  // 敵の弾
static constexpr ColliderLayer Stage = 1 << 4;		  // 地形
static constexpr ColliderLayer Terrain = 1 << 5;	  // 地形
static constexpr ColliderLayer Bumpers = 1 << 6;	  // バンパー
static constexpr ColliderLayer Tackle = 1 << 7;		  // タックル
static constexpr ColliderLayer Bomb = 1 << 8;		  // 爆弾
static constexpr ColliderLayer Charge = 1 << 9;		  // チャージ
static constexpr ColliderLayer Horming = 1 << 10;	  // ホーミング
static constexpr ColliderLayer All = 0xFFFFFFFF;
} // namespace CollisionLayer
