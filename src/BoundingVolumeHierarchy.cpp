#include "BoundingVolumeHierarchy.hpp"
#include "LogType.hpp"
#include "Logger.hpp"

#include "algorithm"

using namespace elmt;

/*
 * creates a bvh from a mesh
 */

BVH::BVH(std::vector<Mesh> &meshes) {
    for (size_t j = 0; j < meshes.size(); j++) {
        auto &mesh = meshes[j];
        for (size_t i = 0; i < mesh.indices.size(); i += 3) {
            Triangle newTri;
            newTri.vertices[0] = mesh.vertices[mesh.indices[i + 0]].Position;
            newTri.vertices[1] = mesh.vertices[mesh.indices[i + 1]].Position;
            newTri.vertices[2] = mesh.vertices[mesh.indices[i + 2]].Position;

            newTri.centre = (newTri.vertices[0] + newTri.vertices[1] + newTri.vertices[2]) / 3.f;
            glm::vec3 e1 = newTri.vertices[1] - newTri.vertices[0];
            glm::vec3 e2 = newTri.vertices[2] - newTri.vertices[0];
            glm::vec3 normal = glm::normalize(glm::cross(e1, e2));
            newTri.normal = normal;
            newTri.meshIndex = j;

            triangles.push_back(newTri);
        }
    }

    numPrimitives = triangles.size();

    calculateModelBounds();
    prepareBoundingBoxes();
    initAAC();
}


BVH::BVH(Mesh &mesh){
    for (size_t i = 0; i < mesh.indices.size(); i += 3) {
        Triangle newTri;
        newTri.vertices[0] = mesh.vertices[mesh.indices[i + 0]].Position;
        newTri.vertices[1] = mesh.vertices[mesh.indices[i + 1]].Position;
        newTri.vertices[2] = mesh.vertices[mesh.indices[i + 2]].Position;

        newTri.centre = (newTri.vertices[0] + newTri.vertices[1] + newTri.vertices[2]) / 3.f;
        glm::vec3 e1 = newTri.vertices[1] - newTri.vertices[0];
        glm::vec3 e2 = newTri.vertices[2] - newTri.vertices[0];
        glm::vec3 normal = glm::normalize(glm::cross(e1, e2));
        newTri.normal = normal;
        newTri.meshIndex = 0;

        triangles.push_back(newTri);
    }

    numPrimitives = triangles.size();

    calculateModelBounds();
    prepareBoundingBoxes();
    initAAC();
}

/*
 * creates a bvh from a vector of existing bvhs, and their associated transforms
 */

BVH::BVH(std::vector<EntityData> &entityData) {

    transform = glm::mat4x4(1.f);

    if (entityData.empty()){
        Logger::Print("Warning - attempting to create a bvh from 0 sub bvhs\n", LOGCAT::CAT_LOADING, LOGSEV::SEV_WARNING);
        return;
    }

    boundingBoxes.clear();
    nodes.clear();

    for (auto & i : entityData){
        AABB rootBox = i.bvh->root;

        rootBox.bvh = this;

        for (int j = 0; j < 4; j++){
            for (int k = 0; k < 4; k++){
                if (isnan((*rootBox.transform)[j][k])){
                    volatile int *x = nullptr;
                    *x = 0;
                }
                assert(!isnan((*rootBox.transform)[j][k]));
            }
        }

//        *rootBox.transform = glm::mat4x4(1.f);
        rootBox.b1 = glm::vec3(*rootBox.transform * glm::vec4(rootBox.b1, 1.f));
        rootBox.b2 = glm::vec3(*rootBox.transform * glm::vec4(rootBox.b2, 1.f));
        rootBox.transform = &transform;

        for (int j = 0; j < 3; j++) {

            //the bounds can become out of order under certain transforms (eg rotation), so make sure they're right
            if (rootBox.b1[j] > rootBox.b2[j]) std::swap(rootBox.b1[j], rootBox.b2[j]);
        }
        assert(!isnan(rootBox.b1.x));
        assert(!isnan(rootBox.b1.y));
        assert(!isnan(rootBox.b1.z));
        assert(!isnan(rootBox.b2.x));
        assert(!isnan(rootBox.b2.y));
        assert(!isnan(rootBox.b2.z));


        assert(rootBox.b1.x <= rootBox.b2.x);
        assert(rootBox.b1.y <= rootBox.b2.y);
        assert(rootBox.b1.z <= rootBox.b2.z);

        boundingBoxes.push_back(rootBox);
    }

    for (auto &box : boundingBoxes){
        nodes.push_back(&box);
        assert(box.b1.x <= box.b2.x);
        assert(box.b1.y <= box.b2.y);
        assert(box.b1.z <= box.b2.z);
    }

    numPrimitives = boundingBoxes.size();

    std::sort(boundingBoxes.begin(), boundingBoxes.end(), AABB_Sort());
    root.transform = &transform;
    initAAC();

}









