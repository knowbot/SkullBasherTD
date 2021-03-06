#pragma once

#include "sre/SDLRenderer.hpp"
#include "sre/Material.hpp"
#include "../data_holders/Tower.hpp"
#include "../components/health/CrystalHealthComponent.hpp"
#include "SceneManager.hpp"


struct EnemySetsInWave {
    int enemyType;
    int quantity;
};

struct WaveScheduleDetails {
    int timeBetweenWaves;
    int timeBetweenEnemies;
};

// struct somethingmeaningful { float meaningful1; float meaningful2; };
// using mymeaningfulmap = std::map<int, somethingmeaninful>;
// from https://stackoverflow.com/questions/24882357/stdpair-vs-array

class GameManager
{
public:
    static GameManager& getInstance()
    {
        static GameManager instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }

    GameManager(GameManager const&)               = delete;
    void operator=(GameManager const&)  = delete;

    void init();

    std::vector<std::shared_ptr<Tower>> GetTowers();
    std::shared_ptr<Tower> selectedTower;
    void onKey(SDL_Event &event);
    void onMouse(SDL_Event &event);
    void TogglePause();
    void ToggleLockMouse() const;
    bool quit = false;
    bool buildModeActive = false;

    int getScore() const;

    void setScore(int score);
    void addScore(int score);

//    float getPower() const;
//
//    void setPower(float power);

    bool levelRunning = true;
    bool won = false;
    bool paused = false;

    void updateTowerIndicator();

    void setPath(std::vector<glm::vec3> pathToBe);
    std::vector<glm::vec3> getPath();
    int getFirstPathIndex();
    glm::vec3 getPathPoint(int index);

    std::shared_ptr<CrystalHealthComponent> crystal;

    //wave details
    int getCurrentEnemy();
    int getCurrentEnemySet();
    int getCurrentWave();
    int getTotalWavesInLevel();

    int getEnemyAmountWave();
    WaveScheduleDetails getCurrentTimeBetweenWaves();

//    void setCurrentEnemy(int currentEnemy);
    void addWave(int waveNumber, std::vector<EnemySetsInWave> enemySets, WaveScheduleDetails waveDetails);

    bool updateAllWaveStats();
    void setInitialWaveStats();
    const std::map<int, std::vector<EnemySetsInWave>> &getWaveAndEnemies() const;
    int getTotalEnemiesInCurrentSet() const;
    void toggleWinState(bool winState);
    const std::unique_ptr<SceneManager> &getSceneManager() const;

    int getTotalEnemiesSpawned() const;
    void setTotalEnemiesSpawned(int totalEnemiesSpawned);
    int getTotalEnemies() const;
    void setTotalEnemies(int totalEnemies);
private:
    GameManager() {}

    std::unique_ptr<SceneManager> sceneManager;
    std::vector<std::shared_ptr<Tower>> towers;

    // Player stats
    int score = 40;

    //path
    std::vector<glm::vec3> path;

//-------------- Wave stats -------------------
//    const std::vector<std::shared_ptr<GameObject>> &getEnemies() const;
    int currentWave = 0;
    int totalWavesInLevel = 0;

    int enemySetsAmount = 0; //assuming this means how many waves

    int currentEnemySet = 0;
    int totalEnemySetsInCurrentWave = 0;
    // int currentEnemyInset = 0;

    int currentEnemy = 0;
    int totalEnemiesInCurrentSet = 0;

    int enemyAmountWave = 0;

    int totalEnemiesSpawned = 0;
    int totalEnemies = 0;

    std::map <int, std::vector<EnemySetsInWave>> waveAndEnemies;
    std::map <int, WaveScheduleDetails> waveAndTimeBetweens;
    bool lastEnemy = false;

    void loadTowers(std::string filename);
    void checkAndUpdateEnemyNumber();
    void checkAndUpdateEnemySetNumber();
    void checkAndUpdateWaveNumber(int tempCurrentEnemyWaveHolder);
    void setTotalEnemiesInCurrentSet(); // made this into a method to maintain consitent code

};
