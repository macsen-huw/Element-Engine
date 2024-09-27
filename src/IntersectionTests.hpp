#pragma once
#include "BoundingVolumeHierarchy.hpp"
#include "glm.hpp"


namespace elmt{

    struct AABB;
    struct Triangle;

    bool AABB_AABB_intersectionTest(AABB *box1, AABB *box2, glm::mat4x4 &transform1, glm::mat4x4 &transform2);
    /*bool AABB_AABB_intersectionTest(AABB* box1, AABB* box2) {
        return AABB_AABB_intersectionTest(box1, box2, glm::mat4x4(1.0), glm::mat4x4(1.0) );
    }*/

    bool triangleTriangleIntersectionTest(Triangle &triangle1, Triangle &triangle2, glm::mat4x4 &transform1,
                                          glm::mat4x4 &transform2, glm::vec3 *intersection);

    bool planeAABB_intersectionTest(AABB *box, glm::mat4x4 &transform, glm::vec4 &plane);

    bool rayAABB_intersectionTest(AABB *box, glm::vec3 rayOrigin, glm::vec3 rayDir, float minD, float maxD);
    bool rayTriangleIntersectionTest(glm::mat4x4 *transform, Triangle &triangle, glm::vec3 *barycentrics, glm::vec3 rayOrigin, glm::vec3 rayDir);

}