//
// Created by Morten Nobel Jørgensen on 2018-11-08.
//

#include "SceneManager.hpp"
#include "../components/game_entities/player/CameraComponent.hpp"
#include "../components/physics/RigidBodyComponent.hpp"
#include "../physics/GhostObject.hpp"

#include "../components/rendering/ModelRendererComponent.hpp"
#include "../components/lights/LightComponent.hpp"
#include "../physics/BulletPhysics.hpp"

//fps camera stuff
#include "../components/game_entities/player/PersonControllerComponent.hpp"

//WorldMap Imports
//WorldObjectComponent
#include "../components/game_entities/world/WorldObjectComponent.hpp"

// Sound imports
#include "../sound/SoundEffectsLibrary.hpp"
#include "../sound/SourceManager.hpp"
#include "../components/sound/PlaylistComponent.hpp"

//rapidjson imports
#include "../rapidjson/istreamwrapper.h"
#include "LevelGuiManager.hpp"

// Collision imports
#include "../components/collisions/EnemyCollisionHandlerComponent.hpp"
#include "../components/collisions/PlayerCollisionHandlerComponent.hpp"

#include "../components/game_entities/enemies/Pathfinder.hpp"
#include "../components/game_entities/coins/CoinComponent.hpp"
#include "../components/collisions/CoinCollisionHandlerComponent.hpp"
#include "../components/lifespan/ProjectileLifespanComponent.hpp"
#include "../music/MusicBuffer.hpp"

#include <iostream>
#include <utility>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCDFAInspection"

#include <Windows.h>



SceneManager::SceneManager()
{
    loadLevelsData();
    coinAnimation.setLooping(true);
    coinAnimation.addFrame(glm::vec3( 0), glm::vec3(1), glm::vec3(360), 3.f);
    coinModel = Model::create().withOBJ(".\\assets\\Coins.obj").withName("coin").build();
}

SceneManager::~SceneManager()= default;

