//
// Created by Morten Nobel-Jørgensen on 29/09/2017.
//

#include <SDL_events.h>
#include "Camera.hpp"

#include "Component.hpp"
#include "Transform.hpp"
#include "GameObject.hpp"

class PersonController : public Component {
public:
    explicit PersonController(GameObject* GameObject);

    void update(float deltaTime);
    void updateInput(float deltaTime);
    void updateVectors();
    void onKey(SDL_Event &event);
    void onMouse(SDL_Event &event);

    void setInitialPosition(glm::vec2 position, float rotation);

    float rotation = 0.f;
    glm::vec3 position;
    bool quit = false;
    std::shared_ptr<Camera> camera;
    std::shared_ptr<GameObject> rayTestedCube;
private:

    glm::vec3 world_up = glm::vec3(0, 1, 0);
    glm::vec3 camera_front;
    glm::vec3 camera_dir;
    glm::vec3 camera_right;

    float movespeed = 8.f;
    float sensitivity = 1.7f;

    // input stuff
    bool key_fwd = false;
    bool key_bwd = false;
    bool key_left = false;
    bool key_right = false;

    //turn these off for the real game
    bool key_flyUp = false;
    bool key_flyDown = false;

    float mouse_offset = 0.f;
};