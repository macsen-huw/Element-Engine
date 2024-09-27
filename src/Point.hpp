#pragma once
#include "Entity.hpp"
#include "core.hpp"
#include <glm/common.hpp>

namespace elmt {

class Point :
    public Entity
{
    // Properties
private:
    friend class core;
    Point() {}; // Only used for serialisation

    // Methods
public:
    Point(const char* name, Entity* parent);
    Point(const char* name, Entity* parent, glm::vec3 pos);

};

}