std::shared_ptr<Scene> SceneManager::createScene(){
    auto res = std::make_shared<LevelScene>();
    auto cameraObj = res->createGameObject("Camera");
    cameraObj->addComponent<CameraComponent>()->clearColor = {0.2, 0.2, 0.2};
    cameraObj->getComponent<TransformComponent>()->position = playerSpawnPoint;
    cameraObj->getComponent<TransformComponent>()->rotation = {0,190,0};
    cameraObj->addComponent<RigidBodyComponent>()->initRigidBodyWithSphere(0.6f, 1, PLAYER, BUILDINGS | ENEMIES | CRYSTAL | COINS); // Dynamic physics object

    //--- end setting cameras

    auto person = cameraObj->addComponent<PersonControllerComponent>();
    cameraObj->addComponent<PlayerCollisionHandlerComponent>();

    auto lightObj = res->createGameObject("Light");
    lightObj->getComponent<TransformComponent>()->rotation = {30,30,0};
    lightObj->addComponent<LightComponent>();

    auto tower = res->createGameObject("Tower");
    tower->getComponent<TransformComponent>()->position = {0,0,0};
    tower->getComponent<TransformComponent>()->rotation = {0,0,0};
    tower->getComponent<TransformComponent>()->scale = {0.2f,0.2f,0.2f};
    auto towerMR = tower->addComponent<ModelRendererComponent>();
    towerMR->setMesh(sre::Mesh::create().withCube(0.99).build());
    cameraObj->getComponent<PersonControllerComponent>()->tower = tower;

    auto hand = res->createGameObject("Hand");
    hand->getComponent<TransformComponent>()->position = {0,0,0};
    hand->getComponent<TransformComponent>()->rotation = {0,0,0};
    hand->getComponent<TransformComponent>()->scale = {0.1f,0.1f,0.1f};
    auto handMR = hand->addComponent<ModelRendererComponent>();
    auto handAN = hand->addComponent<AnimatorComponent>();
    auto handReload = std::make_shared<Animation>(false);
    float resetAnimationTime = 0.2f;
    handReload->addFrame(glm::vec3( 0,-0.75,0), glm::vec3(1), glm::vec3(-15,0,0), (cameraObj->getComponent<PersonControllerComponent>()->getReloadLockoutMillisec() / 1000));
    handReload->addFrame(glm::vec3( 0,0,0), glm::vec3(1), glm::vec3(0,0,0), resetAnimationTime);
    handAN->addAnimation("reload", handReload);

    auto handBuild = std::make_shared<Animation>(false);
    handBuild->addFrame(glm::vec3( 0,0.5,0), glm::vec3(1), glm::vec3(30,0,0), 1.f);
    handBuild->addFrame(glm::vec3( 0,-0.5,0), glm::vec3(1), glm::vec3(-30,0,0), 1.f);
    handBuild->addFrame(glm::vec3( 0,0.5,0), glm::vec3(1), glm::vec3(30,0,0), 1.f);
    handBuild->addFrame(glm::vec3( 0,-0.5,0), glm::vec3(1), glm::vec3(-30,0,0), 1.f);
    handBuild->addFrame(glm::vec3( 0,0,0), glm::vec3(1), glm::vec3(0,0,0), 1.f);
    handAN->addAnimation("build", handBuild);

    //handMR->setMesh(sre::Mesh::create().withCube(0.99).build());
    handMR->setModel(person->getHandModels().find("crossbow")->second);

    cameraObj->getComponent<PersonControllerComponent>()->hand = hand;

    auto crystal = res->createGameObject("Crystal");
    auto crystalTR = crystal->getComponent<TransformComponent>();
    crystalTR->position = {3,-0.5,7};
    crystalTR->rotation = {0,0,0};
    crystalTR->scale = {0.4f,0.4f,0.4f};
    crystal->addComponent<CrystalHealthComponent>();
    crystal->getComponent<CrystalHealthComponent>()->setHealth(100);
    auto crystalMR = crystal->addComponent<ModelRendererComponent>();
    auto crystalPath =  ".\\assets\\crystal.obj";
    auto crystalAN = crystal->addComponent<AnimatorComponent>();
    auto crystalRigidBody = crystal->addComponent<RigidBodyComponent>();
    // ---- set crystal collision group and flags
    crystalRigidBody->initRigidBodyWithSphere(0.7f, 1, CRYSTAL, ENEMIES | PROJECTILES); // crystal needs to be sphere -> skull collision only works with box

    // ---- making sure that crystal can't move if hit
    crystalRigidBody->getRigidBody()->setAngularFactor(btVector3(0,0,0));
    crystalRigidBody->getRigidBody()->setLinearFactor(btVector3(0,0,0));
    crystalRigidBody->getRigidBody()->setActivationState(DISABLE_DEACTIVATION);

    GameManager::getInstance().crystal = crystal->getComponent<CrystalHealthComponent>();

    crystalMR->setModel(Model::create().withOBJ(crystalPath).withName("crystal").build());

    return res;
};

std::shared_ptr<Scene> SceneManager::createMainMenuScene(){
    auto res = std::make_shared<MainMenuScene>();
    auto cameraObj = res->createGameObject("Camera");
    cameraObj->addComponent<CameraComponent>()->clearColor = {0.2, 0.2, 0.2};
    cameraObj->getComponent<TransformComponent>()->position = {20,1.0f,11};
    cameraObj->getComponent<TransformComponent>()->rotation = {0,190,0};

    auto lightObj = res->createGameObject("Light");
    lightObj->getComponent<TransformComponent>()->rotation = {30,30,0};
    lightObj->addComponent<LightComponent>();

    return res;
};

void SceneManager::setCurrentScene(std::shared_ptr<Scene> res){
    this->currentScene = std::move(res);
};

std::shared_ptr<Scene> SceneManager::getCurrentScene(){
    return currentScene;
};

std::string SceneManager::getMapsFolderLoc(){
    return mapsFolderLoc;
};

