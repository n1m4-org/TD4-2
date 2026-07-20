# Wave System Design

## 1. 目的

ゲーム開始後に、JSONで定義されたWave候補から規定数をランダムに選択し、選択されたWaveを順番に実行する。

各Waveは敵そのものを所有する場所ではない。指定時刻に指定地点への敵スポーン命令を発行し、そのWaveから生成された敵の終了を追跡するランタイム単位とする。

## 2. 前提

### 2.1 GameObjectとECSの現状

エンジン内には、以下の異なる二つの実行モデルが存在する。

- `GameObject` 系
  - `GameObjectManager` がGameObjectを管理する。
  - `IGameObjectComponent` の仮想関数を通して各GameObjectを更新する。
  - 現在の敵、プレイヤー、コリジョンはこの系統で実装されている。
  - 現在のゲームループで実際に使用されている。
- `engine/ecs` 系
  - `Registry`、世代付き `EntityID`、コンポーネント配列、`SystemManager` から構成される。
  - Entityの破棄は `DestroyEntityDeferred()` と `FlushGarbageCollection()` を前提とする。
  - 現時点ではGameObjectとの自動同期や、ゲームループ上のRegistry所有者は確認できない。

Wave機能自体をECSにする必要はない。ただし、将来GameObjectからECS Entityへ生成対象が変わってもWave進行ロジックを変更しなくてよい境界を設ける。

### 2.2 設計方針

- Wave進行はGameObjectにもECSにも直接依存させない。
- 静的なJSONデータと、実行中のランタイム状態を分離する。
- 敵の生成と所有をWaveから分離する。
- Wave完了判定では、ゲーム内のEnemy全体ではなく、そのWaveが生成した敵だけを追跡する。
- `WaveSystem` はECSの `ISystem` を継承しない。

## 3. 用語

| 用語 | 意味 |
| --- | --- |
| `WaveSystem` | Wave候補の選択と複数Waveの進行を管理する制御クラス |
| `Wave` | 選択された一つのWave定義を実行するランタイムオブジェクト |
| `WaveConfig` | JSONへ保存されるWave全体の静的設定 |
| `WaveDefinition` | 一つのWaveを規定する不変データ |
| `SpawnCommand` | 指定時刻に敵を指定地点へ生成する命令 |
| `SpawnID` | Waveと生成バックエンドの間で敵を識別する論理ID |
| `IEnemySpawner` | GameObjectやECS Entityの具体的な生成を隠蔽するインターフェース |
| `ISpawnPointResolver` | スポーン地点IDを実座標へ変換するインターフェース |

内部では `Wave` をスポーン命令群の実行単位として扱う。プレイヤーに表示する名称は、ゲーム仕様に応じてWave以外へ変更してもよい。

## 4. 全体構成

```text
GameScene
  `- gameplay::WaveSystem
       |- WaveConfigへの参照
       |- 選択済みWave列
       `- currentWave
            |- 経過時間
            |- 次のSpawnCommand位置
            `- 生存中SpawnID

WaveSystem
  -> Waveを選択・開始・更新

Wave
  -> ISpawnPointResolverへ地点解決を要求
  -> IEnemySpawnerへスポーン命令を送信

GameObjectEnemySpawner
  -> EnemyFactoryを使用
  -> GameObjectManagerを通してGameObjectを生成
```

将来ECSを使用する場合は、`GameObjectEnemySpawner` の代わりに `EcsEnemySpawner` を実装する。

```text
GameObjectEnemySpawner: SpawnID <-> GameObject*
EcsEnemySpawner:        SpawnID <-> EntityID
```

## 5. WaveSystem

### 5.1 責務

- `WaveConfig` からWave候補を受け取る。
- 規定数のWaveをランダムに選択する。
- 選択したWaveの順序を保持する。
- 現在のWaveを生成、開始、更新する。
- Wave完了後、待機時間を経て次のWaveへ進む。
- 全Wave完了をGameSceneへ通知可能にする。
- デバッグ用に乱数シード、選択結果、現在位置を公開する。

### 5.2 責務外

- GameObjectやEntityの具体的な構築。
- 敵のAI、HP、衝突、死亡演出。
- 敵オブジェクトの所有。
- スポーン地点IDから座標への変換。
- JSON構造体の編集方法。

### 5.3 状態

```cpp
enum class WaveSystemState
{
    Idle,
    WaitingToStart,
    Running,
    WaitingForNext,
    Completed,
    Stopped
};
```

基本的な遷移は次のとおり。

```text
Idle
  -> Start()
WaitingToStart
  -> 開始待機時間の経過
