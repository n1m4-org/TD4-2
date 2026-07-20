#ifndef STAGE_HPP_
#define STAGE_HPP_
#include <memory>
#include <queue>
#include <functional>
#include <set>

#include "StageStatus.hpp"
#include "Entity/Enemy/BaseEnemy.hpp"
#include "Wave/Wave.hpp"
#include "Object/ClearDoor.hpp"
#include "Entity/ModuleParameter.hpp"
#include "src/ParticleSystem/ParticleSystem.hpp"

class Stage{    
    const float WAVE_INTERVAL = 5.f;

    GESTD::ReferencePtr<EnemyFactory> factory_;

    std::queue<std::unique_ptr<Wave>> waves_;

    std::set<EnemyType> enemyTypes_;

    // Stage Variables
    // clear flag => prepare module before next stage 
    bool safe_ = false;

    // Objects
    std::vector<std::unique_ptr<BaseEnemy>> enemies_;

    std::function<void()> transitionCallback_;

    std::unique_ptr<ClearDoor> clearDoor_;

    ModuleParameter weaponPara_;

    bool doorActivated_ = false;

    float intervalTimer_ = 0.f;

    int deadCount_ = 0;

    uint16_t totalWaves_ = 0; // このステージの総ウェーブ数（HUD表示用）

public:
    void Initialize(const StageStatus& _status, EnemyFactory* _factory, ParticleSystem* _particle = nullptr);
    void Update();
    void Draw() const;

    void SetTransitionCallback(const std::function<void()>& _callback);

    bool IsSafe() const;

    const std::set<EnemyType>& GetEnemyTypes() const;

    void Debug();

    void SetWeaponPara(ModuleParameter para) { weaponPara_ = para; }

    int GetDeadCount() const { return deadCount_; }

    // HUD 表示用: 残り（生存中）敵数
    size_t GetAliveEnemyCount() const { return enemies_.size(); }

    // HUD 表示用: 総ウェーブ数
    uint16_t GetTotalWaves() const { return totalWaves_; }

    // HUD 表示用: 現在のウェーブ番号（1 始まり、踏破後は総数でクランプ）
    uint16_t GetCurrentWave() const {
        const uint16_t remaining = static_cast<uint16_t>(waves_.size());
        if (remaining >= totalWaves_) { return 1; }
        const uint16_t current = static_cast<uint16_t>(totalWaves_ - remaining + 1);
        return current > totalWaves_ ? totalWaves_ : current;
    }

private:
};

#endif // STAGE_HPP_
