#include "ClearScene.h"

#include "engine/scene/factory/SceneFactory.h"
#include "engine/scene/manager/SceneManager.h"
#include "input/Input.h"
#include <Windows.h>

#include "audio/Audio.h"

#ifdef USE_IMGUI
#include "manager/editor/DebugUIManager.h"
#endif

REGISTER_SCENE(ClearScene);

namespace
{
	// 仮のゲームクリア表示用テクスチャ（後で差し替え）
	constexpr char kGameClearTexturePath[] = "./Resources/white1x1.png";
} // namespace

void ClearScene::Initialize()
{
	background_ = std::make_unique<Sprite>();
	background_->Initialize(sceneManager_->GetSpriteCommon(), kGameClearTexturePath);
	background_->SetPosition({0.0f, 0.0f});
	background_->SetSize({Sprite::kCoordinateWidth, Sprite::kCoordinateHeight});
	background_->SetColor({0.0f, 0.1f, 0.3f, 0.8f});

	// もう一度ボタン
	retryButton_ = std::make_unique<MenuButton>();
	retryButton_->Initialize(sceneManager_->GetSpriteCommon(), kGameClearTexturePath,
							 {960.0f, 480.0f}, {360.0f, 100.0f});
	retryButton_->SetColors({0.2f, 0.5f, 0.2f, 0.9f}, {0.4f, 1.0f, 0.4f, 1.0f});

	// 終了ボタン
	quitButton_ = std::make_unique<MenuButton>();
	quitButton_->Initialize(sceneManager_->GetSpriteCommon(), kGameClearTexturePath,
							{960.0f, 640.0f}, {360.0f, 100.0f});
	quitButton_->SetColors({0.5f, 0.2f, 0.2f, 0.9f}, {1.0f, 0.4f, 0.4f, 1.0f});

	// サウンドのロード
	Audio::GetInstance()->LoadWave("select", "select.wav", SoundGroup::SE);
	Audio::GetInstance()->LoadWave("check", "check.wav", SoundGroup::SE);

#ifdef USE_IMGUI
	DebugUIManager::GetInstance()->RegisterDebugUI(this, "Clear Scene", [this]()
	{ this->DrawImGui(); }, DebugUIArea::Hierarchy);
#endif
}

void ClearScene::OnFinalize()
{
	// サウンドの解放
	Audio::GetInstance()->UnloadWave("select");
	Audio::GetInstance()->UnloadWave("check");

#ifdef USE_IMGUI
	if (DebugUIManager::HasInstance())
	{
		DebugUIManager::GetInstance()->UnregisterDebugUI(this);
	}
#endif
}

void ClearScene::CommonUpdate()
{
	if (background_)
	{
		background_->Update();
	}

	const Vector2 mousePos = Input::GetInstance()->GetMousePosition();
	const bool clicked = Input::GetInstance()->IsMouseButtonTriggered(0); 

	// もう一度：ゲーム本編へ
	if (retryButton_)
	{
		if (retryButton_->Update(mousePos, clicked))
		{
			// サウンド再生 音量を調整
			Audio::GetInstance()->PlayWave("check");
			Audio::GetInstance()->SetVolume("check", 1.0f);

			sceneManager_->ChangeScene("WaveScene");
			return; // 二重ChangeScene防止
		}
		if (retryButton_->IsHoveredEnter())
		{
			// サウンド再生 音量を調整
			Audio::GetInstance()->PlayWave("select");
			Audio::GetInstance()->SetVolume("select", 1.0f);
		}
	}

	// 終了：アプリケーションを閉じる
	if (quitButton_)
	{
		if (quitButton_->Update(mousePos, clicked))
		{
			// サウンド再生 音量を調整
			Audio::GetInstance()->PlayWave("check");
			Audio::GetInstance()->SetVolume("check", 1.0f);

			PostQuitMessage(0);
		}
		if (quitButton_->IsHoveredEnter())
		{
			// サウンド再生 音量を調整
			Audio::GetInstance()->PlayWave("select");
			Audio::GetInstance()->SetVolume("select", 1.0f);
		}
	}
}

void ClearScene::Draw2D()
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

void ClearScene::DrawImGui()
{
}