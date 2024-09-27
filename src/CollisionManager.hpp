#pragma once

#include "BoundingVolumeHierarchy.hpp"
#include "IntersectionTests.hpp"
#include "PhysicsComponent.hpp"
#include "PhysicsWorld.hpp"
#include "Group.hpp"
#include <vector>
#include <glm.hpp>

namespace elmt {

    class BVH;
    class PhysicsComponent;
    struct ContactParams;

    struct EntityData{
        Entity *entity;
        PhysicsComponent *physicsComponent;
        BVH *bvh;
        glm::mat4x4 transform;
    };

    struct bvhTestPair{
        AABB *box1 = nullptr;
        AABB *box2 = nullptr;
    };

    struct CollisionInfo{
        glm::vec3 pointOfContact;
        glm::vec3 surfaceNormalObject1;
        glm::vec3 surfaceNormalObject2;
        EntityData *entityData1;
        EntityData *entityData2;
        size_t meshID1;
        size_t meshID2;
    };

    /*
    returned by getIntersections
    similar to CollisionInfo
    */
    struct IntersectionInfo {
        glm::vec3 pointOfContact = glm::vec3(0.0,0.0,0.0);
        // Collision normal of object being intersected
        glm::vec3 colliderNormal = glm::vec3(0.0,0.0,0.0);
        // Collision normal of object checking for intersection
        glm::vec3 checkerNormal = glm::vec3(0.0,0.0,0.0);
        size_t colliderMeshID = 0;
        size_t checkerMeshID = 0;
        // Entity being intersected with, nullptr if none
        Entity* entity = nullptr;

    };

    struct RayInfo{
        Entity *entity = nullptr;
        size_t modelID = 0;
        size_t meshID = 0;
        bool hit = false;
        glm::vec3 hitPos = glm::vec3(0.0, 0.0, 0.0);
    };


    // Used by IntersectionTestInfo to determine what to return intersections for
    enum class IntersectionTestType {
        // Return all intersections
        INTERSECT_ALL, 
        // Return intersections that are in one the groups specified
        INTERSECT_INC_GROUP,
        // Return intersections that are NOT in one the groups specified
        INTERSECT_EXC_GROUP
    };
    struct IntersectionTestInfo {

        IntersectionTestType testType = IntersectionTestType::INTERSECT_ALL;
        // Groups to include/exclude based on testType
        std::vector<Group*> groups = {};
        // If set, break and return after first intersection
        bool returnFirst = false;
    };

    class CollisionManager {

        public:

            CollisionManager() = default;
            ~CollisionManager();

            size_t addBVH(BVH *bvh, Entity *entity, PhysicsComponent *physicsComponent, glm::mat4x4 &transform, size_t meshID);
            void updateTransform(glm::mat4x4 newTransform, size_t ID);
            bool testCollisions();
            
            void refreshSceneBVH();

            EntityData* getEntityData(BVH *bvh);

            /*
            Convert a CollisionInfo (designed for more internal use) into IntersectionInfo
            */
            IntersectionInfo collisionToIntersectionInfo(PhysicsComponent *p, CollisionInfo& c);

            /*
            Get BVH from an object
            Accepts BVH, PhysicsComponent and Entity
            If a BVH is not found, nullptr is returned
            */
            BVH* getBVH(BVH* obj);
            BVH* getBVH(PhysicsComponent* obj);
            BVH* getBVH(Entity* obj);

            /*
            Get the BVH id of an object
            Accepts BVH, PhysicsComponent and Entity
            If a id is not found, -1 is returned
            */
            int getBVHID(BVH* obj);
            int getBVHID(PhysicsComponent* obj);
            int getBVHID(Entity* obj);


