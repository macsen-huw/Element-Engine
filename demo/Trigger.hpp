#pragma once

#include "GameObject.hpp"


// Forward declaration
class Trigger;

typedef void (*TriggerFunction)(elmt::Entity*, Trigger*);


class Trigger :
    public elmt::Entity
{
private:
    std::map<std::string, TriggerFunction> callbacks;
    elmt::AABB bounds;

    elmt::PhysicsComponent* physComp = nullptr;
    
    std::vector<elmt::Entity*> intersectingObjects = {};

public:
    Trigger(glm::vec3 pos, glm::vec3 dim, std::map<std::string, TriggerFunction> callbacks);
    ~Trigger();

    virtual bool Update();


};

