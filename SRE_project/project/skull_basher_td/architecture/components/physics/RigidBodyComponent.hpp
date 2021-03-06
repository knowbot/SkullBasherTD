//
// Created by Morten Nobel Jørgensen on 2018-11-10.
//

#include "../Component.hpp"
#include "btBulletDynamicsCommon.h"
#include "glm/glm.hpp"

/*
    rigid body sets up and adds a (bullet) rigid body to the dynamics world
*/

class TransformComponent;

// SB is an enum for setting the collision groups and masks
// both objects need to have the collision masks to collide with each other
// USE:
   // set group to group of objects this thing (the object) belongs to
    // mask to the bitwise (PLAYER | BUILDING | WALL)
    // constructor needs to take in which group and object to collide with
    // int group = 2;

    // eg if a player
    // int group = PLAYER;
    // int mask = BUILDINGS | ENEMIES | COINS ;

    // eg if a enemy
    // int group = ENEMY;
    // int mask = PLAYER | CRYSTAL | PROJECTILES ;

enum SBCollisions
{
    BUILDINGS = 64,
    ENEMIES = 128,
    COINS = 256,
    PLAYER = 512,
    CRYSTAL = 1024,
    PROJECTILES = 2048
};

class RigidBodyComponent : public Component {
public:
    explicit RigidBodyComponent(GameObject* gameObject);
    ~RigidBodyComponent();
    void initRigidBodyWithSphere(float radius, float mass = 0);
    void initRigidBodyWithSphere(float radius, float mass, short group, short mask);

    void initRigidBodyWithBox(glm::vec3 halfExtend, float mass = 1.0);
    void initRigidBodyWithBox(glm::vec3 halfExtend, float mass, short group, short mask);
    void initRigidBodyWithStaticPlane(glm::vec3 planeNormal, float planeDist);
    void initRigidBody(const btRigidBody::btRigidBodyConstructionInfo& info);
    void initRigidBody(const btRigidBody::btRigidBodyConstructionInfo& info, short group, short mask);
    btRigidBody* getRigidBody();

    void setLinearVelocityOnRigidBody(btVector3 linear_velocity);

    short getGroupID() const;

private:
    int collisionStartFrameId{};
    int collisionStartUpdateFrameId{};
    int collisionStartEndFrameId{};
    void updateTransformFromPhysicsWorld();
    btRigidBody* rigidBody = nullptr;
    TransformComponent* transform = nullptr;

    short group = -99;
    short mask = -99;

    btCollisionShape* shape = nullptr;
    btDefaultMotionState* fallMotionState = nullptr;

    friend class BulletPhysics;
    friend class Scene;
    friend class LevelScene;
};


