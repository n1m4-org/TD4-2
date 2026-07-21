#pragma once

#include "scene/interface/BaseScene.h"
#include "camerawork/debug/DebugCamera.h"
#include "engine/gameobject/base/GameObject.h"
#include "engine/camerawork/topdown/TopDownCamera.h"
#include "engine/gameobject/component/collision/SphereColliderComponent.h"

/**
 * @brief ゲームオブジェクトの動作確認を行うデバッグ用シーン。
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
	/**
	 * @brief ボムエネミーを生成して必要なコンポーネントを設定する。
	 */
	void InitializeBombEnemy();

	// ディレクショナルライト設定
	static constexpr Vector3 kLightDirection = { -0.2f, -1.0f, 0.3f };
	static constexpr float kLightIntensity = 0.6f;
	static constexpr Vector3 kBombEnemyPosition = { 50.0f, 2.0f, 0.0f };
	static constexpr Vector3 kBombEnemyScale = { 2.0f, 2.0f, 2.0f };
	static constexpr Vector3 kReflectHandLocalPosition = {0.0f, 0.0f, 1.25f};
	static constexpr Vector3 kReflectHandLocalScale = {1.5f, 0.35f, 0.35f};

	// デバッグカメラ
	std::unique_ptr<DebugCamera> debugCamera_;
	// 追従カメラ
	std::unique_ptr<TopDownCamera> topDownCamera_;

	// テスト用のゲームオブジェクト
	std::unique_ptr<GameObject> player_;
	std::unique_ptr<GameObject> groundObject_;
	std::unique_ptr<GameObject> targetObject_;
	std::unique_ptr<GameObject> bumper_;
	std::unique_ptr<GameObject> chargeEnemy_;
	std::unique_ptr<GameObject> bombEnemy_;
	std::unique_ptr<GameObject> hormingTest_;

};
