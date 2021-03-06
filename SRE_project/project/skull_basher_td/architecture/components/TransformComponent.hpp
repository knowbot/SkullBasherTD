//
// Created by Morten Nobel Jørgensen on 2018-11-06.
//
#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <sre/Mesh.hpp>
#include "glm/glm.hpp"
#include <string>
#include <vector>
#include "Component.hpp"
#include "../scenes/GameObject.hpp"
#include "animation/AnimatorComponent.hpp"
#include "rendering/ModelRendererComponent.hpp"


class TransformComponent : public Component {
public:
    explicit TransformComponent(GameObject* gameObject);
    ~TransformComponent();

    std::string name;

    glm::vec3 position = glm::vec3(0,0,0);
    // euler rotation in degrees (default value 0,0,0)
    glm::vec3 rotation = glm::vec3(0,0,0);
    glm::vec3 scale = glm::vec3(1,1,1);

    glm::mat4 localRotation() const;
    glm::mat4 localTransform() const;

    glm::vec3 globalRotation();
    glm::vec3 globalPosition();
    glm::mat4 globalTransform();

    void debugGUI() override;

//    Transform *getParent() const;
//
//    void setParent(Transform *_parent);
//    const std::vector<Transform*> & getChildren();

    void lookAt(glm::vec3 at,glm::vec3 up);
    void lookAt(TransformComponent* at,glm::vec3 up);
};


