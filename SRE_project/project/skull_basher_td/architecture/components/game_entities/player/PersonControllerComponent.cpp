//
// Created by Morten Nobel-Jørgensen on 29/09/2017.
//

#include "PersonControllerComponent.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL

#include <iostream>

// sound includes
#include "../../../sound/SoundDevice.hpp"
#include "../../../sound/SourceManager.hpp"


// physics indludes
#include "../../physics/RigidBodyComponent.hpp"

// Arrow Collision Hanlder include
#include "../../collisions/ProjectileCollisionHandlerComponent.hpp"

// game manager include
#include "../../../managers/GameManager.hpp"

// expirable include (for arrow)
#include "../../lifespan/ProjectileLifespanComponent.hpp"

//using namespace sre;
using namespace glm;

PersonControllerComponent::PersonControllerComponent(GameObject* gameObject)
 : Component(gameObject)
{
    camera = gameObject->getComponent<CameraComponent>(); // sync the camera variable to the camera component

    // get rigid body 
    btRigidBody* myBody = gameObject->getComponent<RigidBodyComponent>()->getRigidBody();
    myBody->setAngularFactor({0,0,0}); // disable angular factor (otherwise player could tip over)
    myBody->setActivationState(DISABLE_DEACTIVATION); // disable deactivation

    camera_front = gameObject->getComponent<TransformComponent>()->rotation;  // set initial target of the camera on the negative Z axis (default)
    
    // set initial target of the camera on the negative Z axis (default)
    camera_dir = normalize(position - camera_front);       // sets the camera direction with a positive Z axis
    camera_right = normalize(cross(world_up, camera_dir)); // get a right vector perpendicular to the Y axis and the Z axis

    // start_footstep_lockout = std::chrono::steady_clock::now(); // initalise the first footstep lockout
    handModels.insert(std::make_pair("crossbow",  Model::create().withOBJ(".\\assets\\lowpoly_crossbow_2_5.obj").build()));
    handModels.insert(std::make_pair("hammer",  Model::create().withOBJ(".\\assets\\hammer.obj").build()));

    auto path = ".\\assets\\crossbow_bolt_2.obj";
    arrowModel = Model::create().withOBJ(path).withName("arrow").build();
}

PersonControllerComponent::~PersonControllerComponent() {
    for(auto& h : handModels)
        h.second.reset();
}

// debug gui info for Person Controller
void PersonControllerComponent::debugGUI() {
    ImGui::PushID(this);
    if (ImGui::TreeNode("PersonControllerComponent")) {
        ImGui::Checkbox("AllowedToBuild", &allowedToBuild);
        ImGui::TreePop();
    }
    ImGui::PopID();
}

