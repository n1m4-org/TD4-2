#include "StageSystem.hpp"

#include <fstream>
#include <string>

#include "imgui.h"
#include "json.hpp"
#include "Camera/Controller/CameraController.hpp"
#include "Pattern/Singleton.hpp"

#undef min
#undef max

void StageSystem::Initialize(ParticleSystem* _particle, Ground* _ground) {
    particle_ = _particle;
    ground_   = _ground;
    current_ = 0;

    InitializePlayer();

    enemyFactory_ = std::make_unique<EnemyFactory>();
    enemyFactory_->Initialize(player_.get(), ground_, particle_);

    transition_ = std::make_unique<Sprite>();
    transition_->Initialize("white_x16.png");
    transition_->SetColor({1.f, 1.f, 1.f, 0.f});
    transition_->SetSize({ 1920.f, 1080.f });
    transition_->SetPosition({ 0.f, 0.f });
    transition_->SetAnchorPoint({});

    Load();

    stage_ = CreateStage(current_);

    ui_ = std::make_unique<Ui::Canvas>();
    ui_->Setup("safezone");
    ui_->SetActive(false);

    // 戦闘中の常時表示HUD（ウェーブ番号・残り敵数）
    hud_ = std::make_unique<Ui::Canvas>();
    hud_->Setup("hud");
    hud_->SetActive(true);

    moduleManager_ = std::make_unique<ModuleManager>();
    moduleManager_->Initialize();
    moduleManager_->HideNotify();
}

void StageSystem::InitializePlayer() {
    player_ = std::make_unique<Player>();
    player_->Initialize();
    player_->SetPosition({ 0.0f, 0.0f, 0.0f });
    player_->SetGround(ground_); // 移動範囲をフィールドに制限するため参照を渡す

    inputHandler_ = std::make_unique<PlayerInputHandler>();
    inputHandler_->Initialize();
    inputHandler_->SetCursorLocked(isCursorLocked_);

    auto camera = Singleton<CameraController>::GetInstance()->GetActive();
    player_->SetPlayerView(std::make_unique<PlayerView>(camera));
}

void StageSystem::Update() {
    if (state_ != State::None) { Transition(); return; }
    if (!stage_) return;

    GESTD::ReferencePtr<Input> input = Singleton<Input>::GetInstance();
    if (input && input->IsTrigger(DIK_TAB) && stage_->IsSafe()) {
        isCursorLocked_ = !isCursorLocked_;
        inputHandler_->SetCursorLocked(isCursorLocked_);
        moduleManager_->SetEditMode(!moduleManager_->IsEditMode());
        input->SetCursorVisible(!moduleManager_->IsEditMode());
    }
#ifdef _DEBUG
    if (input && input->IsTrigger(DIK_F3)) {
        isCursorLocked_ = !isCursorLocked_;
        inputHandler_->SetCursorLocked(isCursorLocked_);
    }
#endif

    player_->ApplyInputState(inputHandler_->Update());
    player_->Update(1.f / 60.f);
    player_->SetModulePara(moduleManager_->GetModuleList()->GetAllParameter());

    stage_->SetWeaponPara(player_->GetWeapon().GetApplyPara());
    stage_->Update();

    for (int i = 0; i < stage_->GetDeadCount(); i++) {

        if (percent_ <= MathUtils::Random(0.f,1.f) ) { continue; }

        moduleManager_->CreateRandomModule();
    }

    moduleManager_->Update();

    UpdateHud();

    if (stage_->IsSafe() && !next_) {
        const uint16_t nextStage = current_ + 1;
        if (!stageStatus_.contains(nextStage)) {
            // 次ステージが無い = 最終ステージ踏破 = ゲームクリア
            cleared_ = true;
            return;
        }
        next_ = CreateStage(current_ + 1);
        BuildUi();
    }
}

void StageSystem::Draw() const {
    if (stage_) stage_->Draw();
    player_->Draw();
    moduleManager_->Draw();
    if (state_ != State::None) {
        transition_->Draw();
    }
}

