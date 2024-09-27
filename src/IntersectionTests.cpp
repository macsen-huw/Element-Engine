#include "IntersectionTests.hpp"
#include "glm.hpp"

using namespace elmt;

/*
 * calculates the determinant of the matrix:
 * [ a1    b1    c1    d1 ]
 * [ a2    b2    c2    d2 ]
 * [ a3    b2    c3    d3 ]
 * [ 1     1     1     1  ]
 */
static inline float calculateDeterminant(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d){
    return glm::dot(d - a, glm::cross(b - a, c - a));
}


//returns -1 if x < 0, 0 if x == 0, 1 if x > 0
static float getSign(float x){
    if (x == 0) return 0;
    return x / abs(x);
}



/*
 * returns true if 2 AABBs overlap each other, else returns false
 */
bool elmt::AABB_AABB_intersectionTest(AABB *box1, AABB *box2, glm::mat4x4 &transform1, glm::mat4x4 &transform2) {

    glm::vec3 b11 = glm::vec3(transform1 * glm::vec4(box1->b1, 1.f));
    glm::vec3 b12 = glm::vec3(transform1 * glm::vec4(box1->b2, 1.f));

    glm::vec3 b21 = glm::vec3(transform2 * glm::vec4(box2->b1, 1.f));
    glm::vec3 b22 = glm::vec3(transform2 * glm::vec4(box2->b2, 1.f));


    for (int i = 0; i < 3; i++){

        //the bounds can become out of order under certain transforms (eg rotation), so make sure they're right
        if (b11[i] > b12[i]) std::swap(b11[i], b12[i]);
        if (b21[i] > b22[i]) std::swap(b21[i], b22[i]);


        if (b11[i] > b22[i] || b21[i] > b12[i]){ //check if AABBs disjoint
            return false;
        }
    }

    return true;
}


/*
 * reorders the vertices and determinants for a triangle so that the first vertex/determinant is the odd one out
 * ie the first vertex is on the other side of the plane from the other 2
 */

static void reorderVertices(glm::vec3 *vertices, float *determinants){

    size_t diff;

    if (getSign(determinants[0]) == getSign(determinants[1])) diff = 2;
    else if (getSign(determinants[0]) == getSign(determinants[2])) diff = 1;

    else return; //the odd one out is already at the front, no action needed

    std::swap(vertices[0], vertices[diff]);
    std::swap(determinants[0], determinants[diff]);

}

/*
 * function returns true if 2 triangles intersect
 * gives a rough estimate of the point of intersection (average of all triangle's vertices)
 * more research needed into a proper way of doing this...
 */

bool elmt::triangleTriangleIntersectionTest(Triangle &triangle1, Triangle &triangle2, glm::mat4x4 &transform1,
                                                  glm::mat4x4 &transform2, glm::vec3 *intersection) {


    //TODO:
    //make sure it handles co planar triangles

    //transform each triangles vertices
    glm::vec3 v1[3] = {glm::vec3(transform1 * glm::vec4(triangle1.vertices[0], 1.f)),
                       glm::vec3(transform1 * glm::vec4(triangle1.vertices[1], 1.f)),
                       glm::vec3(transform1 * glm::vec4(triangle1.vertices[2], 1.f))
    };

    glm::vec3 v2[3] = {glm::vec3(transform2 * glm::vec4(triangle2.vertices[0], 1.f)),
                       glm::vec3(transform2 * glm::vec4(triangle2.vertices[1], 1.f)),
                       glm::vec3(transform2 * glm::vec4(triangle2.vertices[2], 1.f))
    };

    glm::vec3 averageCentre = (v1[0] + v1[1] + v1[2] + v2[0] + v2[1] + v2[2]) / 6.f;
    *intersection = averageCentre;

    //calculate the determinants for triangle 1 of each point in triangle 2
    //geometrically, d1[i] is negative if v2[i] is beneath triangle 1, and positive if v2[i] is above it
    //if all of triangle 2's vertices are above or all are below triangle 1 then its impossible for an intersection
    //so if all of d1 are negative/all positive we return false

    float d1[3];
    d1[0] = calculateDeterminant(v1[0], v1[1], v1[2], v2[0]);
    d1[1] = calculateDeterminant(v1[0], v1[1], v1[2], v2[1]);
    d1[2] = calculateDeterminant(v1[0], v1[1], v1[2], v2[2]);

    //triangles are coplanar, cheat and say that they overlap, needs fixing in the future
    if (getSign(d1[0]) == 0 && getSign(d1[1]) == 0 && getSign(d1[2]) == 0) return true;

    if (getSign(d1[0]) == getSign(d1[1]) && getSign(d1[1]) == getSign(d1[2])) {
        return false; //triangle 1 does not overlap with the plane of triangle 2
    }

    //do the same and check if triangle 1 crosses the plane of triangle 2
    float d2[3];
    d2[0] = calculateDeterminant(v2[0], v2[1], v2[2], v1[0]);
    d2[1] = calculateDeterminant(v2[0], v2[1], v2[2], v1[1]);
    d2[2] = calculateDeterminant(v2[0], v2[1], v2[2], v1[2]);

    if (getSign(d2[0]) == 0 || getSign(d2[1]) == 0 || getSign(d2[2]) == 0) return true;

    if (getSign(d2[0]) == getSign(d2[1]) && getSign(d2[1]) == getSign(d2[2])) {
        return false;
    }


    reorderVertices(v1, d1);
    reorderVertices(v2, d2);

    float test1 = calculateDeterminant(v1[0], v1[1], v2[0], v2[1]);
    float test2 = calculateDeterminant(v1[0], v1[2], v2[2], v2[0]);

    if (test1 <= 0 && test2 <= 0) return true;

    return false;
}


