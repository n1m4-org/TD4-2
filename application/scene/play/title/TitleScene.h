#pragma once
#include <memory>

// scene
#include "engine/scene/interface/BaseScene.h"
#include "graphics/2d/Sprite.h"
#include "graphics/3d/Object3d.h"
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
	std::unique_ptr<Object3d> playerModel_;
	std::unique_ptr<Object3d> enemyModel_;

	// アニメーション用タイマー
	float logoAnimTimer_ = 0.0f;
	float startTextAnimTimer_ = 0.0f;
	float cameraAngle_ = 0.0f;

	// 決定演出フラグ・タイマー・ボム吹っ飛びパラメータ
	bool isDecided_ = false;
	float decisionTimer_ = 0.0f;
	Vector3 enemyStartPos_ = { 5.0f, 1.5f, 2.0f };
	Vector3 enemyRot_ = { 0.0f, 0.0f, 0.0f };

	// シーン遷移時にフェード演出
	SceneTransitionEffect transitionEffect_;
};


