
#include "../../scenes/GameObject.hpp"
#include "../physics/RigidBodyComponent.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"
#include "EnemyCollisionHandlerComponent.hpp"
#include "../health/CrystalHealthComponent.hpp"
#include "../../managers/GameManager.hpp"

class GameObject;



EnemyCollisionHandlerComponent::EnemyCollisionHandlerComponent(GameObject* gameObject) : Component(gameObject) {

}

void EnemyCollisionHandlerComponent::onCollision(size_t collisionId, GameObject* other, glm::vec3 position, bool begin) {
    //check the collision is the 'first time two obejcts collide' 
    if (begin && !stopCollisions){
        auto crystal = other->getComponent<CrystalHealthComponent>();
//        std::cout << "crystal " << crystal <<std::endl;
        if(crystal != nullptr)
        { // if other object is the crystal
            stopCollisions = true; // shouldn't handle more collisions with the crystal
            gameObject->setQueuedForDeletion(true); // skull should 'die' on collision with the crystal
            // std::cout << "hit crystal "<<std::endl;

            // decrease crystal health
            crystal->decreaseHealth(getGameObject()->getComponent<EnemyComponent>()->getDamage());
            
            // decrease the count of total enemies
            GameManager::getInstance().setTotalEnemies(GameManager::getInstance().getTotalEnemies() - 1);
            if(GameManager::getInstance().getTotalEnemies() <= 0)
            {
                GameManager::getInstance().toggleWinState(true);
            }
        }
    }
}

void EnemyCollisionHandlerComponent::onCollisionEnd(size_t collisionId, GameObject *other) {
}