#pragma once
#include "Entity.hpp"
#include "../external/glm-0.9.7.1/glm/glm.hpp"

namespace elmt {
    class PointLight :
        public Entity
    {

        // Properties
    public:
        glm::vec3 colour;
        float intensity;


    // Methods
    public:
        PointLight(const char* name, Entity* parent, glm::vec3 pos, glm::vec3 colour, float intensity);    
    };
}

