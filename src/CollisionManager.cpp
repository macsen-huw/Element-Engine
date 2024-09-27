#include "CollisionManager.hpp"
#include "Logger.hpp"
#include "LogType.hpp"
#include "string"

#include "core.hpp"

#define NDEBUG
using namespace elmt;

/*
 * adds a bvh to the manager
 * returns the new bvh's ID
 * this ID can be used to identify an object involved in a collision
 */
size_t CollisionManager::addBVH(BVH *bvh, Entity *entity, PhysicsComponent *physicsComponent, glm::mat4x4 &transform, size_t meshID) {

    EntityData data{};
    data.entity = entity;
    data.physicsComponent = physicsComponent;
    data.bvh = bvh;
    data.transform = transform;
    entityData.push_back(data);

    return entityData.size() - 1;
}

/*
 * updates the transform associated with a bvh, referenced by its ID
 */
void CollisionManager::updateTransform(glm::mat4x4 newTransform, size_t ID) {
    if (ID >= entityData.size()){
        Logger::Print("Error - attempting to update the transform of an invalid ID in the collision manager (ID = " +  std::to_string(ID) + ")\n",
                      LOGCAT::CAT_PHYSICS, LOGSEV::SEV_WARNING);
        return;
    }

    entityData[ID].transform = newTransform;
    entityData[ID].bvh->transform = newTransform;
}


/*
 * tests the scene for collisions
 * returns true if there were collisions, otherwise returns false
 * info about any collisions can be accessed in the collisions vector of the class
 */
bool CollisionManager::testCollisions() {

    if (entityData.size() <= 1) return false; //we need at least 2 objects for there to be any collisions

    delete sceneBVH;

    lastCollisions.clear();
    sceneBVH = new BVH(entityData);

    for (auto &entity : entityData){
        testBox(entity.bvh, lastCollisions);
    }

    return !lastCollisions.empty();
}

void CollisionManager::refreshSceneBVH() {
    delete sceneBVH;
    for (auto &data : entityData){
        glm::mat4x4 trans(1.f);
        trans[3] = glm::vec4(data.physicsComponent->getPosition(), 1);
        data.bvh->transform = trans * data.physicsComponent->getRotationMatrix();
    }
    sceneBVH = new BVH(entityData);
}

IntersectionInfo CollisionManager::collisionToIntersectionInfo(PhysicsComponent *p, CollisionInfo& c) {
    IntersectionInfo intersectionInfo;
    intersectionInfo.pointOfContact = c.pointOfContact;

    Entity* ent1 = c.entityData1->entity;
    Entity* ent2 = c.entityData2->entity;
    
    Entity* intersectedEntity;
    if (c.entityData1->physicsComponent == p) {
        intersectedEntity = ent2;

        intersectionInfo.checkerNormal = c.surfaceNormalObject1;
        intersectionInfo.colliderNormal = c.surfaceNormalObject2;
        intersectionInfo.checkerMeshID = c.meshID1;
        intersectionInfo.colliderMeshID = c.meshID2;
    }
    else if (c.entityData2->physicsComponent == p) {
        intersectedEntity = ent1;

        intersectionInfo.checkerNormal = c.surfaceNormalObject2;
        intersectionInfo.colliderNormal = c.surfaceNormalObject1;
        intersectionInfo.checkerMeshID = c.meshID2;
        intersectionInfo.colliderMeshID = c.meshID1;
    }
    else {
        intersectedEntity = nullptr;
    }
    intersectionInfo.entity = intersectedEntity;
    return intersectionInfo;
}

BVH* CollisionManager::getBVH(BVH* obj){
    return obj;
}


BVH* CollisionManager::getBVH(PhysicsComponent* obj){
    return entityData[obj->getBVHID()].bvh;
}



BVH* CollisionManager::getBVH(Entity* obj){
    /* 
    auto entityIt = std::find(entities.begin(), entities.end(), obj);
    if (entityIt == entities.end()) {
        return nullptr;
    }
    size_t entityIndex = std::distance(entities.begin(), entityIt);
    size_t bvhIndex = bvhIDs[entityIndex];
    */

    PhysicsComponent* physComp = (PhysicsComponent*)(obj->getComponentOfType("PhysicsComponent"));

    if (!physComp) return nullptr;

    return getBVH(physComp);
}

