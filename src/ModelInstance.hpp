#pragma once
#include "Entity.hpp"
#include "Model.hpp"
#include "RenderManager.hpp"
#include "PhysicsComponent.hpp"
#include "../external/glm-0.9.7.1/glm/glm.hpp"

namespace elmt {

class ModelInstance;

class ModelInstance :
    public Entity
    {
        // Properties
    private:
        Model* model;
        size_t instanceID;

    public:
        InstanceProperties properties;

        // Methods
    public:
        ModelInstance(const char* name, Entity* parent, glm::vec3 pos, Model* model);
        void updateInfo();
        bool Render();
    
    };

}