//uses the bit spreading algorithm presented in Physically Based Rendering (PBR)
//can be found at: https://www.pbr-book.org/3ed-2018/Primitives_and_Intersection_Acceleration/Bounding_Volume_Hierarchies

uint32_t BVH::spreadBits(uint32_t x) {

    if (x >= (1 << 10)) x = (1 << 10) - 1;

    x = (x | (x << 16)) & 0x30000FF;
    x = (x | (x << 8))  & 0x300F00F;
    x = (x | (x << 4))  & 0x30C30C3;
    x = (x | (x << 2))  & 0x9249249;
    return x;
}




void BVH::calculateModelBounds() {

    glm::vec3 min = triangles[0].vertices[0];
    glm::vec3 max = triangles[0].vertices[0];

    for (Triangle &tri : triangles){
        for (auto vertex : tri.vertices) {
            for (int j = 0; j < 3; j++) {
                min[j] = std::min(vertex[j], min[j]);
                max[j] = std::max(vertex[j], max[j]);
            }
        }
    }

    root.b1 = min;
    root.b2 = max;

}



//wraps each triangle in a bounding box and stores that bb in the bounding boxes vector
void BVH::prepareBoundingBoxes() {

    for (size_t i = 0; i < triangles.size(); i++){
        initBoundingBox(i);
    }

    //we use std::sort here, but the authors of AAC recommend using a radix sort
    //if we run into performance issues, we should investigate this option
    std::sort(boundingBoxes.begin(), boundingBoxes.end(), AABB_Sort());

    for (auto &box : boundingBoxes){
        nodes.push_back(&box);
        assert(nodes.back()->b1.x <= nodes.back()->b2.x);
        assert(nodes.back()->b1.y <= nodes.back()->b2.y);
        assert(nodes.back()->b1.z <= nodes.back()->b2.z);

    }

}




void BVH::initBoundingBox(size_t i) {
    //we have a limited number of bits in the morton code for each triangle (10 for each axis) so we're sorting each axis
    //into 1 of 1024 buckets essentially
    //we need to make sure the buckets are the best possible size, we could say each bucket is 1m wide, but that doesn't
    //scale for big models and is too low detail for small models
    //so instead we can use the bounds of the model!

    glm::vec3 modelSize = root.b2 - root.b1;

    Triangle &tri = triangles[i];
    uint32_t xVal = (uint32_t) (((tri.centre.x - root.b1.x) / modelSize.x) * 1024.f);
    uint32_t yVal = (uint32_t) (((tri.centre.y - root.b1.y) / modelSize.y) * 1024.f);
    uint32_t zVal = (uint32_t) (((tri.centre.z - root.b1.z) / modelSize.z) * 1024.f);

    uint32_t mortonCode = (spreadBits(xVal) << 2) | (spreadBits(yVal) << 1) | (spreadBits(zVal) << 0);
    assert(mortonCode);

    AABB boundingBox;
    boundingBox.c1 = nullptr;
    boundingBox.c2 = nullptr;
    boundingBox.transform = &transform;
    boundingBox.bvh = this;

    boundingBox.b1.x = std::min(std::min(tri.vertices[0].x, tri.vertices[1].x), tri.vertices[2].x);
    boundingBox.b1.y = std::min(std::min(tri.vertices[0].y, tri.vertices[1].y), tri.vertices[2].y);
    boundingBox.b1.z = std::min(std::min(tri.vertices[0].z, tri.vertices[1].z), tri.vertices[2].z);

    boundingBox.b2.x = std::max(std::max(tri.vertices[0].x, tri.vertices[1].x), tri.vertices[2].x);
    boundingBox.b2.y = std::max(std::max(tri.vertices[0].y, tri.vertices[1].y), tri.vertices[2].y);
    boundingBox.b2.z = std::max(std::max(tri.vertices[0].z, tri.vertices[1].z), tri.vertices[2].z);

    boundingBox.numTriangles = 1;
    boundingBox.triangleIndex = i;
    boundingBox.mortonCode = mortonCode;
    boundingBoxes.push_back(boundingBox);
}