int CollisionManager::getBVHID(PhysicsComponent* obj)
{
    return (int)(obj->getBVHID());
}


int CollisionManager::getBVHID(BVH* obj){

    for (int i = 0; i < entityData.size(); i++){
        if (obj == entityData[i].bvh) return i;
    }

    Logger::Print("Error - attempting to get the index of a bvh that is not registered with CollisionManager\n",
                  LOGCAT::CAT_PHYSICS, LOGSEV::SEV_WARNING);

    return -1;
}


int CollisionManager::getBVHID(Entity* obj)
{
    for (int i = 0; i < entityData.size(); i++) {
        if (obj == entityData[i].entity) return i;
    }

    Logger::Print("Error - attempting to get the index of a bvh that is not registered with CollisionManager\n",
        LOGCAT::CAT_PHYSICS, LOGSEV::SEV_WARNING);

    return -1;
}


std::vector<IntersectionInfo> CollisionManager::getIntersections(PhysicsComponent* physComp, IntersectionTestInfo& testInfo, bool doRefreshSceneBVH)
{
    std::vector<IntersectionInfo> intersections = {};
    std::set<Entity*> intersectedEntities = {};

    BVH* bvh = getBVH(physComp);

    std::vector<CollisionInfo> collisionsInfo = {};

    if (doRefreshSceneBVH || !sceneBVH) {
        refreshSceneBVH();
    }
    
    testBox(bvh, collisionsInfo);

    
    for (CollisionInfo& c : collisionsInfo) {
        size_t oldEntitiesSize = intersectedEntities.size();

        IntersectionInfo intersectionInfo = collisionToIntersectionInfo(physComp, c);

        if (!intersectionInfo.entity) {
            Logger::Print("Collision detected for Physics Component \"" + physComp->getName() + "\", but it was not in the returned collision info ", LOGCAT::CAT_PHYSICS | LOGCAT::CAT_CORE, LOGSEV::SEV_ERROR);
            return intersections;
        }

        

        // Check to see if we can include
        bool doInclude = true;
        if (testInfo.testType != IntersectionTestType::INTERSECT_ALL) {
            
            if (testInfo.testType == IntersectionTestType::INTERSECT_INC_GROUP) {
                doInclude = false;
            }
            else if (testInfo.testType == IntersectionTestType::INTERSECT_EXC_GROUP) {
                doInclude = false;
            }
            
            for (Group* g : testInfo.groups) {
                if (g->hasEntity(intersectionInfo.entity)) {
                    doInclude = !doInclude;
                    break;
                }
            }
        }


        if (doInclude) {
            intersectedEntities.insert(intersectionInfo.entity);
            

            if (intersectedEntities.size() > oldEntitiesSize) {
                intersections.push_back(intersectionInfo);
                if (testInfo.returnFirst) {
                    return intersections;
                }
            }
        }

    }

    return intersections;
}

std::vector<IntersectionInfo> CollisionManager::getIntersections(Entity* entity, IntersectionTestInfo& testInfo, bool doRefreshSceneBVH)
{
    PhysicsComponent* physComp = (PhysicsComponent*)entity->getComponentOfType("PhysicsComponent");
    if (!physComp) {
        Logger::Print("No PhysicsComponent connected to Entity \"" + entity->getName() + "\", so cannot get intersections", LOGCAT::CAT_PHYSICS, LOGSEV::SEV_ERROR);
    }
    return getIntersections(physComp, testInfo, doRefreshSceneBVH);
}




void CollisionManager::handleCollisions()
{
    dSpaceCollide(elmt::core::getPhysicsWorld()->getSpaceID(), this, &CollisionManager::staticNearCallback);

    //Take step in world simulation
    core::getPhysicsWorld()->takeStep();
    core::getPhysicsWorld()->clearJoints();

}


