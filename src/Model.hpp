#pragma once
#include "core.hpp"

#include "../external/glm-0.9.7.1/glm/glm.hpp"
#include <vector>
#include <iostream>
#include "RenderManager.hpp"
#include "RenderStructs.hpp"
#include "Mesh.hpp"
#include "Material.hpp"
#include "BoundingVolumeHierarchy.hpp"
#include "SkeletonRig.hpp"

namespace elmt {

    class BVH;
    class core;
    class RenderManager;
    struct BlinnPhongMaterial;
    class CollisionManager;


    /*
	* A loaded model file for use in engine
	* NOT an Entity. To put this model in a game use ModelInstance
	*/
	class Model {
        // Properties
        public:

            std::string getMeshName(size_t meshID);
            std::vector<Material> materials;
            std::vector<Mesh> meshes;

            SkeletonRig skeletonRig;

            size_t modelID = -1;
            RenderManager *renderManager = nullptr;
            BVH *bvh = nullptr;
            bool mirror = false;

        // Methods
        public:
            Model(std::vector<Mesh> &meshes, std::vector<Material> &materials, bool mirror = false);

            Model(std::vector<Mesh>& meshes, SkeletonRig skeletonRig, std::vector<Material>& materials, bool mirror = false);
            
            Model() = default;
            
            bool isValid() const {
                return this->modelID != -1;
            };

            void UpdateMaterials(std::vector<Material> &newMaterials); // TODO: implement this
            
            friend std::ostream &operator<<(std::ostream &os, const Model &m);
            
            ~Model();

            std::vector<int> getFaces();
            std::vector<float> getVertices();

            std::vector<float> meshVertices;
            std::vector<int> meshFaces;


        private:
            void sortByMaterials();
            struct IndexPair{
                uint32_t materialIndex;
                uint32_t vectorIndex;
            };
            void storeModel();
    };
}


