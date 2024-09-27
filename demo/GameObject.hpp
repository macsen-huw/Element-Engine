#pragma once

#include "element.hpp"
#include "globals.hpp"

#include <string>
#include <glm.hpp>

using namespace globals;

class GameObject : public elmt::Entity
{
private:
    static elmt::Model* loadModel(std::string modelName);

   


    static bool collideCallback(elmt::PhysicsComponent* p1, elmt::PhysicsComponent* p2, elmt::BodyContact& contactInfo);

    glm::vec3 startPos;

protected:
    elmt::PhysicsComponent* physComp = nullptr;
    elmt::MeshRenderer* meshComp = nullptr;

    glm::vec3 velocity{ 0.0,0.0,0.0 };

    bool hasPhysics;

    // Reset this object
    void Reset();

public:
    GameObject() {};

    GameObject(const char* name, elmt::Entity* parent, glm::vec3 pos, elmt::Model* model, bool hasPhysics);

    GameObject(const char* name, elmt::Entity* parent, glm::vec3 pos, std::string modelName, bool hasPhysics) : GameObject(name, parent, pos, loadModel(modelName), hasPhysics) {};
   
    
    GameObject(const char* name, glm::vec3 pos, std::string modelName, bool hasPhysics) : GameObject(name, g::world, pos, modelName, hasPhysics) {};

    GameObject(const char* name, glm::vec3 pos, elmt::Model* model, bool hasPhysics) : GameObject(name, g::world, pos, model, hasPhysics) {};

    bool hasGravity = true;

    bool getHasPhysics() { return hasPhysics; }

    elmt::PhysicsComponent* getPhysComp() { return physComp;  };

    static bool onClone(elmt::Entity*, elmt::Entity* c);

    virtual bool Update();
};