int CollisionManager::addPhysicsComponent(PhysicsComponent* comp)
{
    if (std::find(physicsComponents.begin(), physicsComponents.end(), comp) != physicsComponents.end()) {
        Logger::Print("Attempted to add Physics Component \"" + comp->getName() + "\" to Collision Manager, but it was already added", LOGCAT::CAT_PHYSICS, LOGSEV::SEV_WARNING );
        return 1;
    }
    physicsComponents.push_back(comp);
    return 0;
}

int CollisionManager::removePhysicsComponent(PhysicsComponent* comp) {
    auto physIt = std::find(physicsComponents.begin(), physicsComponents.end(), comp);
    if (physIt == physicsComponents.end()) {
        Logger::Print("Attempted to remove Physics Component: \"" + comp->getName() + "\" from Collision Manager, but it not found", LOGCAT::CAT_PHYSICS, LOGSEV::SEV_WARNING);
        return 1;
    }

    size_t i = std::distance(physicsComponents.begin(), physIt);

    physicsComponents.erase(physIt);
    entityData.erase(entityData.begin() + i);

    // Change BVH IDs of the other components
    for (auto p = physicsComponents.begin() + i; p != physicsComponents.end(); p++){
        i = std::distance(physicsComponents.begin(), p);
        PhysicsComponent* pComp =  *p;
        pComp->bvhID--;
    }
    return 0;
}

/*
 *
 */

PhysicsComponent* CollisionManager::getComponentFromID(size_t bvhID)
{
    for (PhysicsComponent* physicsComponent : physicsComponents) {
        if (physicsComponent->getBVHID() == bvhID) {
            return physicsComponent;
        }
    }
    return nullptr;
}


void CollisionManager::testBox(BVH* bvh1, BVH* bvh2, std::vector<CollisionInfo>& collisions, bool returnFirst) {
    testPairs.clear();

    bvhTestPair start;
    start.box1 = &bvh1->root;
    start.box2 = &bvh2->root;
    testPairs.push_back(start);

    size_t pointer = 0;

    while (pointer < testPairs.size()) {


        bvhTestPair pair = testPairs[pointer++];

        if (comparePair(pair)) {
            continue;
        }

        if (pair.box1->numTriangles == 1 && pair.box2->numTriangles == 1) {

            if (pair.box1->bvh != bvh1){
                std::swap(pair.box1, pair.box2);
            }

            Triangle tri1 = pair.box1->bvh->getTriangle(pair.box1);
            Triangle tri2 = pair.box2->bvh->getTriangle(pair.box2);

            glm::vec3 averageCentroid;
            if (!triangleTriangleIntersectionTest(tri1, tri2, *pair.box1->transform, *pair.box2->transform, &averageCentroid)) {
                continue; //AABBs intersect, but not the triangles
            }

            CollisionInfo collisionInfo;
            collisionInfo.pointOfContact = averageCentroid;
            collisionInfo.surfaceNormalObject1 = tri1.normal;
            collisionInfo.surfaceNormalObject2 = tri2.normal;
            collisionInfo.entityData1 = getEntityData(pair.box1->bvh);
            collisionInfo.entityData2 = getEntityData(pair.box2->bvh);
            collisionInfo.meshID1 = tri1.meshIndex;
            collisionInfo.meshID2 = tri2.meshIndex;

            if (collisionInfo.entityData1 != collisionInfo.entityData2){
                collisions.push_back(collisionInfo);
            }

            collisions.push_back(collisionInfo);
            if (returnFirst) {
                return;
            }
            continue;
        }

        if (!AABB_AABB_intersectionTest(pair.box1, pair.box2, *pair.box1->transform, *pair.box2->transform)) {
            continue;
        }

        AABB *min, *max;

        if (pair.box1->numTriangles < pair.box2->numTriangles) {
            min = pair.box1;
            max = pair.box2;
        }

        else {
            min = pair.box2;
            max = pair.box1;
        }

        bvhTestPair test1 = { min, max->c1 };
        bvhTestPair test2 = { min, max->c2 };

        testPairs.push_back(test1);
        testPairs.push_back(test2);
    }
}

EntityData* CollisionManager::getEntityData(BVH *bvh) {
    for (auto &data : entityData){
        if (data.bvh == bvh) return &data;
    }
    return nullptr;
}


