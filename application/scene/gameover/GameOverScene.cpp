#include "GameOverScene.h"


#include "engine/scene/factory/SceneFactory.h"
#include "engine/scene/manager/SceneManager.h"
#include "input/Input.h"
#include <Windows.h>

#ifdef USE_IMGUI
#include "manager/editor/DebugUIManager.h"
#endif

REGISTER_SCENE(GameOverScene);

namespace
{
	// 仮のゲームオーバー表示用テクスチャ（後で差し替え）
	constexpr char kGameOverTexturePath[] = "./Resources/white1x1.png";
} // namespace

void GameOverScene::Initialize()
{
	background_ = std::make_unique<Sprite>();
	background_->Initialize(sceneManager_->GetSpriteCommon(), kGameOverTexturePath);
	background_->SetPosition({0.0f, 0.0f});
	background_->SetSize({Sprite::kCoordinateWidth, Sprite::kCoordinateHeight});
	background_->SetColor({0.0f, 0.0f, 0.0f, 0.8f}); // 半透明の黒幕（仮）


	// もう一度ボタン
	retryButton_ = std::make_unique<MenuButton>();
	retryButton_->Initialize(sceneManager_->GetSpriteCommon(), kGameOverTexturePath,
							 {960.0f, 480.0f}, {360.0f, 100.0f});
	retryButton_->SetColors({0.2f, 0.5f, 0.2f, 0.9f}, {0.4f, 1.0f, 0.4f, 1.0f});

	// 終了ボタン
	quitButton_ = std::make_unique<MenuButton>();
	quitButton_->Initialize(sceneManager_->GetSpriteCommon(), kGameOverTexturePath,
							{960.0f, 640.0f}, {360.0f, 100.0f});
	quitButton_->SetColors({0.5f, 0.2f, 0.2f, 0.9f}, {1.0f, 0.4f, 0.4f, 1.0f});
#ifdef USE_IMGUI
	DebugUIManager::GetInstance()->RegisterDebugUI(this, "GameOver Scene", [this]()
	{ this->DrawImGui(); }, DebugUIArea::Hierarchy);
#endif
}

void GameOverScene::OnFinalize()
{
#ifdef USE_IMGUI
	if (DebugUIManager::HasInstance())
	{
		DebugUIManager::GetInstance()->UnregisterDebugUI(this);
	}
#endif
}

void GameOverScene::Draw2D()
{
	if (background_)
	{
		background_->Draw();
	}
	if (retryButton_)
	{
		retryButton_->Draw();
	}
	if (quitButton_)
	{
		quitButton_->Draw();
	}
}

void GameOverScene::CommonUpdate()
{
	if (background_)
	{
		background_->Update();
	}

	const Vector2 mousePos = Input::GetInstance()->GetMousePosition();
	const bool clicked = Input::GetInstance()->IsMouseButtonTriggered(0); // 左クリック

	// もう一度：ゲーム本編へ
	if (retryButton_ && retryButton_->Update(mousePos, clicked))
	{
		sceneManager_->ChangeScene("WaveScene");
		return; 
	}

	// 終了：アプリケーションを閉じる
	if (quitButton_ && quitButton_->Update(mousePos, clicked))
	{
		PostQuitMessage(0);
	}
}

void GameOverScene::DrawImGui()
{
}