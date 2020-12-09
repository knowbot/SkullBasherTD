//
// Created by Morten Nobel Jørgensen on 2018-11-06.
//

#include "GameObject.hpp"
#include "Transform.hpp"


GameObject::GameObject(std::string name_, Scene* scene_)
        :name(name_), scene(scene_)
{
    addComponent<Transform>();
}

GameObject::~GameObject() {
}

void GameObject::setName(const std::string &name) {
    GameObject::name = name;
}


std::string GameObject::getName() {
    return name;
}

const std::vector<CollisionHandler*>& GameObject::getCollisionHandlers(){
    return collisionHandlers;
}

bool GameObject::removeComponent(std::shared_ptr<Component> ptr) {
    for (auto iter = components.begin();iter != components.end(); iter++){
        if (*iter == ptr){
            auto ch = dynamic_cast<CollisionHandler*>(ptr.get());
            if (ch) {
                collisionHandlers.erase(std::find(collisionHandlers.begin(), collisionHandlers.end(), ch));
            }
            components.erase(iter);
            return true;
        }
    }
    return false;
}

const std::vector<std::shared_ptr<Component>> &GameObject::getComponents() {
    return components;
}

Scene *GameObject::getScene() {
    return scene;
}

GameObject *GameObject::getParent() const {
    return parent;
}

void GameObject::setParent(GameObject *parent) {
    if (GameObject::parent != nullptr){
        auto& parentChildren = GameObject::parent->children;
        auto pos = std::find(parentChildren.begin(), parentChildren.end(), this);
        if (pos != parentChildren.end()){
            parentChildren.erase(pos);
        }
    }
    GameObject::parent = parent;
    if (parent != nullptr){
        parent->children.push_back(this);
    }
}

const std::vector<GameObject *> &GameObject::getChildren() {
    return children;
}