void BVH::initAAC() {
    BuildTreeParameters initialParameters{}, returnParam{};
    initialParameters.arrStart = 0;
    initialParameters.size = boundingBoxes.size();
    initialParameters.returnAddress = &returnParam;

    BuildTreeParameters temp{};
    for (size_t i = 0; i < boundingBoxes.size(); i++) callStack.push_back(temp);

    pushToStack(initialParameters);
    buildTree();

    returnParam.rightHandPartition = &temp;

    assert(returnParam.leftHandPartition);
    assert(returnParam.rightHandPartition);
    assert(returnParam.rightHandPartition->size == 0);


    std::vector<AABB*> top = combineClusters(returnParam.leftHandPartition, returnParam.rightHandPartition, 1);

    int count = 0;
    for (int i = 0; i < top.size(); i++){
        if (top[i] != nullptr) {
            root = *top[i];
            count++;
        }
    }
    root.transform = &transform;

    assert(count == 1);

}


void BVH::buildTree() {

    storageStackAABB.resize(numPrimitives * 4); //we are definitely over allocating here, like enormously
    storageStackParameters.resize(numPrimitives * 4); //ditto

    while (stackPointer >= 0){

        BuildTreeParameters param = callStack[stackPointer];

        size_t start = param.arrStart;
        size_t end = param.arrStart + param.size - 1;
        size_t size = param.size;
        BuildTreeParameters *lhs = param.leftHandPartition;
        BuildTreeParameters *rhs = param.rightHandPartition;
        BuildTreeParameters *returnAddress = param.returnAddress;

        assert(start <= end);
        assert(returnAddress);

        /*
         * check if the current cluster is too big, if it is we need to partition the array further
         * conceptually, we are moving DOWN the tree
         */
        if (size > maxClusterSize){


            size_t partitionIndex = makePartition(start, end);
            assert(partitionIndex <= end);
            assert(partitionIndex >= start);
            /*
             * check if the left hand partition of the nodes has been created yet, if not create it
             */
            if (lhs == nullptr) {

                BuildTreeParameters p1{};
                p1.arrStart = start;
                p1.size = partitionIndex - start;
                p1.returnAddress = &callStack[stackPointer]; //tell the left hand partition to store its results here

                assert(p1.size);
                pushToStack(p1);
                continue;
            }

            /*
             * the left hand partition has been created, so create the right hand partition if that hasn't been created
             */

            if (rhs == nullptr) {

                BuildTreeParameters p2{};
                p2.arrStart = partitionIndex;
                p2.size = end - partitionIndex + 1;
                assert(p2.size);
                p2.returnAddress = &callStack[stackPointer]; //tell the right hand partition to store its results here

                pushToStack(p2);
                continue;
            }

            /*
             * both child partitions have been created, now we can fall through to combining the 2 clusters
             */
        }

        /*
         * the current cluster size is small enough, we need to combine clusters
         * conceptually we are moving UP the tree
         *
         * there are 2 ways this can happen:
         * 1) we partitioned the cluster down into a small enough sub cluster -- no action needed, head back up the tree
         *    so that the parent node can merge us with our sibling node
         *
         * 2) we are the parent node, and we have 2 child nodes to merge
         */


        /*
         * check if we're the child node
         */
        if (size <= maxClusterSize){
            BuildTreeParameters returnParam;
            returnParam.arrStart = start;
            returnParam.size = size;

            if (returnAddress->leftHandPartition == nullptr){
                param.returnAddress->leftHandPartition = allocParameterStack();
                *param.returnAddress->leftHandPartition = returnParam;
                popFromStack();
                continue;
            }

            param.returnAddress->rightHandPartition = allocParameterStack();
            *param.returnAddress->rightHandPartition = returnParam;
            popFromStack();
            continue;
        }

        /*
         * if we've made it here, we have 2 clusters to merge
         */

        size_t childClusterSizes = lhs->size + rhs->size;
        assert(childClusterSizes);


        std::vector<AABB*> newCluster = combineClusters(lhs, rhs, calculateClusterSize(childClusterSizes));
        assert(!newCluster.empty());

        size_t index = 0;
        for (size_t i = 0; i < newCluster.size(); i++){
            if (newCluster[i] != nullptr){
                nodes[start + index] = newCluster[i];
                index++;
            }
        }

        assert(index);

        for (size_t i = index; i < size; i++){
            nodes[start + i] = nullptr;
        }

        //sort out the data we return (ie the start index of the new cluster and its end index)
        BuildTreeParameters returnParam;
        returnParam.arrStart = start;
        returnParam.size = index;

        if (param.returnAddress->leftHandPartition == nullptr){
            param.returnAddress->leftHandPartition = allocParameterStack();
            *param.returnAddress->leftHandPartition = returnParam;
            popFromStack();
            continue;
        }

        param.returnAddress->rightHandPartition = allocParameterStack();
        *param.returnAddress->rightHandPartition = returnParam;
        popFromStack();
    }
}


