
#include "ScheduleManager.hpp"
#include "GameManager.hpp"
#include "../scenes/GameObject.hpp"
#include "../components/game_entities/enemies/Pathfinder.hpp"

#include "../music/MusicBuffer.hpp"

ScheduleManager::ScheduleManager(){
    startTime = std::chrono::steady_clock::now();
    lastEnemy = false; // double cover for last enemy
}

ScheduleManager::~ScheduleManager(){

}

void ScheduleManager::update(float deltaTime){

    if (lastEnemy == false)
    {
        /* code */

        kickOffTime = std::chrono::steady_clock::now();
        elapsedTimeInSec = std::chrono::duration_cast<std::chrono::seconds>(kickOffTime - startTime).count();

        if (waveGoGoGo == false)
        {

            if (elapsedTimeInSec >= timeBetweenWaves)
            {
                waveGoGoGo = true;

            }


        } else if ( waveGoGoGo == true) {


            if ( elapsedTimeInSec >= goGoTimeSec)
            {
                goGoTimeSec = elapsedTimeInSec + timeBetweenEnemies;
                auto gameObjectsList = GameManager::getInstance().getSceneManager()->getCurrentScene()->getGameObjects();
                int enemyToGoGoGo = GameManager::getInstance().getCurrentEnemy();
                int enemySetToGoGoGo = GameManager::getInstance().getCurrentEnemySet();
                int enemyWaveToGoGoGo = GameManager::getInstance().getCurrentWave();



                for (size_t i = 0; i < gameObjectsList.size(); i++)
                {
                    auto tempGameObject = gameObjectsList[i];
                    if (auto enemy = tempGameObject->getComponent<EnemyComponent>())
                    { //get the GameObject with Pathfinder
                        //get the wave number to compare with the next enemy to start moving
                        int currentEnemyWaveNumber = enemy->getWave();
                        int currentEnemyNumber = enemy->getEnemyNumber();
                        int currentEnemySetNumber = enemy->getEnemySetNumber();
                        enemy->setHittable(true);

                        // std::cout << "current Enemy Set " << currentEnemySetNumber << std::endl;
                        // std::cout << "looking for: enemy:" << enemyToGoGoGo << ", set: " << enemySetToGoGoGo << " wave: " << enemyWaveToGoGoGo << std::endl;

                        if (currentEnemyWaveNumber == enemyWaveToGoGoGo)
                        {
                            if (currentEnemySetNumber == enemySetToGoGoGo)
                            {
                                if (currentEnemyNumber == enemyToGoGoGo)
                                {
                                    /* code */
                                    //object was the right one, make it go
                                    enemy->getPathfinder()->setMoving(true);
                                    GameManager::getInstance().setTotalEnemiesSpawned(GameManager::getInstance().getTotalEnemiesSpawned() + 1);

                                    //play sound
                                    // Moved to setMoving
                                    // SourceManager * mySourceManager = SourceManager::Get(); // apparently worked!
                                    // mySourceManager->playMyJam_global("pestilence.wav");
                                    // glm::vec3 testPosition(0,0,0);
                                    // float testSoundDist = 30.0f;
                                    // mySourceManager->playMyJam("pestilence.ogg", testPosition, testSoundDist);
                                    // mySourceManager->playMyJam("pestilence.wav", testPosition, testSoundDist);
                                    // mySourceManager->playSource((ALuint)1);

                                    // Only for testing the fade out, can be removed
                                    // MusicBuffer * myMusicBuffer = MusicBuffer::Get();
                                    // myMusicBuffer->changeTracks(R"(.\assets\music\The-Precipice-of-Victory-MP3.wav)");

                                    //make Game Manager Update the enemy and wave
                                    bool updateHappened = GameManager::getInstance().updateAllWaveStats();
                                    if (!updateHappened)
                                    {
                                        lastEnemy = true;
                                    }


                                    waveGoGoGo = false; //just did stuff, reset wave go go go to false to wait for next elapsed time

                                    fetchInitialWaveSchedule(); // TODO change name, also might not need to happen every trigger

                                    break;

                                }

                            }

                        }

                    }

                }
                waveGoGoGo = false; //just did stuff, reset wave go go go to false to wait for next elapsed time
            }

        }
    }

}


void ScheduleManager::fetchInitialWaveSchedule(){
    //set inital timeBetweenWaves
    WaveScheduleDetails initialTimeBetweens = GameManager::getInstance().getCurrentTimeBetweenWaves();
    timeBetweenWaves = initialTimeBetweens.timeBetweenWaves;
    timeBetweenEnemies = initialTimeBetweens.timeBetweenEnemies;
}