void PersonControllerComponent::update(float deltaTime)
{
    updateVectors(); // updates the camera vectors, camera_right, camera_fwd, camera_dir
    updateInput(deltaTime); // Updates position and rotation based on inputs from mouse / keyboard
    // update camera_front using latest inputs from rotation (yaw) and pitch
    glm::vec3 direction;  
    direction.x = cos(glm::radians(rotation)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(rotation)) * cos(glm::radians(pitch));
    
    // normalise the camera front again
    camera_front = glm::normalize(direction);

// ------ update the 'hand'

    camera->moveHandCursor(camera_front, this->hand);
    if(GameManager::getInstance().buildModeActive)
    {
        camera->moveTowerCursor(camera_front, this->tower);
        camera->simpleRayCast(camera_front, this->tower, GameManager::getInstance().getSceneManager()->getCurrentScene()->getGameObjects());
    }
//------- end update the 'hand'    

// ----------------- Update the rigid body -------------------

    // Get the person controllers rigid body
    auto myBody = gameObject->getComponent<RigidBodyComponent>()->getRigidBody();
    // init a transform that will update the rigid body at the end
    btTransform xform;

    // init some quarternions
    btQuaternion aroundY; // for setting the rotation around y
    btQuaternion aroundZ; // for setting the rotation around z

    // vector (0, -1, 0) declares it is the y axis, -1 because otherwise rotation is inverted
    aroundY.setRotation(btVector3(0,-1,0), radians(rotation+90)); // rotate aroudn y (yaw)
    
    // vector (1,0,0) declares rotation in the z axis (z,y,x)
    aroundZ.setRotation(btVector3(1,0,0), radians(pitch)); // rotate around z (pitch)
    
    // multiply the two rotations to get the final quarternion rotation
    btQuaternion final = aroundY * aroundZ; 
    xform.setRotation(final); // set the transform's rotation to the new quarternion

    // sets the position of the transformation to match the players new position
    xform.setOrigin({position.x, position.y, position.z});

    // set the new transform in the motion state, for physics to use
    myBody->getMotionState()->setWorldTransform(xform);

    // can also just set the transform - but this way skips physics current frame processing
    // myBody->setWorldTransform(xform); //

    // set the center of mass transform as well as the world transform - bullet is weird
    myBody->setCenterOfMassTransform(xform);

// ------------- end Update the rigid body   

// set the Transform orientation
    this->getGameObject()->getComponent<TransformComponent>()->lookAt(position + camera_front, world_up);
    
//--------------- update Listener
    SoundDevice::Get()->SetLocation(position.x, position.y, position.z);
    SoundDevice::Get()->SetOrientation(camera_front.x, camera_front.y, camera_front.z,
                                        world_up.x, world_up.y, world_up.z);
// --------------- end update Listener


//-------------- reload the crossbow
    if(!shootable)
    {
        // update the reload timer
        kickOffTime_reload = std::chrono::steady_clock::now();
        int time_elapsed_milli = std::chrono::duration_cast<std::chrono::milliseconds>(kickOffTime_reload - start_reload_lockout).count();

        // TODO match offset to player colision size
        if (time_elapsed_milli > reload_lockout_millisec)
        {
            shootable = true;
            currentMovespeed = normalMovespeed;
            // restart reload lockout
            start_reload_lockout = std::chrono::steady_clock::now();
        }
    }
//-------------- end reloading the crossbow

}

// updats the camera vectors used in camera calculations
void PersonControllerComponent::updateVectors()
{
    camera_dir = normalize(position - camera_front); // sets the camera direction with a positive Z axis
    // camera_right = normalize(cross(world_up, camera_dir));

    camera_right = normalize(cross(world_up, camera_front));
    camera_fwd = normalize(cross(camera_right,world_up));
}

