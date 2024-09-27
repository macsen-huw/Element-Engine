#pragma once

#include "glm.hpp"
#include "Material.hpp"
#include "Mesh.hpp"
#include "Model.hpp"
#include "Entity.hpp"
#include "MeshRenderer.hpp"
#include "PhysicsComponent.hpp"

#include <vector>

namespace elmt {

    enum ObjectTypes{CUBE, SPHERE};
    class MeshRenderer;

    class DebugRenderObject {

        public:
            DebugRenderObject(glm::vec3 origin, glm::vec3 dimensions, glm::vec3 colour, ObjectTypes type);
            DebugRenderObject(glm::vec3 dimensions, glm::vec3 colour, ObjectTypes type);


            Model *model = nullptr;
            Entity *entity = nullptr;
            MeshRenderer *meshRenderer = nullptr;

            size_t instanceID = 0;

        private:
    };

}