void CollisionManager::testBox(BVH *bvh, std::vector<CollisionInfo>& collisions) {
    testBox(bvh, sceneBVH, collisions);
}


bool CollisionManager::comparePair(bvhTestPair &pair) {
    return pair.box1->bvh == pair.box2->bvh;
}

CollisionManager::~CollisionManager() {
    delete sceneBVH;
}


size_t CollisionManager::getID(BVH *bvh) {
    for (size_t i = 0; i < entityData.size(); i++){
        if (bvh == entityData[i].bvh) return i;
    }

    Logger::Print("Error - attempting to get ID of bvh that does not exist\n", LOGCAT::CAT_PHYSICS, LOGSEV::SEV_ERROR);
    return -1;
}

RayInfo CollisionManager::rayCast(glm::vec3 origin, glm::vec3 direction) {

    if (entityData.size() == 0){
        return {};
    }

    delete sceneBVH;
    sceneBVH = new BVH(entityData);

    std::vector<AABB*> testBoxes;
    testBoxes.push_back(&sceneBVH->root);

    size_t stackPointer = 0;
    float maxDistance = 9999999;
    float minDistance = 0;

    RayInfo info{};

    while (stackPointer < testBoxes.size()){
        AABB *box = testBoxes[stackPointer++];

        if (box->numTriangles == 1){
            Triangle tri = box->bvh->getTriangle(box);

            glm::vec3 barycentrics;
            if (!rayTriangleIntersectionTest(box->transform, tri, &barycentrics, origin, direction)) continue;

            float distance = barycentrics.z;
            glm::vec3 point = origin + (distance * direction);
            if (distance > maxDistance) continue;

            maxDistance = distance;
            size_t id = getID(box->bvh);
//            info.modelID = bvhIDs[id];
            info.entity = entityData[id].entity;
            info.meshID = tri.meshIndex;
            info.hit = true;
            info.hitPos = origin = (direction * distance);
            continue;
        }

        if (rayAABB_intersectionTest(box, origin, direction, 0, maxDistance)){
            testBoxes.push_back(box->c1);
            testBoxes.push_back(box->c2);
        }
    }

    return info;
}




void CollisionManager::resolveCollision(dGeomID o1, dGeomID o2){
    //Before going any further, check whether there's actually a contact
    const int MAX_CONTACTS = 1024;
    dContact contact[MAX_CONTACTS];

    int numContacts = dCollide(o1, o2, MAX_CONTACTS, &contact[0].geom, sizeof(dContact));
    if (numContacts == 0) return;

    //Get the bodies of the colliding objects
    dBodyID body1 = dGeomGetBody(o1);
    dBodyID body2 = dGeomGetBody(o2);

    //Check that both bodies are valid
    if (!body1 || !body2) return;

    //Get the physics components for the bodies
    PhysicsComponent *p1 = nullptr, *p2 = nullptr;

    //TODO: this is a linear search -- this could be a big problem for any non tiny amount of components
    for (PhysicsComponent **p = &physicsComponents[0]; !p1 || !p2; p++){
        if ((*p)->getBody() == body1) p1 = *p;
        else if ((*p)->getBody() == body2) p2 = *p;
    }


    //Make sure that body1 is dynamic - this is important for the order of the joints
    //Note there are 2 types of collisions, dynamic-kinematic and dynamic-dynamic
    if (!p1->getIsDynamic()){
//        if (!p2->getIsDynamic()) return;
        std::swap(o1, o2);
        std::swap(body1, body2);
        std::swap(p1, p2);
    }

//    if (p1->customCollision || p2->customCollision) return;

    ContactParams body1Params = p1->getContactParams();
    ContactParams body2Params = p2->getContactParams();


    //Finally, set contact parameters and resolve collisions
    for (size_t i = 0; i < numContacts; i++){

//        if (p1->customCollision || p2->customCollision) {
//            glm::vec3 pos(contact[i].fdir1[0], contact[i].fdir1[1], contact[i].fdir1[2]);
//            glm::vec3 normal(contact[i].geom.normal[0], contact[i].geom.normal[1], contact[i].geom.normal[2]);
//
//            customCollision(p1, p2, pos, normal, 0);
//            continue;
//        }


        setSurfaceParams(contact[i], body1Params, body2Params);
        dJointID c = dJointCreateContact(elmt::core::getPhysicsWorld()->getWorldID(),
                                         elmt::core::getPhysicsWorld()->getContactGroupID(), &contact[i]);

        dJointAttach(c, body1, body2);
    }
}


