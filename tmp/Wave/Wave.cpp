#include "Wave.hpp"

#include <fstream>

#include "json.hpp"
#include "Entity/Enemy/BaseEnemy.hpp"
#include "externals/MagicEnum/magic_enum.hpp"

void Wave::Initialize(const std::string& _wave, const GESTD::ReferencePtr<EnemyFactory>& _factory) {
    Load(_wave);
    factory_ = _factory;
}

void Wave::Update() {
    if(queue_.empty()){
        complete_ = true;
        return;
    }

    timer_ += 1.f / 60.f;
    
    // Spawn Enemy
    while (!queue_.empty() && queue_.front().spawnTime <= timer_) {
        Spawn();
    }
}

bool Wave::IsComplete() const {
    return complete_;
}

std::set<EnemyType> Wave::GetEnemyTypes() const {
    std::set<EnemyType> types;
    auto temp = queue_;
    while (!temp.empty()) {
        types.insert(temp.front().type);
        temp.pop();
    }
    return types;
}

void Wave::Load(const std::string& _file) {
    // Available directory check
    if (!std::filesystem::exists(FILE_PATH + _file + FILE_EXTENSION)) { return; }

    std::ifstream file(FILE_PATH + _file + FILE_EXTENSION);
    if (!file) { return; }

    // Load the file
    nlohmann::json jsonData;
    file >> jsonData;

    if (!jsonData.contains("wave")) { return; }
    if (!jsonData["wave"].is_array()) { return; }
    for (const auto& item : jsonData["wave"]) {
        SpawnData data{};
        data.spawnTime = item["timing"].get<float>();

        auto type = magic_enum::enum_cast<EnemyType>(item["type"].get<std::string>());
        if (!type.has_value()) continue;
        data.type = type.value();
        data.count = item["count"].get<uint16_t>();

        queue_.push(data);
    }
}

void Wave::Spawn() {
    // Spawn Queue Front Enemy
    const EnemyType type = queue_.front().type;
    const uint16_t count = queue_.front().count;

    factory_->CreateEnemy(type, count);
    queue_.pop();
}
