#ifndef WAVE_HPP_
#define WAVE_HPP_
#include <queue>
#include <set>
#include <string>

#include "ReferencePtr.hpp"
#include "Entity/Enemy/EnemyFactory.hpp"
#include "Entity/Enemy/EnemyType.hpp"

class Wave {
    struct SpawnData{
        EnemyType type;
        float spawnTime;
        uint16_t count;
    };

    const std::string FILE_PATH = "Assets/Data/Wave/";
    const std::string FILE_EXTENSION = ".json";

    GESTD::ReferencePtr<EnemyFactory> factory_;

    std::queue<SpawnData> queue_;

    float timer_ = 0.f;

    bool complete_ = false;

public:
    void Initialize(const std::string& _wave, const GESTD::ReferencePtr<EnemyFactory>& _factory);

    void Update();

    bool IsComplete() const;

    std::set<EnemyType> GetEnemyTypes() const;

private:
    void Load(const std::string& _file);

    void Spawn();
};

#endif // WAVE_HPP_
