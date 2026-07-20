#ifndef STAGE_SYSTEM_HPP_
#define STAGE_SYSTEM_HPP_
#include <memory>
#include <unordered_map>

#include "Sprite.hpp"
#include "Ui/UserInterface.hpp"
#include "Stage.hpp"
#include "StageStatus.hpp"
#include "Entity/Enemy/EnemyFactory.hpp"
#include "Entity/Ground.hpp"
#include "Entity/Player/Player.hpp"
#include "Entity/Player/PlayerView.hpp"
#include "InputHandler/PlayerInputHandler.hpp"
#include "Module/Base/ModuleManager.hpp"
#include "src/ParticleSystem/ParticleSystem.hpp"

class StageSystem {
    enum class State {
        None,
        TransitionIn,
        TransitionOut
    };

    const std::string FILE_PATH = "Assets/Data/Stage/data.json";

    std::unique_ptr<Stage> stage_;
    std::unique_ptr<Stage> next_;

    std::unordered_map<uint16_t, StageStatus> stageStatus_;

    uint16_t current_ = 0;

    std::unique_ptr<Sprite> transition_;

    std::unique_ptr<Player> player_;
    std::unique_ptr<PlayerInputHandler> inputHandler_;

    bool isCursorLocked_ = true;

    std::unique_ptr<EnemyFactory> enemyFactory_;

#ifndef _DEBUG 
    const 
#endif
       float TRANSITION = 0.5f;
    
    const float percent_ = 0.2f;

    float timer_ = 0.f;

    State state_ = State::None;

    bool cleared_ = false; // 最終ステージ踏破フラグ

    ParticleSystem* particle_ = nullptr;
    Ground* ground_ = nullptr;

    std::unique_ptr<Ui::Canvas> ui_;

    std::unique_ptr<Ui::Canvas> hud_; // 戦闘中の常時表示HUD（ウェーブ/残り敵数）

    std::unique_ptr<ModuleManager> moduleManager_;

public:
    void Initialize(ParticleSystem* _particle = nullptr, Ground* _ground = nullptr);
    void Update();
    void Draw() const;
    void Debug();

    Player* GetPlayer() const { return player_.get(); }

    // ゲーム結果の条件判定（シーン遷移は呼び出し側=GameSceneが行う）
    bool IsGameOver() const { return player_ && player_->IsDead(); } // 負け条件
    bool IsCleared() const { return cleared_; } // 勝ち条件（最終ステージ踏破）

private:
    void Load();
    void InitializePlayer();

    std::unique_ptr<Stage> CreateStage(uint16_t _number);

    void Transition();

    void BuildUi() const;

    void UpdateHud() const; // ウェーブ番号・残り敵数をHUDテキストへ反映
};

#endif
