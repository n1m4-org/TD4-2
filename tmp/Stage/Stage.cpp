#include "Stage.hpp"

#include <algorithm>
#include "imgui.h"
#include "Object/ClearDoor.hpp"

#include "Entity/Player/Player.hpp"
#include "Math/MathUtils.hpp"

#undef min

void Stage::Initialize(const StageStatus& _status, EnemyFactory* _factory, ParticleSystem* _particle) {
    factory_ = _factory;

    auto table = _status.table;

    clearDoor_ = std::make_unique<ClearDoor>();
    clearDoor_->Initialize();
    clearDoor_->SetPosition({0.f, 0.f, 20.f});
    clearDoor_->SetParticleSystem(_particle);
    clearDoor_->SetCallback([this]() {
        if (transitionCallback_) transitionCallback_();
    });

    std::ranges::shuffle(table, MathUtils::GetRandomEngine());

    uint16_t pick = std::min(_status.count, static_cast<uint16_t>(table.size()));
    totalWaves_ = pick; // HUD 表示用に総ウェーブ数を保持
    for (uint16_t i = 0; i < pick; ++i) {
        auto wave = std::make_unique<Wave>();
        wave->Initialize(table[i], factory_);

        // Collect Enemy Types for Stage
        auto enemyTypes = wave->GetEnemyTypes();
        enemyTypes_.insert(enemyTypes.begin(), enemyTypes.end());

        waves_.push(std::move(wave));
    }
}

void Stage::Update() {
    // Update Waves
    if (!waves_.empty()){
        auto& current = waves_.front();

        if(!current->IsComplete()){
            current->Update();
        } else {
            intervalTimer_ += 1.f / 60.f;
            if (enemies_.empty() || WAVE_INTERVAL <= intervalTimer_) {
                intervalTimer_ = 0.f;
                waves_.pop();
            }
        }
    }

    deadCount_ = 0;

    for (auto& enemy : enemies_) {
        if (enemy->IsDead()) {
            deadCount_++;
        }
    }

    // Recieve Pending
    auto pending = factory_->ConsumePending();
    enemies_.insert(enemies_.end(), std::make_move_iterator(pending.begin()), std::make_move_iterator(pending.end()));
    std::erase_if(enemies_, [](const auto& enemy) { return enemy->IsDead(); });

    // Stage Update
    for (auto& enemy : enemies_) {
        enemy->SetWeaponPara(weaponPara_);
        enemy->Update(1.f/60.f);
    }

    // Check Clear
    if (!safe_ && waves_.empty() && enemies_.empty()) {
        safe_ = true;
    }

    if (safe_ && !doorActivated_) {
        doorActivated_ = true;
        clearDoor_->Activate();
    }

    clearDoor_->Update(1.f / 60.f);
}

void Stage::Draw() const {
    for (const auto& enemy : enemies_) {
        enemy->Draw();
    }

    clearDoor_->Draw();
}

void Stage::SetTransitionCallback(const std::function<void()>& _callback) {
    transitionCallback_ = _callback;
}

bool Stage::IsSafe() const {
    return safe_;
}

const std::set<EnemyType>& Stage::GetEnemyTypes() const {
    return enemyTypes_;
}

void Stage::Debug() {
#ifdef _DEBUG
    ImGui::SeparatorText("Stage");

    ImGui::Text("Enemies : %zu", enemies_.size());
    ImGui::Text("Waves   : %zu", waves_.size());

    if (ImGui::Checkbox("Safe", &safe_)) {
        if (safe_ && !doorActivated_) {
            doorActivated_ = true;
            clearDoor_->Activate();
        }
    }

    ImGui::Checkbox("Door Activated", &doorActivated_);
#endif
}
