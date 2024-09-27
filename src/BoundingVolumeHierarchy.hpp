#pragma once

#include "RenderStructs.hpp"
#include "CollisionManager.hpp"
#include "Model.hpp"
#include "Mesh.hpp"
#include "glm.hpp"
#include <vector>

namespace elmt {

    class BVH;
    struct EntityData;

    struct AABB{
        AABB *c1;
        AABB *c2;
        glm::mat4x4 *transform = nullptr;
        BVH *bvh = nullptr; //the bvh the box belongs to
        glm::vec3 b1;
        glm::vec3 b2;
        size_t numTriangles;
        size_t triangleIndex = (size_t) - 1;
        uint32_t mortonCode;
    };

    struct AABB_Sort{
        bool operator() (const AABB &b1, const AABB &b2){
            return b1.mortonCode < b2.mortonCode;
        }
    };


    struct Triangle{
        glm::vec3 vertices[3];
        glm::vec3 centre;
        glm::vec3 normal;
        size_t meshIndex;
    };


    struct BuildTreeParameters{
        size_t arrStart = 0;
        size_t size = 0;
        BuildTreeParameters *leftHandPartition = nullptr;
        BuildTreeParameters *rightHandPartition = nullptr;
        BuildTreeParameters *returnAddress = nullptr;
    };


    class BVH {

        public:
            explicit BVH(std::vector<Mesh> &meshes);
            explicit BVH(Mesh &mesh);
            explicit BVH(std::vector<EntityData> &data);

            ~BVH();
            size_t maxClusterSize = 8;
            AABB root{};
            glm::mat4x4 transform = glm::mat4x4(1.f);
            Triangle getTriangle(AABB *box);

        private:
            uint32_t spreadBits(uint32_t x);
            void calculateModelBounds();

            void prepareBoundingBoxes();
            void initBoundingBox(size_t i);

            size_t numPrimitives;

            std::vector<Triangle> triangles;
            std::vector<AABB> boundingBoxes;
            std::vector<AABB*> nodes;
            std::vector<AABB> storage;

            std::vector<BuildTreeParameters> callStack;
            int64_t stackPointer = -1;

            std::vector<BuildTreeParameters> storageStackParameters;
            int64_t storageStackParametersPointer = -1;
            BuildTreeParameters *allocParameterStack();

            std::vector<AABB> storageStackAABB;
            int64_t storageStackAABBPointer = -1;
            AABB *allocAABBStack();


            void pushToStack(BuildTreeParameters param);
            void popFromStack();

            void initAAC();
            void buildTree();
            size_t calculateClusterSize(size_t currentSize);
            size_t makePartition(size_t start, size_t end);


            std::vector<AABB*> combineClusters(BuildTreeParameters *left, BuildTreeParameters *right, size_t maxSize);
            static float surfaceAreaHeuristic(AABB *c1, AABB *c2);

            static size_t calculateNodeSAH(std::vector<AABB*> &tempNodes, size_t size, size_t n, float *sah);
            AABB *mergeBoxes(AABB *c1, AABB *c2);
            static void calculateMergedBounds(AABB *c1, AABB *c2, glm::vec3 *b1, glm::vec3 *b2);

    };


}