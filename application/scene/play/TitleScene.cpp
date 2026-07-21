#include "TitleScene.h"

// engine/ecs
#include "engine/ecs/components/TransformComponent.h"
#include "engine/ecs/components/InstancedRenderComponent.h"
#include "engine/ecs/system/InstancedRenderSystem.h"
#include "engine/ecs/system/HierarchySystem.h"

// engine/graphics
#include "engine/manager/graphics/ModelManager.h"
#include "engine/graphics/3d/InstancedModelRenderer.h"
#include "engine/graphics/3d/Object3dCommon.h"

// audio
#include "audio/Audio.h"
// scene
#include "engine/scene/manager/SceneManager.h"
#include "engine/scene/factory/SceneFactory.h"
// input
#include "input/Input.h"
// graphics / manager
#include "manager/effect/PostProcessManager.h"
#include "manager/graphics/LineManager.h"
#include <effects/particle/ParticleManager.h>
#include "effects/particle/ParticleEffect.h"
#ifdef USE_IMGUI
#include "manager/editor/DebugUIManager.h"
#endif

namespace
{
    /**
     * @brief タイトルシーンのプレイステート
     */
    class TitlePlayingState : public ISceneState
    {
    public:
        explicit TitlePlayingState(TitleScene* scene) : scene_(scene) {}

        const std::string& GetName() const override
        {
            static const std::string name = "Title";
            return name;
        }

    private:
        TitleScene* scene_;
    };
}

REGISTER_SCENE(TitleScene);

void TitleScene::Initialize()
{
#ifdef USE_IMGUI
    DebugUIManager::GetInstance()->RegisterDebugUI(this, "Title Scene", [this]() { this->DrawImGui(); }, DebugUIArea::Hierarchy);
#endif

    RegisterState("Playing", std::make_unique<TitlePlayingState>(this));
    ChangeState("Playing");
}

void TitleScene::OnFinalize()
{
#ifdef USE_IMGUI
    if (DebugUIManager::HasInstance()) {
        DebugUIManager::GetInstance()->UnregisterDebugUI(this);
    }
#endif
}

void TitleScene::Draw3D()
{

}

void TitleScene::Draw2D()
{
}

void TitleScene::DrawImGui()
{
}
