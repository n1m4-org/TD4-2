#pragma once
#include <memory>

// scene
#include "engine/scene/interface/BaseScene.h"
#include "graphics/2d/Sprite.h"
#include "transition/SceneTransitionEffect.h"
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

public: // ステートクラスへのアクセッサ
	// シーントランジションの取得
	SceneTransitionEffect& GetTransitionEffect() { return transitionEffect_; }

protected:
    void OnFinalize() override;
	void CommonUpdate() override;

private:
	// タイトルロゴ
	std::unique_ptr<Sprite> titleLogo_;
	// スタートテキスト
	std::unique_ptr<Sprite> startText_;
	
	// アニメーション用タイマー
	float logoAnimTimer_ = 0.0f;
	float startTextAnimTimer_ = 0.0f;

	// シーン遷移時にフェード演出
	SceneTransitionEffect transitionEffect_;
};