void SceneManager::loadMap(const std::string& filename, std::shared_ptr<Scene> res){
    // std::cout << "loading map" << std::endl;
    loadLevelsMap(filename, res);
    
    // std::cout << "loading Enemies" << std::endl;
    loadLevelsEnemies(filename, res);
    
    // std::cout << "loading Sounds" << std::endl;
    loadLevelsSound(filename);
}

void SceneManager::changeScene(const std::shared_ptr<LevelData>& sceneData) {
    std::string path = ".\\maps\\";
    path.append(sceneData->fileName);

    GameManager::getInstance().init();

    if(sceneData->sceneType == 0)
    {
        auto scene = createScene();
        setCurrentScene(scene);
        getCurrentScene()->guiManager = std::make_shared<LevelGuiManager>();
        loadMap(path, getCurrentScene());

        auto scheduleManager = std::make_shared<ScheduleManager>();
        getCurrentScene()->scheduleManager = scheduleManager;

        GameManager::getInstance().setInitialWaveStats();
        scheduleManager->fetchInitialWaveSchedule();
        GameManager::getInstance().ToggleLockMouse();

        MusicBuffer * myMusicBuffer = MusicBuffer::Get();
        myMusicBuffer->Load(R"(.\assets\music\The Tread of War - MP3.wav)"); // Start playing a music track. First music track played should use "Load()"
    }
    else
    {
        auto scene = createMainMenuScene();
        setCurrentScene(scene);
        getCurrentScene()->guiManager = std::make_shared<MainMenuGuiManager>();
    }
}

const std::vector<std::shared_ptr<LevelData>> &SceneManager::getLevelsData() const {
    return levelsData;
}

void SceneManager::loadLevelsData() {
    std::string pattern(".\\maps\\*.json*");
    WIN32_FIND_DATA data;
    HANDLE hFind;
    // std::cout << "--Reading Level Files--" << std::endl;
    if ((hFind = FindFirstFile(pattern.c_str(), &data)) != INVALID_HANDLE_VALUE) {
        do {
            // std::cout << "Reading: " << data.cFileName << std::endl;
            using namespace rapidjson;
            std::string wholePath = ".\\maps\\";
            wholePath.append(data.cFileName);
            std::ifstream fis(wholePath);
            IStreamWrapper isw(fis);
            Document d;
            d.ParseStream(isw);

            auto levelId = d["levelId"].GetFloat(); //inner array is the number of positions in the current row
            auto levelName = d["levelName"].GetString(); //inner array is the number of positions in the current row

            auto levelDifficulty = d["levelDifficulty"].GetInt(); //inner array is the number of positions in the current row
            auto difficultyEnum = static_cast<DifficultyEnum>(levelDifficulty);

            auto sceneType = d["sceneType"].GetInt(); //inner array is the number of positions in the current row
            auto sceneTypeEnum = static_cast<SceneType>(sceneType);

            levelsData.push_back(std::make_shared<LevelData>(levelId,levelName, data.cFileName, difficultyEnum, sceneTypeEnum));
        } while (FindNextFile(hFind, &data) != 0);
        FindClose(hFind);
    }
}

float SceneManager::createScaledBounds(float boundSideZero, float boundSideOne, float scale, float factor) {
    return (fabs(boundSideZero + (scale * 2)) + fabs(boundSideOne + (scale * 2)))/factor;
}

// adds the levels sound effects from the level file
void SceneManager::loadLevelsSound(const std::string& filename) {
    using namespace rapidjson;
    std::ifstream fis(filename);
    IStreamWrapper isw(fis);
    Document d;
    d.ParseStream(isw);

    // ----------------- LOAD SOUNDS --------------------------
    int howManySounds = d["soundEffects"].GetArray().Size(); // how many sounds to load
    auto mySoundEffectsLibrary = SoundEffectsLibrary::Get(); // get the library - for adding sounds to

    for (size_t sound = 0; sound < howManySounds; sound++){
        //Go through the list of sounds and add them to the soundsEffectsLibrary
        std::string soundStr = d["soundEffects"][sound]["location"].GetString();
        const char *soundChar = soundStr.c_str();
        mySoundEffectsLibrary->Load(soundChar);
    }
}

