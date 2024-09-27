#include "Model.hpp"
#include <cstdio>
#include <cstdlib>
#include <unordered_set>
#include <cassert>
#include "../external/rapidobj.hpp"
#include "core.hpp"
#include "CollisionManager.hpp"

#define STB_IMAGE_STATIC
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

using namespace elmt;

//sorts all the vertices, normals and material IDs in O(n * m) time, where n is number of vertices and m is number of different materials
//we can assume m is low, so this is effectively a linear time sort
//void Model::sortByMaterials() {
//
//    //check the material, vertex and normal vectors are the same size
//    assert(mesh.materialIndices.size() == mesh.vertices.size());
//    assert(mesh.vertices.size() == mesh.normals.size());
//    assert(mesh.normals.size() == mesh.texCoords.size());
//
//    //now organise the meshes so that the 100% opaque materials go first, then anything else after
//    std::vector<size_t> newIndices(mesh.materials.size());
//
//    std::vector<BlinnPhongMaterial> opaqueMaterials;
//    std::vector<BlinnPhongMaterial> transparentMaterials;
//
//    size_t numOpaque = 0;
//    for (auto &m : mesh.materials){
//        if (m.opaque) ++numOpaque;
//    }
//
//    for (size_t i = 0; i < mesh.materials.size(); i++){
//        auto &m = mesh.materials[i];
//        if (m.opaque){
//            newIndices[i] = opaqueMaterials.size();
//            opaqueMaterials.push_back(m);
//        }
//        else {
//            newIndices[i] = numOpaque + transparentMaterials.size();
//            transparentMaterials.push_back(m);
//        }
//    }
//
//
//    for (size_t i = 0; i < opaqueMaterials.size(); i++){
//        mesh.materials[i] = opaqueMaterials[i];
//    }
//    for (size_t i = 0; i < transparentMaterials.size(); i++){
//        mesh.materials[i + numOpaque] = transparentMaterials[i];
//    }
//
//    for (size_t i = 0; i < mesh.materialIndices.size(); i++){
//        mesh.materialIndices[i] = newIndices[mesh.materialIndices[i]];
//    }
//
//
//    //create a vector containing the unsorted indices
//    std::vector<IndexPair> unsortedIndices;
//    for (uint32_t i = 0; i < mesh.materialIndices.size(); i++){
//        unsortedIndices.push_back(IndexPair{
//            mesh.materialIndices[i],
//            i
//        });
//    }
//
//    //count the number of each material
//    std::vector<size_t> materialCount(mesh.materials.size(), 0);
//    for (size_t i = 0; i < unsortedIndices.size(); i++){
//        materialCount[mesh.materialIndices[i]]++;
//    }
//
//
//    //store the offsets of each material in a look up table
//    std::vector<size_t> offsets(materialCount.size(), 0);
//    for (size_t i = 1; i < materialCount.size(); i++){
//        offsets[i] = offsets[i - 1] + materialCount[i - 1];
//    }
//
//    //sort the index pair's by material index by placing them into buckets defined by the offsets
//    std::vector<IndexPair> sortedIndices(unsortedIndices.size());
//    for (size_t i = 0; i < sortedIndices.size(); i++){
//        IndexPair pair = unsortedIndices[i];
//        size_t offset = offsets[pair.materialIndex]++;
//        sortedIndices[offset] = pair;
//    }
//
//    //create some buffers to store the old values in
//    //im pretty sure this is unnecessary and it can be done in place somehow
//    //but that is a job for another day
//    std::vector<glm::vec3> oldVertices(mesh.vertices);
//    std::vector<glm::vec3> oldNormals(mesh.normals);
//    std::vector<glm::vec2> oldTexCoords(mesh.texCoords);
//    std::vector<uint32_t> oldIndices(mesh.materialIndices);
//
//    //copy the old data into the new vectors, using the sorted index pairs
//    for (size_t i = 0; i < sortedIndices.size(); i++){
//        size_t expectedIndex = sortedIndices[i].vectorIndex;
//        mesh.vertices[i] = oldVertices[expectedIndex];
//        mesh.normals[i] = oldNormals[expectedIndex];
//        mesh.materialIndices[i] = oldIndices[expectedIndex];
//        mesh.texCoords[i] = oldTexCoords[expectedIndex];
//    }
//
//    //check all the sorting went well
//    for (size_t i = 0; i < mesh.materialIndices.size() - 1; i++){
//        assert(mesh.materialIndices[i] <= mesh.materialIndices[i + 1]);
//    }
//
//
//    printf("Num opaque materials = %lu\n", numOpaque);
//    printf("Num transparent materials = %lu\n\n", transparentMaterials.size());
//
//    assert(numOpaque == opaqueMaterials.size());
//    assert(opaqueMaterials.size() + transparentMaterials.size() == mesh.materials.size());
//
//    for (size_t i = 0; i < mesh.materialIndices.size() - 1; i++){
//        assert(mesh.materialIndices[i] <= mesh.materialIndices[i + 1]);
//    }
//}


