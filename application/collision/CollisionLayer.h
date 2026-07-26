#pragma once
#include "engine/gameobject/component/collision/CollisionLayer.h"

namespace CollisionLayer
{
using GameObjectComponent::ColliderLayer;

static constexpr ColliderLayer None = 0;
static constexpr ColliderLayer Player = 1 << 0; // プレイヤー
static constexpr ColliderLayer Enemy = 1 << 1;  // 敵
static constexpr ColliderLayer Stage = 1 << 2;  // 地形
static constexpr ColliderLayer All = 0xFFFFFFFF;
} // namespace CollisionLayer
