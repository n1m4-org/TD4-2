#pragma once
#include <string>
#include "engine/scene/interface/ISceneState.h"
#include "application/transition/SceneTransitionEffect.h"

/**
 * @brief 汎用登場演出ステート（Enter）
 * 
 * シーン開始時のフェードアウト演出（画面の覆いが明ける演出）を行い、
 * 完了後に指定された次のシーン内ステートへ遷移する。
 */
class SceneEnterState : public ISceneState
{
public:
    /**
     * @brief コンストラクタ
     * @param transitionEffect シーン遷移エフェクトのポインタ
     * @param nextStateName 演出完了後に遷移するシーン内ステート名
     * @param duration 演出時間（秒）
     * @param mode 遷移演出モード
     * @param easeType イージングの種類
     */
    SceneEnterState(
        SceneTransitionEffect* transitionEffect,
        const std::string& nextStateName = "",
        float duration = 2.0f,
        TransitionMode mode = TransitionMode::TopToBottom,
        SceneTransitionEase easeType = SceneTransitionEase::OutSine
    );

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
    SceneTransitionEffect* transitionEffect_ = nullptr;
    std::string nextStateName_ = "";
    float duration_ = 2.0f;
    TransitionMode mode_ = TransitionMode::TopToBottom;
    SceneTransitionEase easeType_ = SceneTransitionEase::OutSine;
};
