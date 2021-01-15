
#include "ScheduleManager.hpp"
#include "../GameManager.hpp"
#include "Scene.hpp"
#include "GameObject.hpp"
#include "PathFinder.hpp"

class GameManager;

ScheduleManager::ScheduleManager(){
    startTime = std::chrono::steady_clock::now();
}

ScheduleManager::~ScheduleManager(){

}

void ScheduleManager::update(float deltaTime){
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
            auto gameObjectsList = currentScene->getGameObjects();
            int enemyToGoGoGo = currentScene->gameManager->getCurrentEnemy();
            int enemySetToGoGoGo = currentScene->gameManager->getCurrentEnemySet();
            int enemyWaveToGoGoGo = currentScene->gameManager->getCurrentWave();

            for (size_t i = 0; i < gameObjectsList.size(); i++)
            {
                auto tempGameObject = gameObjectsList[i];
                if (auto objectWithPathFinder = tempGameObject->getComponent<PathFinder>())
                { //get the GameObject with PathFinder
                    //get the wave number to compare with the next enemy to start moving
                    int currentEnemyWaveNumber = objectWithPathFinder->getWave();
                    int currentEnemyNumber = objectWithPathFinder->getEnemyNumber();
                    int currentEnemySetNumber = objectWithPathFinder->getEnemySetNumber();

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
                                objectWithPathFinder->setMovingStatus(true);

                                //make Game Manager Update the enemy and wave
                                currentScene->gameManager->updateAllWaveStats();
                                waveGoGoGo = false; //just did stuff, reset wave go go go to false to wait for next elapsed time
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


void ScheduleManager::fetchInitialWaveSchedule(){
        //set inital timeBetweenWaves
    waveScheduleDetails initialTimeBetweens = currentScene->gameManager->getCurrentTimeBetweenWaves();
    timeBetweenWaves = initialTimeBetweens.timeBetweenWaves;
    timeBetweenEnemies = initialTimeBetweens.timeBetweenEnemies;
}