size_t BVH::calculateClusterSize(size_t currentSize) {
    return std::max(currentSize / 2, (size_t) 1);
}


/*
 * partitions a cluster in 2
 * takes the start index and end index of the cluster, and returns the start index of the right hand child cluster
 * determines the partition index by looking at the start and end node's morton codes
 * it partitions on the most significant different bit
 *
 * for example:
 * left = ... 0110 0101, right = ... 0111 0011
 * will return the index of the first node who's morton code has this bit set: ...0111 0000
 */


size_t BVH::makePartition(size_t start, size_t end) {
    uint32_t left = boundingBoxes[start].mortonCode;
    uint32_t right = boundingBoxes[end].mortonCode;
    assert(left <= right);
    assert(start <= end);

    if (left == right) return (start + end) / 2;

    uint32_t val = 1 << 31;
    uint32_t foo = val;
    while (true){
        if ((left & val) != (right & val)) break;
        val = val & left;
        val |= (foo >>= 1);
    }

    //we are now splitting on the val bit
    //we just need to find the first node with a morton code >= val
    //we can do this through a binary search
    //ideally we would use c++ binary search, but i don't think it works with a custom accept condition...

    size_t a = start;
    size_t b = end;

    assert(val >= left);
    assert(val <= right);

    while (true){
        size_t mid = (a + b) / 2;
        assert(mid >= start);
        assert(mid <= end);

        if (mid == a) return a + 1;

        if (boundingBoxes[mid].mortonCode >= val && boundingBoxes[mid].mortonCode - 1 < val) return mid;
	

        if (boundingBoxes[mid].mortonCode < val) {
            a = mid;
            continue;
        }

        if (boundingBoxes[mid].mortonCode >= val){
            b = mid;
            continue;
        }
    }

    //we should never get here...
    //the binary search should have already found the index to return
    assert(0);
    return 0;

}


std::vector<AABB*> BVH::combineClusters(BuildTreeParameters *left, BuildTreeParameters *right, size_t maxSize) {

    size_t startSize = left->size + right->size;
    size_t currentSize = startSize;

    //this may not be needed, we should be able to safely write from left->start to right->end without overwriting anything
    std::vector<AABB*> tempNodes;

    for (size_t i = 0; i < left->size; i++) {
        assert(nodes[left->arrStart + i]);
        tempNodes.push_back(nodes[left->arrStart + i]);
    }

    for (size_t i = 0; i < right->size; i++) {
        assert(nodes[right->arrStart + i]);
        tempNodes.push_back(nodes[right->arrStart + i]);
    }


    assert(tempNodes.size() == startSize);

    while (currentSize > maxSize){

        float bestVal = std::numeric_limits<float>::max();
        size_t bestIndex1 = 0, bestIndex2 = 0;

        //search array for the best nodes to merge
        for (size_t i = 0; i < startSize; i++){
            float sah;
            if (tempNodes[i] == nullptr) continue;
            size_t index = calculateNodeSAH(tempNodes, startSize, i, &sah);

            if (sah < bestVal){
                assert(i != index);
                bestIndex1 = i;
                bestIndex2 = index;
                bestVal = sah;
            }
        }

        assert(bestIndex1 != bestIndex2);

        //we now have the best pairing, so merge them!
        tempNodes[bestIndex1] = mergeBoxes(tempNodes[bestIndex1], tempNodes[bestIndex2]);
        tempNodes[bestIndex2] = nullptr; //mark the other node as visited

        --currentSize;
    }

    return tempNodes;

}



