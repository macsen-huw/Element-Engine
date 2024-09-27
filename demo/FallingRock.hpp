#pragma once

#include "element.hpp"
#include "GameObject.hpp"

class FallingRock :
    public GameObject
{
public:
    FallingRock();

    FallingRock(glm::vec3 pos);
};