void SceneManager::loadLevelsMap(const std::string& filename, std::shared_ptr<Scene> res) {
    using namespace rapidjson;
    std::ifstream fis(filename);
    IStreamWrapper isw(fis);
    Document d;
    d.ParseStream(isw);

    // std::cout << "loading player spawn" << std::endl;
// --------------- set player spawn point
    float spawnPointX = 0.f;
    float spawnPointY = 0.f;
    float spawnPointZ = 0.f;

    spawnPointX = d["playerSpawnPoint"].GetArray()[0].GetFloat();
    spawnPointY = d["playerSpawnPoint"].GetArray()[1].GetFloat();
    spawnPointZ = d["playerSpawnPoint"].GetArray()[2].GetFloat();
    playerSpawnPoint = {spawnPointX,spawnPointY,spawnPointZ};

//--------------- initialse players starting money
    GameManager::getInstance().setScore(d["startMoney"].GetFloat());

    playerSpawnRotation = d["playerSpawnRotation"].GetFloat();
    // std::cout << " spawnPointRotation should be: " << playerSpawnRotation << std::endl;

//---------------- put the player on the spawn point
    auto tempCam = currentScene->cameras[0]->getGameObject();
    tempCam->getComponent<TransformComponent>()->position = playerSpawnPoint;
    tempCam->getComponent<TransformComponent>()->rotation.y = playerSpawnRotation;

//----------- setup camera positioning
    glm::vec3 glmCameraPosition =  tempCam->getComponent<TransformComponent>()->position;
    btTransform transform;
    btVector3 btCameraPosition = {glmCameraPosition.x, glmCameraPosition.y, glmCameraPosition.z};
    transform.setOrigin(btCameraPosition);

    // std::cout << " spawnPointRotation is actually:  " << tempCam->getComponent<Transform>()->rotation.y << std::endl;
    glm::quat inputQuat = glm::quat(tempCam->getComponent<TransformComponent>()->rotation);
    btQuaternion btInputQuat = {inputQuat.x, -inputQuat.y, inputQuat.z, inputQuat.w,};
    transform.setRotation(btInputQuat);

    // set the rigid bodie's transform based on camera + player position
    tempCam->getComponent<RigidBodyComponent>()->getRigidBody()->setWorldTransform(transform);

// ------------------- end setting player Spawn point

// ------------------- load tiles
// std::cout << "loading tiles" << std::endl;
    //init a map row to temporarily hold the map row
    std::vector<int> mapRow;

    mapRow.clear(); //because mapRow is used in the original

    std::vector<glm::vec3> pathHolder;

    int rowArrayCount = d["tileMap"].GetArray().Size(); //get the tile map key, the array size is the outset loop's size
    int tileTypeInt;
    std::string tileTypeStr;
    float rotationHolder;
    glm::vec3 positionHolder;
    glm::vec3 scaleHolder;// scale
    glm::vec3 collisionHolder;

    bool isBuildableHolder = false;
    bool isPathHolder = false;

    std::string modelName;
    std::shared_ptr<Model> modelHolder;
    std::shared_ptr<sre::Mesh> meshHolder;
    std::shared_ptr<sre::Material> matHolder;
    std::vector<std::shared_ptr<sre::Material>> materialsLoaded;

    std::vector<glm::vec3> pathBuffer;
    bool reversePathBuffer = false; // some sections of the path need to be added to the path in reverse
    // std::cout << "starting tile iteration" << std::endl;

    for (size_t row = 0; row < rowArrayCount; row++) //go through each 'row' of the map
    {
        mapRow.clear(); //clear the temporary buffer
        int heightArrayCount = d["tileMap"][row].GetArray().Size(); //inner array is the number of positions in the current row
        pathBuffer.clear();
        // push each field's value into the buffer
        for (size_t heightFactor = 0; heightFactor < heightArrayCount; heightFactor++)
        {
            double tileHeight = tileHeightOffset + heightFactor;

            //variables defined here, attempting to save processing power
            int columnArrayCount = d["tileMap"][row].GetArray()[heightFactor].GetArray().Size();
            for (size_t column = 0; column < columnArrayCount; column++)
            {
                tileTypeInt = d["tileMap"].GetArray()[row].GetArray()[heightFactor].GetArray()[column].GetInt();

                if (tileTypeInt != 0)
                {
                    tileTypeStr = std::to_string(tileTypeInt);
                    const char *c = tileTypeStr.c_str();

                    //get position, scale and rotation of the block
//                    std::cout << "featching tile from lookup" << std::endl;
                    rotationHolder = d["MapLookup"][c]["rotation"].GetFloat();

                    scaleHolder.x = d["MapLookup"][c]["scaleFactors"]["x"].GetFloat();
                    scaleHolder.y = d["MapLookup"][c]["scaleFactors"]["y"].GetFloat();
                    scaleHolder.z = d["MapLookup"][c]["scaleFactors"]["z"].GetFloat();

                    positionHolder = glm::vec3((row * (scaleHolder.x * 2)) + tilePosOffset,tileHeight * (scaleHolder.y * 2),(column * (scaleHolder.z * 2))+ tilePosOffset);

//                    std::cout << "tile is buildable" << std::endl;
                    isBuildableHolder = d["MapLookup"][c]["isbuildable"].GetBool();
                    isPathHolder = d["MapLookup"][c]["isPath"].GetBool();

                    modelName = d["MapLookup"][c]["object"].GetString();

                    //Load the model from file
                    auto filePath = mapAssetFolderLoc + "\\" + modelName;
                    //auto filePath = ".\\assets\\skull.obj";
                    modelHolder = Model::create().withOBJ(filePath).withName(modelName).build();

                    //create game object
                    auto mapTile = res->createGameObject(modelName);
                    auto mapTileMR = mapTile->addComponent<ModelRendererComponent>();
                    mapTile->addComponent<WorldObjectComponent>();
                    mapTile->getComponent<WorldObjectComponent>()->setBuildable(isBuildableHolder);
                    mapTile->getComponent<WorldObjectComponent>()->setIsPath(isPathHolder);

                    // NEW
                    mapTileMR->setModel(modelHolder);

//                    std::cout << "tile position" << std::endl;
                    float xOffset = d["MapLookup"][c]["posOffset"]["x"].GetFloat();
                    float yOffset = d["MapLookup"][c]["posOffset"]["y"].GetFloat();
                    float zOffset = d["MapLookup"][c]["posOffset"]["z"].GetFloat();
                    
                    // offset the tiles position
                    positionHolder.x += xOffset;
                    positionHolder.y += yOffset;
                    positionHolder.z += zOffset;

                    // set the position, rotation and scale of the tile
                    mapTile->getComponent<TransformComponent>()->position = positionHolder;
                    mapTile->getComponent<TransformComponent>()->rotation.y = rotationHolder;
                    mapTile->getComponent<TransformComponent>()->scale = scaleHolder;
                    auto bounds = mapTileMR->getMesh()->getBoundsMinMax();
                    
                    // add rigid body to the tile
//                    std::cout << "tile collision" << std::endl;
                    collisionHolder.x = d["MapLookup"][c]["collision"]["x"].GetFloat();
                    collisionHolder.y = d["MapLookup"][c]["collision"]["y"].GetFloat();
                    collisionHolder.z = d["MapLookup"][c]["collision"]["z"].GetFloat();

                    float length = createScaledBounds(bounds[0].z, bounds[1].z, collisionHolder.z, 4);
                    float width = createScaledBounds(bounds[0].x, bounds[1].x, collisionHolder.x, 4);
                    float height = createScaledBounds(bounds[0].y, bounds[1].y, collisionHolder.y, 4);

                    mapTile->addComponent<RigidBodyComponent>()->initRigidBodyWithBox({length, height, width}, 0, BUILDINGS, PLAYER | PROJECTILES);

                    if (isPathHolder)
                    { // if the tile is part of the path, add it to the path buffer
                        pathBuffer.push_back(positionHolder);
                        //check if reverseBuffer should be set
                        reversePathBuffer = d["MapLookup"][c]["reversePathBuffer"].GetBool(); //a 'reverseBuffer' parameter tells the game that the path section needs to be loaded in reverse
                    }
                }
            }
        }

        //if the path is running from 'right to left' in the map, then it needs to be loaded into the path in reverse
        if (!reversePathBuffer && !pathBuffer.empty())
        {
            for (size_t pathInBuffer = 0; pathInBuffer <= pathBuffer.size()-1; pathInBuffer++)
            {
                pathHolder.push_back(pathBuffer[pathInBuffer]);
            }
        } else if (!pathBuffer.empty()) {
            for (size_t pathInBuffer = pathBuffer.size(); pathInBuffer > 0; pathInBuffer--)
            {
                pathHolder.push_back(pathBuffer[pathInBuffer-1]);
            }
        }
        reversePathBuffer = false;
    }

    // hand over the whole path to the Game Manager
    GameManager::getInstance().setPath(pathHolder);
    // ------------ end loading tiles

    // ------------ loading crystal
    // std::cout << "loading crystal" << std::endl;
    auto crystal = GameManager::getInstance().crystal->getGameObject();
    auto path = GameManager::getInstance().getPath();
    crystal->getComponent<TransformComponent>()->position = path[0];
}