void CollisionManager::setSurfaceParams(dContact &contact, ContactParams &body1Params, ContactParams &body2Params) {
    contact.surface.mode = body1Params.mode;
    contact.surface.mu = body1Params.mu;
    contact.surface.mu2 = body2Params.mu;
    contact.surface.rho = body1Params.rho;
    contact.surface.rho2 = body2Params.rho;
    //contact[i].surface.rhoN = contactParameters.rhoN;

    contact.surface.bounce = std::max(body1Params.bounce, body2Params.bounce); //Use the maximum of the bounce values
    contact.surface.bounce_vel = std::max(body1Params.bounce_vel, body2Params.bounce_vel);

    contact.surface.soft_erp = std::max(body1Params.soft_erp, body2Params.soft_erp);
    contact.surface.soft_cfm = std::max(body1Params.soft_cfm, body2Params.soft_cfm);
    contact.surface.motion1 = body1Params.motion;
    contact.surface.motion2 = body2Params.motion;

    //contact[i].surface.motionN = body.motionN;
    contact.surface.slip1 = body1Params.slip;
    contact.surface.slip2 = body2Params.slip;
}



void CollisionManager::staticNearCallback(void* data, dGeomID o1, dGeomID o2){
    auto *manager = static_cast<CollisionManager*>(data);
    manager->resolveCollision(o1, o2);
}


bool CollisionManager::simulateEntityStep(BVH* obvh, std::set<BVH*>& bvhs, std::vector<CollisionInfo>& stepCollisions) {


    stepCollisions.clear();
    bool isColliding = false;
    for (BVH* bvh : bvhs) {
        testBox(obvh, bvh, stepCollisions, true);
        if (!stepCollisions.empty()) {
            isColliding = true;
            break;
        }
    }


    return isColliding;
}


bool CollisionManager::simulateEntityStep(BVH* obvh, BVH* bvhTest, std::vector<CollisionInfo>& stepCollisions) {
    stepCollisions.clear();
    bool isColliding = false;
    testBox(obvh, bvhTest, stepCollisions, true);
    if (!stepCollisions.empty()) {
        isColliding = true;
    }
    return isColliding;

}

// Used by simulateEntity, get slope/forward of a triangle... or guess it, at least
glm::vec3 getSlope(const glm::vec3& normal) {
    glm::vec3 down = glm::vec3(0.0, -1.0, 0.0);
    if (normal == down || -normal == down) {
        return glm::vec3(1.0, 0.0, 0.0);
    }

    glm::vec3 right = glm::cross(normal, down);
    glm::vec3 forward = glm::cross(normal, right);
    return forward;
}


