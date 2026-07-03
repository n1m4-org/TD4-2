#pragma once
#include "scene/interface/BaseScene.h"
#include "camerawork/debug/DebugCamera.h"
#include "engine/gameobject/base/GameObject.h"
#include "engine/camerawork/follow/FollowCamera.h"
/**
 * @brief ゲームオブジェクトのテストを行うデバッグ用シーン
 */
class TestScene : public BaseScene
{
public:
	void Initialize() override;
	void Finalize() override;
	void Draw3D() override;
	void Draw2D() override;
	void DrawShadow() override;
	void DrawGBuffer() override;

#ifdef USE_IMGUI
	void DrawImGui();
#endif

protected:
	void OnUpdatePlaying() override;

private:
	// ディレクショナルライト設定
	static constexpr Vector3 kLightDirection = { -0.2f, -1.0f, 0.3f };
	static constexpr float kLightIntensity = 0.6f;

	// デバッグカメラ
	std::unique_ptr<DebugCamera> debugCamera_;
	// 追従カメラ
	std::unique_ptr<FollowCamera> followCamera_;

	// テスト用のゲームオブジェクト
	std::unique_ptr<GameObject> cubeObject_;
	std::unique_ptr<GameObject> groundObject_;
	std::unique_ptr<GameObject> targetObject_;
	std::unique_ptr<GameObject> bumper_;
	std::unique_ptr<GameObject> bombEnemy_;
};