            /*
            Get a vector of all the entitiesd a given entity is colliding with
            If refreshSceneBVH is NOT set, then it is assumed the positions of entities have not been changed since the last refresh
            */
            std::vector<IntersectionInfo> getIntersections(PhysicsComponent* physComp, IntersectionTestInfo& testInfo, bool doRefreshSceneBVH);
            std::vector<IntersectionInfo> getIntersections(PhysicsComponent* physComp, IntersectionTestInfo& testInfo) { return getIntersections(physComp, testInfo, true); };
            std::vector<IntersectionInfo> getIntersections(Entity* entity, IntersectionTestInfo& testInfo, bool doRefreshSceneBVH);
            std::vector<IntersectionInfo> getIntersections(Entity* entity, IntersectionTestInfo& testInfo) { return getIntersections(entity, testInfo, true); };

            /*
            Check whether two things are intersecting
            */
            template <typename T1, typename T2>
            IntersectionInfo isIntersecting(T1* obj1, T2* obj2)
            {
                std::vector<CollisionInfo> collisions;
                IntersectionInfo res;

                BVH* bvh1 = getBVH(obj1);
                if (!bvh1) {
                    Logger::Print("Attempted to get BVH for object of type \"" + std::string(typeid(T1).name()) + "\" , but failed", LOGCAT::CAT_PHYSICS, LOGSEV::SEV_ERROR);
                    return res;
                }

                BVH* bvh2 = getBVH(obj2);
                if (!bvh2) {
                    Logger::Print("Attempted to get BVH for object of type \"" + std::string(typeid(T2).name()) + "\" , but failed", LOGCAT::CAT_PHYSICS, LOGSEV::SEV_ERROR);
                    return res;
                }

                testBox(bvh1, bvh2, collisions, true);

                if (!collisions.empty()) {
                    int BVHID = getBVHID(obj1);

                    if (BVHID == -1) {
                        Logger::Print("Attempted to get BVH ID for object of type \"" + std::string(typeid(T1).name()) + "\" , but failed", LOGCAT::CAT_PHYSICS, LOGSEV::SEV_ERROR);
                        return res;
                    }

                    PhysicsComponent *p = nullptr;
                    for (PhysicsComponent* physicsComponent : physicsComponents) {
                        if (physicsComponent->getBVHID() == BVHID) {
                            p = physicsComponent;
                        }
                    }

                    assert(p);

                    res = collisionToIntersectionInfo(p, collisions.back());
                }

                return res;
            }

            void handleCollisions();
            std::vector<CollisionInfo> lastCollisions;


            RayInfo rayCast(glm::vec3 origin, glm::vec3 direction);

            /*
            Add a component to the list of physics components
            Returns 0 on success, error code on failure
            */
            int addPhysicsComponent(PhysicsComponent* comp);
            /*
            Removes a component from the list of physics components
            Returns 0 on success, error code on failure
            */
            int removePhysicsComponent(PhysicsComponent* comp);

            /*
            Get a PhysicsComponent from a given BVH ID
            If no matching PhysicsComponent is found, nullptr is returned
            */
            PhysicsComponent* getComponentFromID(size_t bodyID);

            void resolveCollision(dGeomID o1, dGeomID o2);

            glm::vec3 simulateEntity(PhysicsComponent *physicsComponent, float time);
            bool simulateEntityStep(BVH* bvh, std::set<BVH*>& bvhs, std::vector<CollisionInfo>& stepCollisions);
            bool simulateEntityStep(BVH* bvh, BVH* bvhTest, std::vector<CollisionInfo>& stepCollisions);
        private:

            BVH *sceneBVH = nullptr;
            std::vector<bvhTestPair> testPairs;

            std::vector<EntityData> entityData;

            void testBox(BVH *bvh, std::vector<CollisionInfo>& collisions);
            void testBox(BVH* bvh1, BVH* bvh2, std::vector<CollisionInfo>& collisions, bool returnFirst = false);
            static bool comparePair(bvhTestPair &pair);
            size_t getID(BVH *bvh);

            std::vector<PhysicsComponent*> physicsComponents;
            void customCollision(PhysicsComponent *p1, PhysicsComponent *p2, glm::vec3 intersection, glm::vec3 normal, float depth);
            void setSurfaceParams(dContact &contact, ContactParams &body1Params, ContactParams &body2Params);

            static void staticNearCallback(void* data, dGeomID o1, dGeomID o2);
    };
}

