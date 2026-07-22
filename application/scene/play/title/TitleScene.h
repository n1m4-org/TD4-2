#pragma once
#include <memory>

// scene
#include "engine/scene/interface/BaseScene.h"

/**
 * @brief タイトルシーン。
 * 
 * プレイヤーの入力待ちとタイトル演出を行う。
 */
class TitleScene : public BaseScene
{
public:
    /**
     * @brief 初期化。
     */
    void Initialize() override;
    
    /**
     * @brief 3D描画。
     */
    void Draw3D() override;
    
    /**
     * @brief 2D描画。
     */
    void Draw2D() override;
    
    /**
     * @brief ImGuiデバッグUI。
     */
    void DrawImGui() override;

    /**
     * @brief シーンマネージャーを取得する。
     * @return シーンマネージャーのポインタ
     */
    SceneManager* GetSceneManager() const { return sceneManager_; }

protected:
    void OnFinalize() override;

private:

};