/*
 * for a node n, in the range [start, end], calculates the SAH for potentially merging node i with node x, x in [start, end], x != i
 * returns the index of the best partner, and stores the SAH for merging with that partner in the sah parameter
 */

size_t BVH::calculateNodeSAH(std::vector<AABB*> &tempNodes, size_t size, size_t n, float *sah) {

    assert(size > 1);
    assert(n < size);
    assert(size == tempNodes.size());
    assert(tempNodes[n]);
    assert(sah);

    float bestVal = std::numeric_limits<float>::max();
    size_t bestIndex = n;


    for (size_t i = 0; i < size; i++){

        if (i == n) continue; //dont compare node with itself

        if (tempNodes[i] == nullptr) continue; //node has been eaten by previous round of merging

        float val = surfaceAreaHeuristic(tempNodes[i], tempNodes[n]);

        if (val < bestVal){
            bestVal = val;
            bestIndex = i;
        }
    }

    //sanity check that we found a partner node
    assert(bestIndex != n);

    *sah = bestVal;
    return bestIndex;
}


float BVH::surfaceAreaHeuristic(AABB *c1, AABB *c2) {
    assert(c1);
    assert(c2);

    glm::vec3 min, max;
    calculateMergedBounds(c1, c2, &min, &max);
    glm::vec3 bounds = max - min;

    assert(bounds.x >= 0);
    assert(bounds.y >= 0);
    assert(bounds.z >= 0);

    float area = 2 * ((bounds.x * bounds.y) + (bounds.y * bounds.z) + (bounds.z * bounds.x));
    assert(area >= 0);
    return area;
}


void BVH::calculateMergedBounds(AABB *c1, AABB *c2, glm::vec3 *bound1, glm::vec3 *bound2) {

    assert(c1);
    assert(c2);
    assert(bound1);
    assert(bound2);
    
    glm::vec3 min, max;
    min.x = std::min(c1->b1.x, c2->b1.x);
    min.y = std::min(c1->b1.y, c2->b1.y);
    min.z = std::min(c1->b1.z, c2->b1.z);

    max.x = std::max(c1->b2.x, c2->b2.x);
    max.y = std::max(c1->b2.y, c2->b2.y);
    max.z = std::max(c1->b2.z, c2->b2.z);

    *bound1 = min;
    *bound2 = max;
}


AABB* BVH::mergeBoxes(AABB *c1, AABB *c2) {

    assert(c1);
    assert(c2);

    AABB *newBoxStorage = allocAABBStack();

    AABB newBox;
    calculateMergedBounds(c1, c2, &newBox.b1, &newBox.b2);
    newBox.transform = &transform;
    newBox.bvh = this;
    newBox.mortonCode = 0; //we don't need the morton code when travelling up, but set it to 0 anyways
    newBox.triangleIndex = 0; //ditto with the triangle index
    newBox.numTriangles = c1->numTriangles + c2->numTriangles;
    newBox.c1 = c1;
    newBox.c2 = c2;

    *newBoxStorage = newBox;
    return newBoxStorage;
}


void BVH::pushToStack(BuildTreeParameters param) {
    callStack[++stackPointer] = param;
    assert(stackPointer < callStack.size());
}


void BVH::popFromStack() {
    --stackPointer;
}


BuildTreeParameters* BVH::allocParameterStack() {
    assert(storageStackParametersPointer + 1 < storageStackParameters.size());
    return &storageStackParameters[++storageStackParametersPointer];
}


AABB *BVH::allocAABBStack() {
    assert(storageStackAABBPointer + 1 < storageStackAABB.size());
    return &storageStackAABB[++storageStackAABBPointer];
}



Triangle BVH::getTriangle(AABB *box) {
    assert(box);
    assert(box->triangleIndex < triangles.size());
    assert(box->bvh == this);

    return triangles[box->triangleIndex];
}

BVH::~BVH() {

}