Running
  -> 現在Waveの完了
WaitingForNext
  -> 次Waveが存在する: WaitingToStart
  -> 次Waveが存在しない: Completed
```

### 5.4 想定API

```cpp
namespace gameplay
{
    class WaveSystem
    {
    public:
        void Initialize(
            const WaveConfig* config,
            IEnemySpawner* enemySpawner,
            ISpawnPointResolver* spawnPointResolver);

        void Start(uint32_t seed);
        void Update(float deltaTime);
        void Stop();
        void Reset();

        bool IsCompleted() const;
        size_t GetCurrentWaveNumber() const;
        size_t GetSelectedWaveCount() const;

    private:
        void SelectWaves(uint32_t seed);
        void StartCurrentWave();
        void AdvanceWave();
    };
}
```

シングルトンにはせず、ゲームシーンまたはゲーム進行を統括するクラスが所有する。

## 6. Wave

### 6.1 責務

- Wave内の経過時間を進める。
- 次に発行する `SpawnCommand` の位置を保持する。
- 指定時刻へ到達した命令をSpawnerへ送る。
- 発行した命令から返された `SpawnID` を追跡する。
- スポーン命令の発行完了と所属敵の終了からWave完了を判定する。
- キャンセル時に新しい命令の発行を停止する。

### 6.2 責務外

- 次に実行するWaveの選択。
- GameObjectやEntityの直接生成。
- GameObject、Entityおよび敵コンポーネントの所有。
- JSONファイルの読み書き。
- ゲームクリアへの状態遷移。

### 6.3 状態

```cpp
enum class WaveState
{
    Ready,
    Running,
    WaitingForEnemies,
    Completed,
    Cancelled
};
```

### 6.4 ランタイムデータ

```cpp
class Wave
{
public:
    Wave(
        const WaveDefinition& definition,
        IEnemySpawner& spawner,
        ISpawnPointResolver& spawnPointResolver);

    void Start();
    void Update(float deltaTime);
    void Cancel();

    void OnEnemyDestroyed(SpawnID id);

    bool IsCompleted() const;
    WaveState GetState() const;

private:
    const WaveDefinition* definition_ = nullptr;
    float elapsedTime_ = 0.0f;
    size_t nextCommandIndex_ = 0;
    std::unordered_set<SpawnID> aliveEnemies_;
    WaveState state_ = WaveState::Ready;
};
```

`WaveDefinition` は不変データとして参照し、実行状態は `Wave` 内にのみ保持する。

## 7. JSONデータ

### 7.1 WaveConfig

`WaveConfig` が `JsonEditableBase` を継承する。

```cpp
class WaveConfig : public JsonEditableBase
{
public:
    WaveConfig();

    const std::vector<WaveDefinition>& GetWavePool() const;
    size_t GetSelectionCount() const;

private:
    int selectionCount_ = 3;
    bool allowDuplicates_ = false;
    std::vector<WaveDefinition> wavePool_;
};
```

`WaveDefinition` と `SpawnCommand` には `to_json` / `from_json` を定義する。

現在の `JsonEditableBase` は任意型のJSON変換を利用できる一方、自動ImGui描画が対応する型は限定されている。このため、`std::vector<WaveDefinition>` の編集UIが必要になった場合は `WaveConfig::DrawImGui()` を専用実装する。エンジン共通の `JsonEditableBase` にゲーム固有型を直接追加しない。

### 7.2 WaveDefinition

```cpp
struct WaveDefinition
{
    std::string id;
    float selectionWeight = 1.0f;
    float startDelay = 0.0f;
    float nextWaveDelay = 0.0f;
    std::vector<SpawnCommand> commands;
};
```

### 7.3 SpawnCommand

初期実装では一つの `SpawnCommand` を一回のスポーンとして扱う。

```cpp
struct SpawnCommand
{
    float time = 0.0f;
    std::string enemyType;
    std::string spawnPoint;
};
```

`time` は直前の命令からの相対時間ではなく、Wave開始からの絶対時刻とする。命令の追加や削除によって後続タイミングが意図せず変化することを防ぐ。

同種の敵を連続生成する必要性が明確になった場合、次のプロパティを追加する。

```cpp
int count = 1;
float interval = 0.0f;
```

この拡張時には、Waveが命令内の未発行数と次回発行時刻を管理する必要がある。

### 7.4 JSON例

```json
{
    "selectionCount": 3,
    "allowDuplicates": false,
    "wavePool": [
        {
            "id": "cross_attack",
            "selectionWeight": 1.0,
            "startDelay": 1.0,
            "nextWaveDelay": 2.0,
            "commands": [
                {
                    "time": 0.0,
                    "enemyType": "Bomb",
                    "spawnPoint": "Left"
                },
                {
                    "time": 0.0,
                    "enemyType": "Bomb",
                    "spawnPoint": "Right"
                },
                {
                    "time": 2.5,
                    "enemyType": "Charge",
                    "spawnPoint": "Center"
                }
            ]
        }
    ]
}
```

## 8. ランダム選択

初期仕様では、設定された重みに基づく規定数の抽選を想定する。

- `selectionCount`: 1回のゲームで実行するWave数。
- `allowDuplicates`: 同じWaveを複数回選択できるか。
- `selectionWeight`: 各Waveの抽選重み。
- `seed`: 実行時に外部から渡し、選択結果を再現可能にする。

WaveSystem内へ抽選アルゴリズムを固定せず、必要になった段階で次のインターフェースへ分離できる構造にする。

```cpp
class IWaveSelector
{
public:
    virtual ~IWaveSelector() = default;