Model::~Model() {

    for (auto& m : materials) {
        if (m.diffuseTexture.textureData != nullptr)          stbi_image_free(m.diffuseTexture.textureData);
        if (m.normalTexture.textureData != nullptr)           stbi_image_free(m.normalTexture.textureData);
        if (m.roughnessTexture.textureData != nullptr)        stbi_image_free(m.roughnessTexture.textureData);
        if (m.metallicnessTexture.textureData != nullptr)     stbi_image_free(m.metallicnessTexture.textureData);
        if (m.parallaxTexture.textureData != nullptr)         stbi_image_free(m.parallaxTexture.textureData);
        if (m.ambientOcclusionTexture.textureData != nullptr) stbi_image_free(m.ambientOcclusionTexture.textureData);

    }

    delete bvh;
}


void Model::storeModel() {

    for (auto &mat : materials){
        if (mat.diffuseTexture.textureData == nullptr) continue;
        for (size_t i = 0; i < mat.diffuseTexture.textureWidth * mat.diffuseTexture.textureHeight; i++){
            float alpha = (float) (mat.diffuseTexture.textureData[(4 * i) + 3]) / 255.f;
            if (alpha < 1){
                mat.translucent = true;
                mat.opaque = false;
                break;
            }
        }
    }

    bvh = new BVH(meshes);
    glm::mat4x4 I(1.f);

    modelID = renderManager->addNewModel(meshes, materials, bvh, mirror);
//    core::getCollisionManager()->addBVH(bvh, I, modelID);

}


Model::Model(std::vector<Mesh> &meshes, std::vector<Material> &materials, bool mirror) {
    this->meshes = meshes;
    this->materials = materials;
    this->renderManager = core::getRenderManager();
    this->mirror = mirror;
    storeModel();
}

Model::Model(std::vector<Mesh> &meshes, SkeletonRig skeletonRig, std::vector<Material> &materials, bool mirror) {
    this->meshes = meshes;

    for (auto &mesh : this->meshes){
        std::vector<size_t> kill;
        for (size_t i = 0; i < mesh.indices.size(); i += 3){
            glm::vec3 v1 = mesh.vertices[mesh.indices[i + 0]].Position;
            glm::vec3 v2 = mesh.vertices[mesh.indices[i + 1]].Position;
            glm::vec3 v3 = mesh.vertices[mesh.indices[i + 2]].Position;

            glm::vec3 e1 = v2 - v1;
            glm::vec3 e2 = v3 - v1;

            if (isnan(glm::normalize(glm::cross(e1, e2)).x)){
                kill.push_back(i + 0);
                kill.push_back(i + 1);
                kill.push_back(i + 2);
            }
        }
        size_t count = 0;
        for (auto index : kill){
            mesh.indices.erase(mesh.indices.begin() + (index - count++) );
        }

        for (size_t i = 0; i < mesh.indices.size(); i += 3){
            glm::vec3 v1 = mesh.vertices[mesh.indices[i + 0]].Position;
            glm::vec3 v2 = mesh.vertices[mesh.indices[i + 1]].Position;
            glm::vec3 v3 = mesh.vertices[mesh.indices[i + 2]].Position;

            glm::vec3 e1 = v2 - v1;
            glm::vec3 e2 = v3 - v1;

            glm::vec3 norm = glm::normalize(glm::cross(e1, e2));
            assert(!isnan(norm.x));
            assert(!isnan(norm.y));
            assert(!isnan(norm.z));

        }

    }

    this->materials = materials;
    this->renderManager = core::getRenderManager();
    this->skeletonRig = skeletonRig;
    this->mirror = mirror;
    storeModel();
}

std::string Model::getMeshName(size_t meshID) {
    assert(meshID < meshes.size());
    Mesh &mesh = meshes[meshID];
    return materials[mesh.materialIndex].materialName;
}