void SceneManager::loadLevelsEnemies(const std::string& filename, std::shared_ptr<Scene> res) {
    using namespace rapidjson;
    std::ifstream fis(filename);
    IStreamWrapper isw(fis);
    Document d;
    d.ParseStream(isw);

    //-----------------Load enemies----------------------
    int howManyWaves = d["waves"].GetArray().Size();
    int enemyTypeInt;
    std::string enemyTypeStr;
    float rotationHolder;
    glm::vec3 positionHolder;
    glm::vec3 collisionHolder;
    glm::vec3 scaleHolder;// scale
    std::string modelName;
    std::shared_ptr<Model> modelHolder;
    std::shared_ptr<sre::Mesh> meshHolder;
    std::shared_ptr<sre::Material> matHolder;
    std::vector<std::shared_ptr<sre::Material>> materialsLoaded;
    auto floatingAnimation = std::make_shared<Animation>(true);
    floatingAnimation->addFrame(glm::vec3(0, 0.5, 0), glm::vec3(1), glm::vec3(0), .5f);
    floatingAnimation->addFrame(glm::vec3(0, 0, 0), glm::vec3(1), glm::vec3(0), .5f);
    floatingAnimation->addFrame(glm::vec3(0, -0.5, 0), glm::vec3(1), glm::vec3(0), .5f);
    floatingAnimation->addFrame(glm::vec3(0, 0, 0), glm::vec3(1), glm::vec3(0), .5f);

    for (size_t wave = 0; wave < howManyWaves; wave++) { //wave level
        //get the number of enemyTypes and quantities in order
        int numberOfEnemySets = d["waves"][wave]["enemies"].GetArray().Size();
        std::vector<EnemySetsInWave> enemySetsHolder;
        EnemySetsInWave tempEnemySet{};

        //wave schedule parameters per this current wave
        WaveScheduleDetails waveScheduleDetailHolder{};
        waveScheduleDetailHolder.timeBetweenWaves = d["waves"][wave]["timeBetweenWaves"].GetInt();
        waveScheduleDetailHolder.timeBetweenEnemies = d["waves"][wave]["timeBetweenEnemy"].GetInt();

        for (size_t currentEnemySet = 0; currentEnemySet < numberOfEnemySets; currentEnemySet++) { //inside set level
            enemyTypeInt = d["waves"][wave]["enemies"][currentEnemySet]["enemyType"].GetInt();
            enemyTypeStr = std::to_string(enemyTypeInt);
            const char *enemyTypeChar = enemyTypeStr.c_str();

            int howManyOfEnemyType = d["waves"][wave]["enemies"][currentEnemySet]["quantity"].GetInt();
            rotationHolder = d["enemyLookup"][enemyTypeChar]["rotation"].GetFloat();

            //put the set into the enemySetHolder to be sent to GameManager later
            tempEnemySet.enemyType = enemyTypeInt;
            tempEnemySet.quantity = howManyOfEnemyType;
            GameManager::getInstance().setTotalEnemies(GameManager::getInstance().getTotalEnemies() + howManyOfEnemyType);
            enemySetsHolder.push_back(tempEnemySet);


            std::vector<glm::vec3> path = GameManager::getInstance().getPath();
            int endOfPath = path.size();
            positionHolder = path[endOfPath - 1]; //sets where the enemy will spawn
            positionHolder.y += 1.0; //compensates for the fact that the path is well, the ground

            scaleHolder.x = d["enemyLookup"][enemyTypeChar]["scaleFactors"]["x"].GetFloat();// scale
            scaleHolder.y = d["enemyLookup"][enemyTypeChar]["scaleFactors"]["y"].GetFloat();// scale
            scaleHolder.z = d["enemyLookup"][enemyTypeChar]["scaleFactors"]["z"].GetFloat();// scale

            // get what model to use for the enemy
            modelName = d["enemyLookup"][enemyTypeChar]["object"].GetString();

            //Load the model from file
            auto filePath = enemiesAssetFolderLoc + "\\" + modelName;
            modelHolder = Model::create().withOBJ(filePath).withName(modelName).build();
            // std::cout << "Asset folder: " << filePath << "\n";
            // std::cout << "Model Name: " << modelName << "\n";

            // set the enemies movement speed
            float enemyMoveSpeed = d["enemyLookup"][enemyTypeChar]["moveSpeed"].GetFloat();

            // make the enemy
            for (int anEnemy = 0; anEnemy < howManyOfEnemyType; anEnemy++) {
                //create game object
                auto enemy = res->createGameObject(modelName);
                auto enemyTR = enemy->getComponent<TransformComponent>();
                auto enemyMR = enemy->addComponent<ModelRendererComponent>();
                auto enemyAN = enemy->addComponent<AnimatorComponent>();
                enemyMR->setModel(modelHolder);
                // set the enemies animation
//                enemyAN->addAnimation("floating", floatingAnimation);
//                enemyAN->setAnimationState("floating");

                // set the enemies transform
                enemyTR->position = positionHolder;
                enemyTR->rotation.y = rotationHolder;
                enemyTR->scale = scaleHolder;
                auto bounds = enemyMR->getMesh()->getBoundsMinMax();

                // set the enemies rigid body
                // std::cout << "adding collisions to enemy" << std::endl;
                collisionHolder.x = d["enemyLookup"][enemyTypeChar]["collision"]["x"].GetFloat();
                collisionHolder.y = d["enemyLookup"][enemyTypeChar]["collision"]["y"].GetFloat();
                collisionHolder.z = d["enemyLookup"][enemyTypeChar]["collision"]["z"].GetFloat();

                float length = createScaledBounds(bounds[0].z, bounds[1].z, collisionHolder.z, 7);
                float width = createScaledBounds(bounds[0].x, bounds[1].x, collisionHolder.x, 7);
                float height = createScaledBounds(bounds[0].y, bounds[1].y, collisionHolder.y, 7);

                auto enemyRB = enemy->addComponent<RigidBodyComponent>();
                enemyRB->initRigidBodyWithBox({length, width, height}, 1, ENEMIES, PLAYER | CRYSTAL | PROJECTILES);
                enemyRB->getRigidBody()->setGravity({0, 0, 0});
                // skulls should not be moved by external forces
                enemyRB->getRigidBody()->setLinearFactor({0,0,0});
                enemyRB->getRigidBody()->setAngularFactor({0,0,0});
                enemyRB->getRigidBody()->setLinearVelocity({0,0,0});
                enemyRB->getRigidBody()->setAngularVelocity({0,0,0});
                // skulls should never deactivate - this might be changed for improved performance
                enemyRB->getRigidBody()->setActivationState(DISABLE_DEACTIVATION);
                enemy->addComponent<EnemyCollisionHandlerComponent>();

                //  std::cout << "adding enemy component to enemy" << std::endl;
                //Add EnemyComponent to Skull
                auto enemyEC = enemy->addComponent<EnemyComponent>();
                enemyEC->setEnemyNumber(anEnemy);
                enemyEC->setWave(wave);
                enemyEC->setEnemySetNumber(currentEnemySet);
                enemyEC->getPathfinder()->setMoveSpeed(enemyMoveSpeed);
                enemyEC->setHealth(d["enemyLookup"][enemyTypeChar]["health"].GetFloat());
                enemyEC->setMoney(d["enemyLookup"][enemyTypeChar]["money"].GetFloat());
                enemyEC->setDamage(d["enemyLookup"][enemyTypeChar]["damage"].GetFloat());

                //--------- Add playlist to enemy
                //  std::cout << "adding playlist to enemy" << std::endl;
                auto enemyPlaylsitComponent = enemy->addComponent<PlaylistComponent>();
                
                int howManySoundEffects = d["enemyLookup"][enemyTypeChar]["soundEffectsPlaylist"].GetArray().Size();
                    // load the sound effect playist associated with this enemy type
                    for (size_t i = 0; i < howManySoundEffects;  i++)
                    {
                        std::string soundEffectCodeToSet = d["enemyLookup"][enemyTypeChar]["soundEffectsPlaylist"][i]["soundEffectCode"].GetString();
                        std::string soundEffectNameToSet = d["enemyLookup"][enemyTypeChar]["soundEffectsPlaylist"][i]["soundEffectName"].GetString();

                        enemyPlaylsitComponent->addSoundEffect(soundEffectCodeToSet, soundEffectNameToSet);
                        // std::cout << "just added a sound effect to the playlist" << std::endl;
                    }
                    

                //--------- end add playlist to enemy

                // log creation - could be removed for performance
                // std::cout << "created enemy with enemy number: " << anEnemy << std::endl;
                // std::cout << "created enemy with set number: " << currentEnemySet << std::endl;
                // std::cout << "created enemy with wave number: " << wave << std::endl;
            }
        }

        //send the wave details to the Game Manager
        GameManager::getInstance().addWave(wave, enemySetsHolder, waveScheduleDetailHolder);
    }
}

