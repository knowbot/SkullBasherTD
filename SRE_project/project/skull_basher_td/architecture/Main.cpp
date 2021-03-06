#include <iostream>
#include "sre/SDLRenderer.hpp"
#include "scenes/Scene.hpp"
#include "managers/SceneManager.hpp"
#include "scenes/GameObject.hpp"
#include "components/game_entities/player/CameraComponent.hpp"
#include "components/lights/LightComponent.hpp"

#include "components/physics/RigidBodyComponent.hpp"

#include "components/game_entities/player/PersonControllerComponent.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "./physics/BulletPhysics.hpp"
#include "Main.hpp"


//sound Device
#include "./sound/SoundDevice.hpp" //i.e a device that is the "Listener"
#include "./music/MusicBuffer.hpp"

#include "./sound/SourceManager.hpp"

Main::Main() {
    using namespace sre;
    SDLRenderer r;
    r.init();

    //setup sound

    // myNewSpeaker = mySpeaker;
    //--------- setup sound
    SoundDevice * mySoundDevice = SoundDevice::Get(); // Initialise sound device
    SourceManager * mySourceManager = SourceManager::Get(); // Initialise Source manager

    //new way - MusicBuffer is now a singleton
    MusicBuffer * myMusicBuffer = MusicBuffer::Get(); // Initialise music buffer 
    myMusicBuffer->Load(R"(.\assets\music\The Tread of War - MP3.wav)"); // Start playing a music track. First music track played should use "Load()"

    auto fonts = ImGui::GetIO().Fonts;
    fonts->AddFontDefault();
    //-------------- handshaking
    GameManager::getInstance().init();
    for (const auto& levelData : GameManager::getInstance().getSceneManager()->getLevelsData()) {
        if(levelData->sceneType != 1)
            continue;

        GameManager::getInstance().getSceneManager()->changeScene(levelData);
        break;
    }

//--------------- Start Playing music - lives here because the buffers run out before things load
    myMusicBuffer->Play();

// --------- start update cycles
    r.frameUpdate = [&](float deltaTime){
        GameManager::getInstance().getSceneManager()->getCurrentScene()->update(deltaTime);

        mySourceManager->CheckAndReleaseOALSource(); // Checks for any sources that are finished playing their sound, and releases the source

        //Update Music buffer (keep playing music - stream in the next few bytes of sound)
        myMusicBuffer->UpdateBufferStream(); // Updates the music buffer so music keeps playing. Note: doesn't need to be called every frame, so we could optimise here
    };
    r.frameRender = [&]{
        GameManager::getInstance().getSceneManager()->getCurrentScene()->render();
    };
    r.keyEvent = [&](SDL_Event &e) {
        GameManager::getInstance().getSceneManager()->getCurrentScene()->onKey(e); //worked! // asks scene to manage the onKey
    };
    r.mouseEvent = [&](SDL_Event &e) {
        GameManager::getInstance().getSceneManager()->getCurrentScene()->onMouse(e); // asks scene to manage the mouse thing
    };
    r.startEventLoop();
}

const sre::SDLRenderer &Main::getR() const {
    return r;
}

int main(){
    new Main();
    return 0;
}