/*
 * algorithm from real time rendering 4th edition
 */

bool elmt::planeAABB_intersectionTest(AABB *box, glm::mat4x4 &transform, glm::vec4 &plane){


    glm::vec3 c = (box->b1 + box->b2) / 2.f;
    glm::vec4 transformed = transform *  glm::vec4(c, 1);
    c = glm::vec3(transformed.x, transformed.y, transformed.z);
    c /= transformed.w;

    glm::vec3 h = (box->b2 - box->b1) / 2.f;
    glm::vec3 n = glm::vec3(plane.x, plane.y, plane.z);
    float d = plane.w;

    float e = (h.x * abs(n.x)) + (h.y * abs(n.y)) + (h.z * abs(n.z));

    float s = glm::dot(c, n) + d;

    return s - e <= 0;
}


//ray triangle intersection test from real time rendering
bool elmt::rayTriangleIntersectionTest(glm::mat4x4 *transform, Triangle &triangle, glm::vec3 *barycentrics, glm::vec3 rayOrigin, glm::vec3 rayDir){
    glm::vec3 e1 = glm::vec3(*transform * glm::vec4(triangle.vertices[1] - triangle.vertices[0], 1));
    glm::vec3 e2 = glm::vec3(*transform * glm::vec4(triangle.vertices[2] - triangle.vertices[0], 1));

    glm::vec3 n = glm::cross(e1, e2);
    if (glm::dot(rayDir, n) > 0) return false; //cull back face

    glm::vec3 q = glm::cross(rayDir, e2);
    float a = glm::dot(e1, q);

    const float eps = 0.0000005;
    if (a > -eps && a < eps) return false;

    float f = 1.f / a;
    glm::vec3 s = rayOrigin - triangle.vertices[0];

    float u = f * glm::dot(s, q);
    if (u < 0) return false;

    glm::vec3 r = glm::cross(s, e1);
    float v = f * glm::dot(s, q);
    if (v < 0 || u + v > 1) return false;

    float t = f * glm::dot(e2, r);

    *barycentrics = glm::vec3(u, v, t);
    return true;
}




bool elmt::rayAABB_intersectionTest(AABB *box, glm::vec3 rayOrigin, glm::vec3 rayDir, float minD, float maxD){

    if (rayOrigin.x >= box->b1.x && rayOrigin.x <= box->b2.x &&
        rayOrigin.y >= box->b1.y && rayOrigin.y <= box->b2.y &&
        rayOrigin.z >= box->b1.z && rayOrigin.z <= box->b2.z) return true;

    glm::vec3 c1 = glm::vec3(*box->transform * glm::vec4(box->b1 - rayOrigin, 1)) / rayDir;
    glm::vec3 c2 = glm::vec3(*box->transform * glm::vec4(box->b2 - rayOrigin, 1)) / rayDir;

    glm::vec4 min (std::min(c1.x, c2.x), std::min(c1.y, c2.y), std::min(c1.z, c2.z), minD);
    glm::vec4 max (std::max(c1.x, c2.x), std::max(c1.y, c2.y), std::max(c1.z, c2.z), maxD);

    float tMin = std::max(std::max(min.x, min.y), std::max(min.z, min.w));
    float tMax = std::min(std::min(max.x, max.y), std::min(max.z, max.w));

    return (tMin <= tMax && tMax > 0);
}







