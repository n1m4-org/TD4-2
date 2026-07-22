#pragma once
#include <string>
#include "engine/scene/interface/ISceneState.h"

/**
 * @brief タイトル登場演出ステート（Enter）
 */
class TitleEnterState : public ISceneState
{
public:
    /**
     * @brief コンストラクタ
     */
    TitleEnterState() = default;

    /**
     * @brief ステート開始時処理
     * @param scene 所属シーン
     */
    void OnEnter(BaseScene& scene) override;

    /**
     * @brief 毎フレーム更新処理
     * @param scene 所属シーン
     */
    void OnUpdate(BaseScene& scene) override;

    /**
     * @brief 遷移判定処理
     * @param scene 所属シーン
     */
    void CheckTransition(BaseScene& scene) override;

    /**
     * @brief ステート名取得
     * @return ステート名
     */
    const std::string& GetName() const override;

private:
    // 演出タイマー（秒）
    float timer_ = 0.0f;
};
