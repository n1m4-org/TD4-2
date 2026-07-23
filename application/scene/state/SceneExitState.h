#pragma once
#include <string>
#include "engine/scene/interface/ISceneState.h"
#include "application/transition/SceneTransitionEffect.h"

/**
 * @brief 汎用退出演出ステート（Exit）
 * 
 * シーン終了時のフェードイン演出（画面が覆われる演出）を行い、
 * 完了後にコンストラクタ等で指定された次のシーンへ切り替える。
 */
class SceneExitState : public ISceneState
{
public:
    /**
     * @brief コンストラクタ
     * @param transitionEffect シーン遷移エフェクトのポインタ
     * @param nextSceneName 演出完了後に切り替える次のシーン名（例: "Title", "Test" など）
     * @param duration 演出時間（秒）
     * @param mode 遷移演出モード
     * @param easeType イージングの種類
     */
    SceneExitState(
        SceneTransitionEffect* transitionEffect,
        const std::string& nextSceneName = "Title",
        float duration = 1.5f,
        TransitionMode mode = TransitionMode::BottomToTop,
        SceneTransitionEase easeType = SceneTransitionEase::InSine
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

    /**
     * @brief 遷移先の次シーン名を取得
     * @return シーン名
     */
    const std::string& GetNextSceneName() const { return nextSceneName_; }

    /**
     * @brief 遷移先の次シーン名属性を設定
     * @param nextSceneName シーン名
     */
    void SetNextSceneName(const std::string& nextSceneName) { nextSceneName_ = nextSceneName; }

private:
    SceneTransitionEffect* transitionEffect_ = nullptr;
    std::string nextSceneName_ = "Title";
    float duration_ = 1.5f;
    TransitionMode mode_ = TransitionMode::BottomToTop;
    SceneTransitionEase easeType_ = SceneTransitionEase::InSine;
    bool sceneChanged_ = false;
};
