//
// Created by kb on 09/12/2020.
//
#pragma once
#include "Animation.hpp"
#include "../../scenes/GameObject.hpp"
#include "../../mixins/Updatable.hpp"
#include "../Component.hpp"

#include "glm/glm.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

struct TransformData {
    glm::vec3 position;
    glm::vec3 scale;
    glm::vec3 rotation;
};

class AnimatorComponent : public Component, public Updatable {
public:
    explicit AnimatorComponent(GameObject* gameObject);

    void update(float deltaTime) override;

    void addAnimation(std::string state, std::shared_ptr<Animation> animation);

    void setAnimationState(const std::string& state);
    const std::string& getAnimationState() const;

    std::shared_ptr<Animation> getAnimationForState(const std::string& state);

    glm::mat4 getSQTMatrix() const;

private:
    std::pair<std::string, std::shared_ptr<Animation>> currentAnimation;
    std::map<std::string, std::shared_ptr<Animation>> animations{}; // animations are given a name; can be later defined in an an animation json/script/whatever
    glm::mat4 SQTMatrix = glm::mat4(1.0f);

    TransformData startTransform{glm::vec3(0), glm::vec3(1), glm::vec3(0) };
    TransformData currentTransform{glm::vec3(0), glm::vec3(1), glm::vec3(0) };
    TransformData targetTransform{glm::vec3(0), glm::vec3(1), glm::vec3(0) };

    static glm::mat4 getQuaternionRotation(glm::vec3 rotation);
    void updateSQTMatrix();
    void resetVectors();

    static TransformData initTransformData(glm::vec3 position, glm::vec3 scale, glm::vec3 rotation);
};