    virtual std::vector<size_t> Select(
        const WaveConfig& config,
        uint32_t seed) = 0;
};
```

デバッグログには最低限、乱数シードと選択されたWave IDの順序を出力する。

## 9. 敵スポーン境界

### 9.1 SpawnID

Waveは `GameObject*` や `EntityID` を直接保持せず、バックエンド非依存の `SpawnID` を保持する。

```cpp
using SpawnID = uint64_t;

struct SpawnResult
{
    SpawnID id;
    bool succeeded;
};
```

`SpawnID` は単調増加させ、ゲーム実行中に再利用しない。遅延した死亡通知が別の敵へ適用されることを防ぐ。

### 9.2 IEnemySpawner

```cpp
class IEnemySpawner
{
public:
    using DeathCallback = std::function<void(SpawnID)>;

    virtual ~IEnemySpawner() = default;

    virtual SpawnResult Spawn(
        const std::string& enemyType,
        const Vector3& position) = 0;

    virtual void SetDeathCallback(DeathCallback callback) = 0;
    virtual void Despawn(SpawnID id) = 0;
};
```

GameObject実装では以下の所有関係とする。

```text
Wave
  SpawnIDのみ保持

GameObjectEnemySpawner
  SpawnID -> GameObject* の対応表を保持
  GameObjectは所有しない

GameObjectManager
  CreateGameObject()で生成した動的GameObjectを所有
```

### 9.3 スポーン地点

JSONに直接座標を書くのではなく、基本的にはスポーン地点IDを指定する。

```cpp
class ISpawnPointResolver
{
public:
    virtual ~ISpawnPointResolver() = default;
    virtual std::optional<Vector3> Resolve(
        const std::string& spawnPointId) const = 0;
};
```

これにより、ステージ構造や座標管理をWave定義から分離できる。固定座標が必要になった場合は、SpawnCommandへ位置指定方式を追加して拡張する。

## 10. 敵の死亡と破棄

現状の `StatusComponent` はHPが0になると `isAlive_ = false` にするが、GameObjectの破棄や外部への死亡通知までは行わない。

Wave導入時には、次のライフサイクルを確立する。

```text
HPが0になる
  -> 死亡が一度だけ成立する
  -> GameObjectEnemySpawnerへSpawnID付き死亡通知
  -> WaveがaliveEnemies_からSpawnIDを削除する
  -> 敵の死亡演出を実行する
  -> 演出完了後にGameObject::Destroy()を呼ぶ
```

Wave上の撃破成立と、GameObjectの実際のメモリ破棄は別のタイミングになり得る。

死亡通知は一度だけ発行する。Waveが既に終了またはキャンセルされている場合や、通知されたIDが `aliveEnemies_` に存在しない場合は無視する。

将来ECSへ移行した場合は、HealthまたはDeath Systemから同じ論理通知を発行し、最終破棄を `Entity::Destroy()` とRegistryのGCへ置き換える。

## 11. Wave完了条件

初期仕様では、次の両方を満たしたときにWave完了とする。

```cpp
const bool commandsIssued =
    nextCommandIndex_ == definition_->commands.size();

const bool enemiesDefeated =
    aliveEnemies_.empty();

const bool completed =
    commandsIssued && enemiesDefeated;
```

ゲーム内の全Enemyを検索してはならない。固定配置の敵、別Waveの敵、召喚された敵などが完了条件へ混入するためである。

召喚された敵を元のWaveへ所属させるかどうかは、召喚仕様を追加するときに明示的に決定する。

## 12. GameSceneとの接続

```cpp
void GameScene::Initialize()
{
    waveConfig_.LoadJson("waves.json");

    waveSystem_.Initialize(
        &waveConfig_,
        &enemySpawner_,
        &spawnPointResolver_);

    StartState(SceneState::Playing);
}

