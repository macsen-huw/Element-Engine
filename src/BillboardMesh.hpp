#pragma once


#include "../external/glm-0.9.7.1/glm/glm.hpp"
#include <vector>
#include <iostream>
#include "RenderManager.hpp"
#include "RenderStructs.hpp"

namespace elmt
{
    class RenderManager;
    struct BlinnPhongMaterial;

    class BillboardMesh
    {
    public:
        std::vector<float> attributes;
        unsigned int vCount; // number of vertices

        size_t billboardID;
        RenderManager* renderManager;

    public:
        BillboardMesh(float width, float height, RenderManager* renderManager);
        ~BillboardMesh();

    private:
        void sortByMaterials();
        struct IndexPair {
            uint32_t materialIndex;
            uint32_t vectorIndex;
        };
    };
}


