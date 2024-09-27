#pragma once
#include "Entity.hpp"
#include "../external/glm-0.9.7.1/glm/glm.hpp"

namespace elmt {
    class DirectionalLight :
        public Entity
    {

        // Properties
    public:
        glm::vec3 direction;
        glm::vec3 colour;
        float intensity;


        // Methods
    public:
        DirectionalLight(const char* name, Entity* parent, glm::vec3 direction, glm::vec3 colour, float intensity);
    };
}
