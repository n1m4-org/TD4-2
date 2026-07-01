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
static constexpr ColliderLayer Reflector = 1 << 7;	  // 反射板
static constexpr ColliderLayer All = 0xFFFFFFFF;
} // namespace CollisionLayer
