
#pragma once
#include "../Component.hpp"
#include "../../mixins/Updatable.hpp"

#include <chrono>

class GameObject;
/*
    ProjectileLifespanComponent is a simple component to help manage a gameObject lifespan
    Although not as ideal as a pool, it does help cut down the number of objects 
    needing updates (especially physics updates)
*/


class ProjectileLifespanComponent : public Component, public Updatable {
public:
    explicit ProjectileLifespanComponent(GameObject *gameObject);
    ~ProjectileLifespanComponent();

    void update(float deltaTime) override;

    virtual void setLifespan(int life_span); // sets the lifespan of the object

    int getLifespan();

private:
    std::chrono::steady_clock::time_point start_life; // time point lifespan starts
    int lifespan_millisec = 10000; // how long the obect will live in millisec, default is 10 sec
};