glm::vec3 CollisionManager::simulateEntity(PhysicsComponent *physicsComponent, float time) {
    EntityData data;
    for (auto &d : entityData){
        if (d.physicsComponent == physicsComponent){
            data = d;
            break;
        }
    }

    glm::vec3 velocity = physicsComponent->customVelocity;
    glm::vec3 attemptedMove = velocity * time;

    std::vector<CollisionInfo> collisions;
    refreshSceneBVH();


    auto oldBVHTransform = data.bvh->transform;

    const bool enableRotations = false;
    if (!enableRotations) {
        auto& t = data.bvh->transform;
        t[0][0] = data.entity->scale.x;
        t[0][1] = 0.0;
        t[0][2] = 0.0;

        t[1][0] = 0.0;
        t[1][1] = data.entity->scale.y;
        t[1][2] = 0.0;


        t[2][0] = 0.0;
        t[2][1] = 0.0;
        t[2][2] = data.entity->scale.z;
    }

    // Use to double check
    //std::vector<CollisionInfo> oldCollisions;
    //testBox(data.bvh, oldCollisions);

    // First see if we can do the whole thing
    data.bvh->transform[3][0] += attemptedMove.x;
    data.bvh->transform[3][1] += attemptedMove.y;
    data.bvh->transform[3][2] += attemptedMove.z;
    testBox(data.bvh, collisions);
    data.bvh->transform[3][0] -= attemptedMove.x;
    data.bvh->transform[3][1] -= attemptedMove.y;
    data.bvh->transform[3][2] -= attemptedMove.z;

    
    if (collisions.empty()) {
//        printf("No collision detected\n");
        return velocity * time;
    }


    glm::vec3 totalResponse = glm::vec3(0, 0, 0);

    IntersectionInfo info;
    

    std::set<PhysicsComponent*> physComps = {};
    std::set<BVH*> bvhs = {};
    std::vector<BVH*> bvhVector = {};

    PhysicsComponent* pc;
    /*size_t lowestPoint;
    float lowestPointVal = 99999.0;
    size_t highestPoint;
    float highestPointVal = -99999.0;*/

    // Get all the data we will need to check later
    size_t index = 0;
    for (auto& collision : collisions) {
        info = collisionToIntersectionInfo(physicsComponent, collision);
        pc = (PhysicsComponent*)info.entity->getComponentOfType("PhysicsComponent");
        physComps.insert(pc);

        size_t oldBVHCount = bvhs.size();
        bvhs.insert(pc->bvh);
        if (bvhs.size() > oldBVHCount) {
            bvhVector.push_back(pc->bvh);
        }

        /*if (collision.pointOfContact.y < lowestPointVal) {
            lowestPoint = index;
            lowestPointVal = collision.pointOfContact.y;
        }
        if (collision.pointOfContact.y > highestPointVal) {
            highestPoint = index;
            highestPointVal = collision.pointOfContact.y;
        }*/
        index++;
        
    }
    /*lowestPoint = 0;
    highestPoint = 0;*/

    info = collisionToIntersectionInfo(physicsComponent, collisions[0]); //highestPoint
    //auto infoHighest = collisionToIntersectionInfo(physicsComponent, collisions[highestPoint]);
    //std::cout << "lowest: " << info.colliderNormal.x << ", " << info.colliderNormal.y << ", " << info.colliderNormal.z << std::endl;

    bool isTouchingFloor;
    if (info.colliderNormal.y > 0.8) {
        isTouchingFloor = true;
    }
    else {
        isTouchingFloor = false;
    }
    if (isTouchingFloor) {
        if (attemptedMove.y < 0) {
            attemptedMove.y = 0;
        }
    }

    // Break the movement into steps and see how many steps we can do
    glm::vec3 startPos = glm::vec3(physicsComponent->bvh->transform[3][0], physicsComponent->bvh->transform[3][1], physicsComponent->bvh->transform[3][2]);
    std::vector<CollisionInfo> stepCollisions = {};
    float steps = 10.0;
    float step;


    std::vector<unsigned int> dims = { 1, 0, 2 };
    glm::mat3 axes = { {1.0,0.0,0.0}, {0.0,1.0,0.0}, {0.0,0.0,1.0} };

    // Used later for displacement, basically shifting back along the normal
    glm::vec3 bitangent = getSlope(info.colliderNormal);
    glm::vec3 normal = info.colliderNormal;
    glm::vec3 tangent = glm::cross(normal, bitangent);

    auto mAxes = axes;
    mAxes[0] = glm::normalize(axes[1] + (normal * glm::dot(normal, axes[1])));
    mAxes[1] = glm::normalize(axes[0] + (bitangent * glm::dot(bitangent, axes[0])));
    mAxes[2] = glm::normalize(axes[1] + (normal * glm::dot(normal, axes[1])));


    float stepMult = 1.0;
    for (int i = 0; i < steps; i++) {
        stepMult /= 2.0;
        
        

        glm::vec3 axis, stepVec;
        for (unsigned int dim: dims) {
            step = attemptedMove[dim] * stepMult;
            if (!step) {
                continue;
            }
            if (dim == 1 && isTouchingFloor) {
                continue;
            }
            axis = axes[dim];
            
            
            
            // Performa a single step
            stepVec = axis * step;

            
            data.bvh->transform[3][0] += stepVec.x;
            data.bvh->transform[3][1] += stepVec.y;
            data.bvh->transform[3][2] += stepVec.z;
            bool isColliding = simulateEntityStep(data.bvh, bvhs, stepCollisions);

            if (isColliding) {
                    
                // If we collide, see if we can displace our way out of it (used for slopes and the like)
                glm::vec3 vStep;

                size_t largestComp = 0;
                if (fabs(normal[1]) > fabs(normal[largestComp])) {
                    largestComp = 1;
                }
                if (fabs(normal[2]) > fabs(normal[largestComp])) {
                    largestComp = 2;
                }

                glm::vec3 vAxis = axes[largestComp] * (normal[largestComp] / fabs(normal[largestComp]));

                unsigned int vSteps = 8;
                float startStepSize = 0.05;
                float stepSize = startStepSize;

                float totalStepFrac = 0.0;

                for (unsigned int vi = 0; vi < vSteps; vi++ ) {
                    vStep = vAxis * stepSize;
                    // Basically, see if we can do the full displacement
                    if (vi != 0) {
                        vStep *= -1.0;
                        totalStepFrac -= stepSize;
                    }
                    else {
                       // If we can do the full displacement, see how little displacement we can get away with
                        totalStepFrac += stepSize;
                            
                    }
                        
                    stepSize /= 2.0;

                    data.bvh->transform[3][0] += vStep.x;
                    data.bvh->transform[3][1] += vStep.y;
                    data.bvh->transform[3][2] += vStep.z;
                    bool isColliding = simulateEntityStep(data.bvh, bvhs, stepCollisions);
                        


                    if (isColliding) {
                        data.bvh->transform[3][0] -= vStep.x;
                        data.bvh->transform[3][1] -= vStep.y;
                        data.bvh->transform[3][2] -= vStep.z;
                            
                        if (vi == 0) {
                            vStep = glm::vec3(0.0, 0.0, 0.0);
                            data.bvh->transform[3][0] -= stepVec.x;
                            data.bvh->transform[3][1] -= stepVec.y;
                            data.bvh->transform[3][2] -= stepVec.z;
                                                        break;
                        }
                        else {
                            totalStepFrac -= stepSize;

                        }


                        std::vector<CollisionInfo> newCollisions;

                    }


                }

                 
                //std::cout << vStep.x << ", " << vStep.y << ", " << vStep.z << ": " << totalStepFrac << std::endl;
                    

        
            }

            
        }
    }
    glm::vec3 endPos = glm::vec3(physicsComponent->bvh->transform[3][0], physicsComponent->bvh->transform[3][1], physicsComponent->bvh->transform[3][2]);

    glm::vec3 ans = endPos - startPos;



    glm::vec3 diff = attemptedMove - ans;

    //std::cout << "res: " << glm::length(attemptedMove) << " -> " << glm::length(ans) << ", " << glm::length(diff) << "(" << diff.x << ", " << diff.y << ", " << diff.z << ")" << " (" << bvhVector.size() << ")" << std::endl;

    data.bvh->transform = oldBVHTransform;
    return ans;// glm::length(ans) < 0.008 ? glm::vec3(0, 0, 0) : ans;

}

void CollisionManager::customCollision(PhysicsComponent *p1, PhysicsComponent *p2, glm::vec3 intersection, glm::vec3 normal, float depth) {
    glm::vec3 v1 = p1->getLinearVelocity();

//    if (glm::dot(normal, v1) > 0){
//        p1->setNewPos(normal * glm::length(v1)) ;
//        return;
//    }
//    if (glm::dot(glm::normalize(v1), glm::normalize(normal)) < -0.8){
//        p1->setNewPos(-v1);
//        return;
//    }

    glm::vec3 response = -1.f * glm::dot(normal, v1) * normal;
    response += normal * depth;

//    if (fabs(response.x) < 0.05) response.x = 0;
//    if (fabs(response.y) < 0.05) response.y = 0;
//    if (fabs(response.z) < 0.05) response.z = 0;

    p1->setNewPos(response);
}

