// update the transform (rotation and translation) of the PersonControllerComponent's Transform and RigidBodyComponent
void PersonControllerComponent::updateInput(float deltaTime)
{
    // updates the rotation, pitch and position variables of the player, based on inputs
    float velocity = currentMovespeed * deltaTime;
    
    
    // get the player's rigid body
    btRigidBody* hasRigidBody = gameObject->getComponent<RigidBodyComponent>()->getRigidBody();

    // init a btTransform for storing transform manipulations
    btTransform transform;

    // assign transform to rigid body's current world transform
    transform = hasRigidBody->getWorldTransform();

    // set a btVector to the transform's origin
    btVector3& origin = transform.getOrigin();
    position = {origin.x(), origin.y(), origin.z()}; // links origin and position
  
    // init some force to nothing - for use setting linear velocity later
    btVector3 force = {0,0,0};


    // update the rotation and pitch based on the mouse movements recorded
    // NB: "rotation" is yaw, could update variable name
    rotation += mouse_offset.x * deltaTime;
    pitch += mouse_offset.y*deltaTime;
   
    // keep rotation in 360 degree range
    if (rotation > 360.f)
        rotation -= 360.f;
    else if (rotation < -0.f)
        rotation += 360.f;
    mouse_offset.x = 0.f; // reset x offset

    // keep pitch in +- limit degree range (set in header)
    if (pitch > pitch_limit)
    {
        pitch = pitch_limit;
    } else if (pitch < -pitch_limit-5){
        pitch = -pitch_limit-5;
    }
    mouse_offset.y = 0.f; // reset y offset
    
    // std::cout << "Pitch is: " << pitch << std::endl;

    // update position based on current keypresses
    // using the camera_front vector allows to keep account of rotation automatically
    // NB, changed above to use camera_fwd, a cross between up and camera_right
    // using camera_fwd keeps movement in the x-z axes only

    // init old position, for calculating force later
    glm::vec3 oldPosition = position;

    if (key_fwd){

        // position += velocity * camera_front;
        position += velocity * camera_fwd;
    }

    if (key_bwd){

        // position -= velocity * camera_front;
        position -= velocity * camera_fwd;
    }
    if (key_left) {
        position -= velocity * cross(camera_front, world_up);
    }
    if (key_right){
        position += velocity * cross(camera_front, world_up);
    }

    // fly up and down removed for gameplay, could created a 'developer' mode and re-enable these 
    // if (key_flyUp){
    //     position += velocity * world_up; //works! fly's up
    // }
    // if (key_flyDown){
    //     position -= velocity * world_up; //flys down!
    // }

    // Calculate the force to apply to the character
    glm::vec3 positionDifference = oldPosition - position;
    
    // assign force to btVector3 (x-z plane only)
    force = {positionDifference.x, 0, positionDifference.z};
    
    // get the current total force (i.e current speed of the player)
    btScalar totalForce = hasRigidBody->getLinearVelocity().length();

    // if speed (total force) is less than 'some value', apply force (speed up)
    if(totalForce <= 7) {

        //kinda works, NB: have to set both origin (done later) and force for physics to move
        hasRigidBody->applyCentralImpulse(-force); 
    } 
    
    btVector3 currentVelocity = hasRigidBody->getLinearVelocity();

    // If force (i.e. walking) is not being applied, artificially change friction to slow down quickly
    if ((force.x() == 0 && force.z() == 0) 
    && (currentVelocity.x() != 0 && currentVelocity.y() != 0)
    && hasRigidBody->getFriction() != 5)
    {
        hasRigidBody->setFriction(5);
    } else if (hasRigidBody->getFriction() != 1) {
        // otherwise set a baselinie friction
        hasRigidBody->setFriction(1);
    }

    // if jump key is pressed, jump!
    if (key_jump && grounded == true)
    {
        if (currentVelocity.y() < 0.5) //helps a little
        {
            hasRigidBody->applyCentralImpulse({0,jumpHeight,0});
            grounded = false; // poor mans isGrounded
        }
        // TODO put in proper isGrounded check here, so there is no mid-air jumping
        
    }
    
}

void PersonControllerComponent::onKey(SDL_Event &event)
{
    switch (event.key.keysym.sym)
    {
    case SDLK_w:
        key_fwd = event.type == SDL_KEYDOWN;
        // std::cout << "forward \n";
        break;
    case SDLK_a:
        key_left = event.type == SDL_KEYDOWN;
        break;
    case SDLK_s:
        key_bwd = event.type == SDL_KEYDOWN;
        break;
    case SDLK_d:
        key_right = event.type == SDL_KEYDOWN;
        break;
    case SDLK_r:
        key_flyUp = event.type == SDL_KEYDOWN;
        break;
    case SDLK_f:
        key_flyDown = event.type == SDL_KEYDOWN;
        break;                
    case SDLK_SPACE:
        key_jump = event.type == SDL_KEYDOWN;
        break;
    case SDLK_ESCAPE:
        quit = event.type == SDL_KEYDOWN;
        break;
    default:
        break;
    }
}