void GameScene::OnEnterPlaying()
{
    waveSystem_.Start(seed_);
}

void GameScene::OnUpdatePlaying()
{
    waveSystem_.Update(deltaTime_);

    GameObjectManager::GetInstance()->Update();
    CollisionManager::GetInstance()->CheckCollisions();

    if (waveSystem_.IsCompleted())
    {
        ChangeState(SceneState::End);
    }
}
```

実際の更新順序は、スポーンした敵を同一フレームから更新するか、次フレームから更新するかを決めた上で固定する。

推奨する初期順序は以下。

```text
1. WaveSystem::Update()でスポーン命令を発行
2. GameObjectManager::Update()
3. CollisionManager::CheckCollisions()
4. 死亡通知を処理
5. Wave完了は次回のWaveSystem::Update()で確定
```

コールバック中に次Waveを即時開始せず、WaveSystemの次回Updateで状態遷移する。GameObject更新中の生成・破棄と進行遷移が重なることを避ける。

## 13. エラー処理

- JSONのWave IDが空の場合は設定エラーとする。
- Wave IDの重複を検出する。
- `selectionCount` が候補数を超え、重複選択が禁止されている場合は開始に失敗する。
- `SpawnCommand::time` が負の場合は設定エラーとする。
- commandは読込後に時刻順へ並べるか、順序違反をエラーにする。初期方針としては順序違反をログに出して安定ソートする。
- 未登録の `enemyType` はスポーン失敗としてログへ出す。
- 未定義の `spawnPoint` はスポーン失敗としてログへ出す。
- スポーン失敗した命令を生存敵数へ加えない。
- 一部のスポーン失敗でWave全体を停止するかは設定せず、初期実装では処理を継続する。

## 14. デバッグ表示

WaveSystemのデバッグUIでは、少なくとも次を確認可能にする。

- WaveSystemの状態。
- 使用中の乱数シード。
- 選択されたWave IDの順序。
- 現在のWave番号とWave ID。
- Wave経過時間。
- 次のSpawnCommandと発行予定時刻。
- 未発行Command数。
- Wave所属の生存敵数。
- Waveの強制開始、スキップ、リセット。

ランタイムのデバッグUIと `WaveConfig` のJSON編集UIは分離する。

## 15. 実装順序

1. `WaveDefinition`、`SpawnCommand`、`WaveConfig` とJSON変換を実装する。
2. Wave設定のバリデーションを実装する。
3. `SpawnID` と `IEnemySpawner` を定義する。
4. 既存の敵生成処理を `EnemyFactory` へ移す。
5. `GameObjectEnemySpawner` を実装する。
6. `StatusComponent` 周辺へ一度限りの死亡通知契約を追加する。
7. 単一Waveを実行する `Wave` を実装する。
8. ランダム選択と複数Wave進行を担う `WaveSystem` を実装する。
9. GameSceneへ接続する。
10. ランタイムデバッグUIを追加する。
11. 必要になった段階で `WaveConfig` 専用ImGui編集UIを追加する。

## 16. 未決事項

実装前に次を確定する。

1. Wave選択は重み付きか、等確率か。
2. 同じWaveの重複選択を許可するか。
3. 選択したWaveの実行順もランダムにするか。
4. Wave完了を所属敵の全滅とするか。
5. 敵の死亡成立時と死亡演出完了時のどちらでWave追跡から外すか。
6. 召喚された敵を召喚元Waveへ所属させるか。
7. Wave間待機中も残存する弾やエフェクトを維持するか。
8. スポーン命令を発行したフレームから敵を更新するか。
9. Pause中はWave時間を停止するか。基本方針はゲーム時間の `deltaTime` に従って停止する。
10. `WaveSystem::Stop()` 時にWave所属敵を破棄するか、そのまま残すか。

## 17. 初期決定案

未決事項を確定するまでの初期案は以下とする。

- Waveは重み付きで重複なし抽選する。
- 選択された順番で実行する。
- 全SpawnCommand発行済み、かつ所属敵全滅でWave完了とする。
- HPが0になり死亡が成立した時点でWave追跡から外す。
- Wave間待機は `WaveSystem` が管理する。
- `WaveSystem` はGameScene所有とし、シングルトンおよびECS Systemにはしない。
- `WaveConfig` のみ `JsonEditableBase` を継承する。
- WaveおよびWaveSystemはGameObjectやEntityを直接所有しない。
