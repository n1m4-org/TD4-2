#pragma once
#include <memory>

#include "graphics/3d/Object3d.h"

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

    /**
     * @brief 決定ボタン押下時の演出開始処理
     */
    void OnDecision();

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
	
	// 3D演出用オブジェクト
	std::unique_ptr<Object3d> skydome_;
	std::unique_ptr<Object3d> ground_;
	std::unique_ptr<Object3d> playerModel_;
	std::unique_ptr<Object3d> enemyModel_;

	// アニメーション用タイマー
	float logoAnimTimer_ = 0.0f;
	float startTextAnimTimer_ = 0.0f;
	float cameraAngle_ = 0.0f;

	// 決定演出フラグ・タイマー
	bool isDecided_ = false;
	float decisionTimer_ = 0.0f;

	// シーン遷移時にフェード演出
	SceneTransitionEffect transitionEffect_;
};