void StageSystem::Load() {
    nlohmann::json json;
    std::ifstream(FILE_PATH) >> json;

    for (auto& [key, value]: json.items()) {
        StageStatus status{};
        status.count = value["waves"].get<uint16_t>();

        for (const auto& table : value["difficulty_table"]) {
            const auto& fileName = table["difficulty"].get<std::string>() + "_" + std::to_string(table["numbering"].get<uint16_t>());
            status.table.push_back(fileName);
        }

        stageStatus_[static_cast<uint16_t>(std::stoi(key))] = status;
    }
}

std::unique_ptr<Stage> StageSystem::CreateStage(const uint16_t _number) {
    auto stage = std::make_unique<Stage>();
    stage->Initialize(stageStatus_[_number], enemyFactory_.get(), particle_);
    stage->SetTransitionCallback([this] {
        timer_ = 0.f;
        state_ = State::TransitionIn;
        ui_->SetActive(false);
    });

    return std::move(stage);
}

void StageSystem::Transition() {
    const float dt = 1.f / 60.f;

    timer_ += dt;
    const float alpha = std::clamp(timer_ / TRANSITION, 0.f, 1.f);

    switch(state_){
    case State::TransitionIn:
        transition_->SetColor({1.f, 1.f, 1.f, alpha});

        if (TRANSITION <= timer_) {
            stage_ = std::move(next_);
            next_.reset();
            ++current_;
            timer_ = 0.f;
            state_ = State::TransitionOut;

            player_->SetPosition({0.f, 0.f, 0.f});
        }
        break;
    case State::TransitionOut:
        transition_->SetColor({1.f, 1.f, 1.f, 1.f - alpha});

        if (TRANSITION <= timer_) {
            transition_->SetColor({1.f, 1.f, 1.f, 0.f});
            timer_ = 0.f;
            state_ = State::None;
        }

        break;
    }
}

void StageSystem::Debug() {
    ImGui::Begin("Stage");

    ImGui::Text("Stage : %d / %d", current_, static_cast<uint16_t>(stageStatus_.size() - 1));

    constexpr const char* stateNames[] = { "None", "TransitionIn", "TransitionOut" };
    ImGui::Text("State : %s", stateNames[static_cast<int>(state_)]);

    if (state_ != State::None) {
        ImGui::ProgressBar(timer_ / TRANSITION, {-1, 0});
    }

    if (stage_) {
        stage_->Debug();
    }

    ImGui::Separator();

    //ImGui::DragFloat("Transition", &TRANSITION, 0.01f, 0.1f, 5.0f);

    ImGui::SeparatorText("Modules");
    moduleManager_->Debug();

    ImGui::End();
}

void StageSystem::BuildUi() const {
    const auto& types = next_->GetEnemyTypes();

    auto* tank  = ui_->FindElementByName("TankIcon");
    auto* scout = ui_->FindElementByName("ScoutIcon");
    auto* zako  = ui_->FindElementByName("ZakoIcon");

    if (tank)  tank->SetVisible(types.contains(EnemyType::Tank));
    if (scout) scout->SetVisible(types.contains(EnemyType::Scout));
    if (zako)  zako->SetVisible(types.contains(EnemyType::Zako));

    ui_->SetActive(true);
}

void StageSystem::UpdateHud() const {
    if (!hud_ || !stage_) { return; }

    // ウェーブ番号: "WAVE 1 / 3"
    if (auto* waveText = hud_->FindElementByName("WaveText")) {
        const std::string text = "WAVE " + std::to_string(stage_->GetCurrentWave())
                               + " / " + std::to_string(stage_->GetTotalWaves());
        waveText->SetText(text);
    }

    // 残り敵数: "ENEMY 4"
    if (auto* enemyText = hud_->FindElementByName("EnemyText")) {
        const std::string text = "ENEMY " + std::to_string(stage_->GetAliveEnemyCount());
        enemyText->SetText(text);
    }
}