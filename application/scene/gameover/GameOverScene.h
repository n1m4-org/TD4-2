#pragma once
#include <memory>

#include "application/scene/ui/MenuButton.h"
#include "engine/graphics/2d/Sprite.h"
#include "engine/scene/interface/BaseScene.h"

class GameOverScene : public BaseScene
{
public:
	void Initialize() override;
	void Draw2D() override;
	void CommonUpdate() override;

protected:
	void OnFinalize() override;
	void DrawImGui() override;

private:
	std::unique_ptr<Sprite> background_;
	// もう一度
	std::unique_ptr<MenuButton> retryButton_;
	// 終了
	std::unique_ptr<MenuButton> quitButton_;  
};