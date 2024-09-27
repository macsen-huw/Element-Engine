#pragma once
#include "GameObject.hpp"
#include "element.hpp"

class Collectable :
    public GameObject
{
public:
    Collectable(glm::vec3 pos, elmt::Model* model);

    virtual bool Update();
    // Called when intersected with player
    virtual void Collect();
};