void PersonControllerComponent::onMouse(SDL_Event &event)
{
    if (event.type == SDL_MOUSEMOTION)
    {
        // std::cout << "mouse event recorded \n";
        mouse_offset.x = event.motion.xrel * sensitivity;
        mouse_offset.y = -event.motion.yrel * sensitivity; //flipped direction
        /*if(currentScene->gameManager->buildModeActive)
            camera->simpleRayCast(camera_front, this->tower, currentScene->getGameObjects());*/
    }

//------------- fire projectile on left click
    if (shootable)
    {
        if(event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
        {           
            if (GameManager::getInstance().levelRunning && !GameManager::getInstance().paused)
            { // check the level is running and not pasued - don't shoot if paused
                if(!GameManager::getInstance().buildModeActive)
                { // check if build mode is active - don't shoot if building
                    shootable = false;

                    start_reload_lockout = std::chrono::steady_clock::now(); // init a reload lockout
                    // std::cout << "mouse clicked" << std::endl;
                    fireProjectile(); // shoot!

                    // play the reloading sound
                    SourceManager::Get()->playMyJam_global("crossbow_reload.wav");

                    // start the reload animation
                    if(hand->getComponent<AnimatorComponent>()->getAnimationState() != "reload")
                        hand->getComponent<AnimatorComponent>()->setAnimationState("reload");
                    currentMovespeed = slowMovespeed; // artificially slow down player when reloading. Running while reloading is just unrealistic ;-)
                }
            }
        }
    }
}

// not sure if used anymore
void PersonControllerComponent::setInitialPosition(glm::vec2 position, float rotation)
{
    this->position = glm::vec3(position.x, 0, position.y);
    this->rotation = rotation;
}

void PersonControllerComponent::fireProjectile(){
    // make the arrow game object
    auto arrow = gameObject->getScene()->createGameObject("arrow");
    arrow->getComponent<TransformComponent>()->position = hand->getComponent<TransformComponent>()->position; // set the arrow position to the crossbow
    // arrow->getComponent<Transform>()->position = position; // get the location to shoot from

    // set the direction for the arrow to travel in
    arrow->getComponent<TransformComponent>()->lookAt(arrow->getComponent<TransformComponent>()->position + camera_front, glm::vec3(0, 1, 0));

    // add a model and mesh to the arrow
    auto arrowMR = arrow->addComponent<ModelRendererComponent>();
    arrowMR->setModel(arrowModel);

    // add a rigid body to the arrow
    auto arrowBody = arrow->addComponent<RigidBodyComponent>();
    // set the arrow to collide with buildings and enemies
    // box appears to be the right size but could use bounds for a better fit
    arrowBody->initRigidBodyWithBox({0.01,0.01,0.5}, 0.1, PROJECTILES, BUILDINGS | ENEMIES);
    
    // get the body and set a few factors
    auto arrowRigidBody = arrow->getComponent<RigidBodyComponent>()->getRigidBody();
    arrowRigidBody->setGravity({0,-0.5,0}); // not 'realistic' gravity, but more fun

    // set the force vector, so the physics engine does the work
    btVector3 arrowForce = {camera_front.x, camera_front.y, camera_front.z};
    // arrowForce *= 12; // good setting for linear velocity
    arrowForce *= 2; // good for central impulse

    // arrowRigidBody->setLinearVelocity(arrowForce);
    arrowRigidBody->applyCentralImpulse(arrowForce);

    arrowRigidBody->setAngularVelocity({0,0,0}); // to make sure the arrow doesn't spin in the air
    // arrowRigidBody->setLinearFactor({0,0,0});

    //------ use these to slow arrow bouncing
    // arrowRigidBody->setRollingFriction(6);
    // arrowRigidBody->setFriction(3);
    // arrowRigidBody->setSpinningFriction(5);
    // arrowRigidBody->setAngularFactor(5);
    // -------- end arrow bouncing

    // add the collision handler for the arrow
    arrow->addComponent<ProjectileCollisionHandlerComponent>();

    // give the arrow a lifespan
    arrow->addComponent<ProjectileLifespanComponent>();

    // play the 'shooting' sound
    SourceManager::Get()->playMyJam_global("Bow.wav");
}


void PersonControllerComponent::updateHandModel(std::string model) {
    hand->getComponent<ModelRendererComponent>()->setModel(handModels[model]);
    hand->getComponent<AnimatorComponent>()->setAnimationState("none");
}

int PersonControllerComponent::getReloadLockoutMillisec() const {
    return reload_lockout_millisec;
}

const std::map<std::string, std::shared_ptr<Model>> &PersonControllerComponent::getHandModels() const {
    return handModels;
}

void PersonControllerComponent::setHandModels(const std::map<std::string, std::shared_ptr<Model>> &handModels) {
    PersonControllerComponent::handModels = handModels;
}

const bool PersonControllerComponent::isGrounded() const {
    return grounded;
}

void PersonControllerComponent::setIsGrounded(bool isGrounded) {
    PersonControllerComponent::grounded = isGrounded;
}

bool &PersonControllerComponent::getGrounded() {
    return grounded;
}