// spawns a coin object
void SceneManager::SpawnCoin(float money,glm::vec3 position) {
    auto coin = currentScene->createGameObject("Coin");
    auto coinTR = coin->getComponent<TransformComponent>();
    coinTR->position = position;
    coinTR->rotation = {0,0,0};
    coinTR->scale = {0.4f,0.4f,0.4f};
    auto coinMR = coin->addComponent<ModelRendererComponent>();
    coinMR->setModel(coinModel);
    //TODO: review animation

    auto coinAN = coin->addComponent<AnimatorComponent>();
    coinAN->addAnimation("rotate", std::make_shared<Animation>(coinAnimation));
    coinAN->setAnimationState("rotate");
    auto bounds = coinMR->getMesh()->getBoundsMinMax();

    float length = (fabs(bounds[0].z) + fabs(bounds[1].z));
    float width = (fabs(bounds[0].x) + fabs(bounds[1].x))/5;
    float height = (fabs(bounds[0].y) + fabs(bounds[1].y))/12;

    auto coinRigidBody = coin->addComponent<RigidBodyComponent>();
    coinRigidBody->initRigidBodyWithBox({length,width,height}, 1, COINS, PLAYER ); // crystal needs to be sphere -> skull collision only works with box

    // ---- making sure that Coin can't move if hit
    coinRigidBody->getRigidBody()->setAngularFactor(btVector3(0,0,0));
    coinRigidBody->getRigidBody()->setLinearFactor(btVector3(0,0,0));

    coin->addComponent<CoinComponent>();
    coin->getComponent<CoinComponent>()->setMoney(money);
    coin->addComponent<CoinCollisionHandlerComponent>();
    coin->addComponent<ProjectileLifespanComponent>();
    coin->getComponent<ProjectileLifespanComponent>()->setLifespan(20000);
}

#pragma clang diagnostic pop