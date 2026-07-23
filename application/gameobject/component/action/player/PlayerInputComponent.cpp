#include "PlayerInputComponent.h"
#include "input/Input.h"
#include "engine/graphics/2d/SpriteCommon.h"
#include "time/TimeManager.h"


namespace
{
constexpr int kLeftMouseButton = 0;
constexpr int kRightMouseButton = 2;
}

void GameObjectComponent::PlayerInputComponent::Update(GameObject* owner)
{
	ui_.background->Update();

	// 移動方向のベクトル
	// 前後
	if (Input::GetInstance()->PushKey(DIK_W))
	{
		moveDirection_.z = 1.0f;
	}
	else if (Input::GetInstance()->PushKey(DIK_S))
	{
		moveDirection_.z = -1.0f;
	}
	else
	{
		moveDirection_.z = 0.0f;
	}

	// 左右
	if (Input::GetInstance()->PushKey(DIK_A))
	{
		moveDirection_.x = -1.0f;
	}
	else if (Input::GetInstance()->PushKey(DIK_D))
	{
		moveDirection_.x = 1.0f;
	}
	else
	{
		moveDirection_.x = 0.0f;
	}

	// 反射トリガーの判定
	if (Input::GetInstance()->TriggerKey(DIK_SPACE) ||
		Input::GetInstance()->IsMouseButtonTriggered(kLeftMouseButton))
	{
		isReflectTriggered_ = true;
	}
	else
	{
		isReflectTriggered_ = false;
	}

	// ロック対象の確定・切り替えは右クリックの立ち上がりで行う。
	if (Input::GetInstance()->IsMouseButtonTriggered(kRightMouseButton))
	{
		isLockOnTriggered_ = true;
	}
	else
	{
		isLockOnTriggered_ = false;
	}

	// スローモーショントリガーの判定
	if (Input::GetInstance()->TriggerKey(DIK_LSHIFT))
	{
		isSlowMotionTriggered_ = true;
	
    	// UIを2秒間非表示
		isHideUI_ = true;
		uiHideTimer_ = 2.0f;
	}
	else
	{
		isSlowMotionTriggered_ = false;
	}


	if (isHideUI_)
	{
		uiHideTimer_ -= TimeManager::GetInstance().GetGameContext().deltaTime;

		if (uiHideTimer_ <= 0.0f)
		{
			uiHideTimer_ = 0.0f;
			isHideUI_ = false;
		}
	}
}

void GameObjectComponent::PlayerInputComponent::Draw2D()
{
	if (isHideUI_)
	{
		return;
	}

	ui_.background->Draw();

	if (Input::GetInstance()->PushKey(DIK_W))
		ui_.wBlack->Draw();

	if (Input::GetInstance()->PushKey(DIK_A))
		ui_.aBlack->Draw();

	if (Input::GetInstance()->PushKey(DIK_S))
		ui_.sBlack->Draw();

	if (Input::GetInstance()->PushKey(DIK_D))
		ui_.dBlack->Draw();

	if (Input::GetInstance()->PushKey(DIK_LSHIFT))
		ui_.shiftBlack->Draw();

	if (Input::GetInstance()->PushKey(DIK_SPACE))
		ui_.spaceBlack->Draw();

	if (Input::GetInstance()->IsMouseButtonPressed(0))
		ui_.mouseLeft->Draw();

	if (Input::GetInstance()->IsMouseButtonPressed(2))
		ui_.mouseRight->Draw();
}

void GameObjectComponent::PlayerInputComponent::InitializeUI(SpriteCommon* spriteCommon)
{
	spriteCommon_ = spriteCommon;

	ui_.background = CreateSprite("play/playUI.png");

	ui_.wBlack = CreateSprite("play/wB_.png");
	ui_.aBlack = CreateSprite("play/aB.png");
	ui_.sBlack = CreateSprite("play/sB.png");
	ui_.dBlack = CreateSprite("play/dB.png");

	ui_.shiftBlack = CreateSprite("play/shiftB.png");
	ui_.spaceBlack = CreateSprite("play/spaceB.png");

	ui_.mouseLeft = CreateSprite("play/mouseLeft.png");
	ui_.mouseRight = CreateSprite("play/mouseRight.png");
}

std::unique_ptr<Sprite> GameObjectComponent::PlayerInputComponent::CreateSprite(const std::string& tex)
{
	auto sprite = std::make_unique<Sprite>();

	sprite->Initialize(spriteCommon_, tex);
	sprite->SetAnchorPoint({0.0f, 0.0f}); // 左上基準
	sprite->SetPosition({0.0f, 0.0f});	  // 画面左上
	sprite->SetSize({1920.0f, 1080.0f});  // 全て同じサイズ
	sprite->Update();

	return sprite;
}
