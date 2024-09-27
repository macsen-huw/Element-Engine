#define GL_GLEXT_PROTOTYPES

#include "RenderManager.hpp"
#include "core.hpp"
#include "ThreadManager.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cstring>
#include <string>
#include <fstream>
#include <sstream>
#include <random>
#include <bitset>

#include "../external/glm-0.9.7.1/glm/gtc/matrix_transform.hpp"
#include "IntersectionTests.hpp"
#include "CollisionManager.hpp"
#include "core.hpp"
#include "Logger.hpp"
#include "LogType.hpp"

#define GLT_IMPLEMENTATION
#include "gltext.h"

using namespace elmt;

#define MAX_ANIM_BONES 256;


//constructor
//performs basic OpenGL initialisation
//this is all fixed for now, but in the future it should be changed so the the user can specify how the parameters

RenderManager::RenderManager(RenderParameters *suppliedRenderParameters) {


    renderParameters = *suppliedRenderParameters;

    glEnable(GL_FRAMEBUFFER_SRGB);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glfwGetWindowSize(core::getWindow(), &suppliedRenderParameters->screenResolution.x, &suppliedRenderParameters->screenResolution.y);

    glClearColor(renderParameters.backGroundColour.x, renderParameters.backGroundColour.y,
                 renderParameters.backGroundColour.z, renderParameters.backGroundColour.w);

    textureManager = new TextureManager(renderParameters.maxTextureBucketSize);
    shadowManager = new ShadowManager(textureManager, this);
    uiManager = new UIManager();

    depthBufferTextureID = textureManager->createDepthBufferTexture(renderParameters.screenResolution.x, renderParameters.screenResolution.y);
    glGenFramebuffers(1, &depthBufferFrameBufferID);
    glBindFramebuffer(GL_FRAMEBUFFER, depthBufferFrameBufferID);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthBufferTextureID, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenFramebuffers(1, &irradianceTextureFramebuffer);


    char vertexPath[] = "shaders/simple.vert";
    char fragmentPath[] = "shaders/simple.frag";
    mainProgram = setupProgram(vertexPath, fragmentPath);

    char vertexPathPassThrough[] = "shaders/passThrough.vert";
    char fragmentPathPassThrough[] = "shaders/passThrough.frag";
    passThroughProgram = setupProgram(vertexPathPassThrough, fragmentPathPassThrough);

    char vertexPathPlainDraw[] = "shaders/plainDraw.vert";
    char fragmentPathPlainDraw[] = "shaders/plainDraw.frag";
    plainDrawProgram = setupProgram(vertexPathPlainDraw, fragmentPathPlainDraw);

    char vertexPathSkybox[] = "shaders/skybox.vert";
    char fragmentPathSkybox[] = "shaders/skybox.frag";
    skyboxProgram = setupProgram(vertexPathSkybox, fragmentPathSkybox);

    char vertexPathDiffuseIrradiance[] = "shaders/diffuseIrradianceSampler.vert";
    char fragmentPathDiffuseIrradiance[] = "shaders/diffuseIrradianceSampler.frag";
    diffuseIrradianceProgram = setupProgram(vertexPathDiffuseIrradiance, fragmentPathDiffuseIrradiance);

    char vertexPathAmbientOcclusion[] = "shaders/ambientOcclusion.vert";
    char fragmentPathAmbientOcclusion[] = "shaders/ambientOcclusion.frag";
    ambientOcclusionProgram = setupProgram(vertexPathAmbientOcclusion, fragmentPathAmbientOcclusion);

    char vertexPathAmbientOcclusionBlur[] = "shaders/quadPassThrough.vert";
    char fragmentPathAmbientOcclusionBlur[] = "shaders/ambientOcclusionBlur.frag";
    ambientOcclusionBlurProgram = setupProgram(vertexPathAmbientOcclusionBlur, fragmentPathAmbientOcclusionBlur);


    activeProgram = 0;
    glLineWidth(16.f);

    fillPoissonSphere();

    glGenFramebuffers(1, &ambientOcclusionTextureFramebuffer);
    textureManager->loadRenderTexture(&ambientOcclusionTextureID, &ambientOcclusionTextureRenderObject,
                                      ambientOcclusionTextureFramebuffer, renderParameters.screenResolution.x, renderParameters.screenResolution.y);

    glBindFramebuffer(GL_FRAMEBUFFER, ambientOcclusionTextureFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ambientOcclusionTextureID, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    glm::vec3 quadVertices[6] = {
            glm::vec3(-1, -1, 1), glm::vec3(1, -1, 1), glm::vec3(-1, 1, 1),
            glm::vec3(1, 1, 1), glm::vec3(-1, 1, 1), glm::vec3(1, -1, 1)
    };

    GLuint verticesID;
    glGenBuffers(1, &verticesID);
    glBindBuffer(GL_ARRAY_BUFFER, verticesID);

    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &quadVao);
    glBindVertexArray(quadVao);

    //add the vertex and texture coord buffers to it
    glBindBuffer(GL_ARRAY_BUFFER, verticesID);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    gltInit();


}



/*
 * performs setup for a shader program
 */

GLuint RenderManager::setupProgram(char *vertexShader, char *fragmentShader) {

    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    loadShader(vertexShader, VertexShaderID);
    loadShader(fragmentShader, FragmentShaderID);

    Logger::Print("Linking program...", LOGCAT::CAT_LOADING | LOGCAT::CAT_RENDERING, LOGSEV::SEV_INFO);
    GLuint programID = glCreateProgram();

    glAttachShader(programID, VertexShaderID);
    glAttachShader(programID, FragmentShaderID);
    glLinkProgram(programID);

    //check that all the compilation went well
    int InfoLogLength;
    glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &InfoLogLength);

    if (InfoLogLength > 0) {
        std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
        glGetProgramInfoLog(programID, InfoLogLength, nullptr, ProgramErrorMessage.data());
        Logger::Print("Error: " + std::string(ProgramErrorMessage.data()), LOGCAT::CAT_LOADING | LOGCAT::CAT_RENDERING, LOGSEV::SEV_ERROR);
        exit(EXIT_FAILURE);
    }

    return programID;

}


/*
 *  given a shader ID and the path to the shader file the function compiles the shader
 *  reuses code from rasterisation tutorial
 */

void RenderManager::loadShader(char *shaderPath, GLuint shaderID) {
    std::string shaderCode;
    std::ifstream shaderStream(shaderPath, std::ios::in);

    if (!shaderStream.is_open()) {
        Logger::Print("Error opening shader \"" + std::string(shaderPath) + "\"", LOGCAT::CAT_LOADING | LOGCAT::CAT_RENDERING, LOGSEV::SEV_ERROR);
        exit(EXIT_FAILURE);
    }

    std::stringstream sstr;
    sstr << shaderStream.rdbuf();
    shaderCode = sstr.str();
    shaderStream.close();

    Logger::Print("Compiling shader \"" + std::string(shaderPath) + "\"", LOGCAT::CAT_LOADING | LOGCAT::CAT_RENDERING, LOGSEV::SEV_INFO);
    const char *sourcePointer = shaderCode.c_str();
    glShaderSource(shaderID, 1, &sourcePointer, nullptr);
    glCompileShader(shaderID);


    GLint Result = GL_FALSE;
    int InfoLogLength;

    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);

    if (InfoLogLength > 0) {
        std::vector<char> shaderErrorMessage(InfoLogLength + 1);
        glGetShaderInfoLog(shaderID, InfoLogLength, nullptr, shaderErrorMessage.data());
        Logger::Print(shaderErrorMessage.data(), LOGCAT::CAT_LOADING | LOGCAT::CAT_RENDERING, LOGSEV::SEV_ERROR);
        exit(EXIT_FAILURE);
    }
    Logger::Print("Shader loaded successfully", LOGCAT::CAT_LOADING | LOGCAT::CAT_RENDERING, LOGSEV::SEV_INFO);
}





void RenderManager::calculateCubeCoords(float *vertices, float *values, glm::vec3 b1, glm::vec3 b2){
    //coords for the cube that the skybox texture is mapped onto
    float cubeCoords[] = {
            +1.f, +1.f, -1.f, //110
            -1.f, +1.f, +1.f, //011
            -1.f, +1.f, -1.f, //010 --

            +1.f, +1.f, -1.f, //110
            +1.f, +1.f, +1.f, //111 --
            -1.f, +1.f, +1.f, //011

            +1.f, -1.f, +1.f, //101
            -1.f, +1.f, +1.f, //011
            +1.f, +1.f, +1.f, //111 --

            +1.f, -1.f, +1.f, //101
            -1.f, -1.f, +1.f, //001 --
            -1.f, +1.f, +1.f, //011

            -1.f, -1.f, +1.f, //001
            -1.f, +1.f, -1.f, //010
            -1.f, +1.f, +1.f, //011 --

            -1.f, -1.f, +1.f, //001
            -1.f, -1.f, -1.f, //000 --
            -1.f, +1.f, -1.f, //010

            -1.f, -1.f, -1.f, //000
            +1.f, -1.f, +1.f, //101
            +1.f, -1.f, -1.f, //100 --

            -1.f, -1.f, -1.f, //000
            -1.f, -1.f, +1.f, //001 --
            +1.f, -1.f, +1.f, //101

            +1.f, -1.f, -1.f, //100
            +1.f, +1.f, +1.f, //111
            +1.f, +1.f, -1.f, //110 --

            +1.f, -1.f, -1.f, //100
            +1.f, -1.f, +1.f, //101 --
            +1.f, +1.f, +1.f, //111

            -1.f, -1.f, -1.f, //000
            +1.f, +1.f, -1.f, //110
            -1.f, +1.f, -1.f, //010 --

            -1.f, -1.f, -1.f, //000
            +1.f, -1.f, -1.f, //100 --
            +1.f, +1.f, -1.f, //110
    };


    for (int i = 0; i < 36; i++){ //for each vertex
        for (int j = 0; j < 3; j++){ //for each axis
            if (cubeCoords[(i * 3) + j] == -1.f) vertices[(i * 3) + j] = b1[j];
            else vertices[(i * 3) + j] = b2[j];
        }
    }

    if (values == nullptr) return;

    for (int i = 0; i < 12; i++){ //for triangle

        uint32_t bits[3];
        for (int j = 0; j < 3; j++){ //for each vertex
            bits[j] = 0;
            for (int k = 0; k < 3; k++){ //for each axis
                bits[j] |= ((cubeCoords[(i * 9) + (j * 3) + k] == -1.f) << k);
            }
        }

        for (int j = 0; j < 3; j++) {
            std::bitset<32> bitsets[2];
            bitsets[0] = bits[j] ^ bits[(j + 1) % 3];
            bitsets[1] = bits[j] ^ bits[(j + 2) % 3];
            values[(i * 3) + j] = (bitsets[0].count() == 1 && bitsets[1].count() == 1);
        }
    }

}


/*
 * buffers a skybox's data onto the gpu
 */

GLuint RenderManager::createPlainVao(float *coords, size_t coordsSize, float *values) {


    //coords for the cube that the skybox texture is mapped onto


    //we only need to load the vertices, the texture coords are implicit
    GLuint verticesID;
    glGenBuffers(1, &verticesID);
    glBindBuffer(GL_ARRAY_BUFFER, verticesID);

    size_t vertexBufferSize = (sizeof *coords) * coordsSize;
    glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, nullptr, GL_STATIC_DRAW);

    void *verticesPointer = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    memcpy(verticesPointer, coords, vertexBufferSize);
    glUnmapBuffer(GL_ARRAY_BUFFER);

    GLuint valuesID;
    if (values != nullptr) {
        glGenBuffers(1, &valuesID);
        glBindBuffer(GL_ARRAY_BUFFER, valuesID);

        size_t valuesBufferSize = vertexBufferSize / 3;
        glBufferData(GL_ARRAY_BUFFER, valuesBufferSize, nullptr, GL_STATIC_DRAW);

        void *valuesPointer = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        memcpy(valuesPointer, values, valuesBufferSize);
        glUnmapBuffer(GL_ARRAY_BUFFER);
    }


    GLuint arrayID;
    glGenVertexArrays(1, &arrayID);
    glBindVertexArray(arrayID);

    //add the vertex and texture coord buffers to it
    glBindBuffer(GL_ARRAY_BUFFER, verticesID);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    if (values != nullptr) {
        glBindBuffer(GL_ARRAY_BUFFER, valuesID);
        glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(1);
    }

    //unbind everything
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //clean up no longer needed buffers
    glDeleteBuffers(1, &verticesID);
    if (values != nullptr) glDeleteBuffers(1, &valuesID);

    return arrayID;
}

TangentPair RenderManager::calculateTangentPair(glm::vec3 *vertices, glm::vec2 *uvs) {

    glm::vec3 edge1 = vertices[2] - vertices[0];
    glm::vec3 edge2 = vertices[1] - vertices[0];

    glm::vec2 uvEdge1 = uvs[2] - uvs[0];
    glm::vec2 uvEdge2 = uvs[1] - uvs[0];

    float mult = 1.f / (uvEdge1.x * uvEdge2.y - uvEdge2.x * uvEdge1.y);
    TangentPair tangents = {
            (edge1 * uvEdge2.y) - (edge2 * uvEdge1.y),
            (edge2 * uvEdge1.x) - (edge1 * uvEdge2.x),
    };
    tangents.tangent *= mult;
    tangents.biTangent *= mult;

    return tangents;
}


std::vector<TangentPair> RenderManager::calculateNormalMapTangents(Mesh &mesh) {

    std::vector<TangentPair> tangents(mesh.vertices.size());

    for (size_t i = 0; i < mesh.indices.size(); i += 3){
        uint32_t i1 = mesh.indices[i];
        uint32_t i2 = mesh.indices[i + 1];
        uint32_t i3 = mesh.indices[i + 2];

        glm::vec3 vertices[3] = {mesh.vertices[i1].Position, mesh.vertices[i2].Position, mesh.vertices[i3].Position};
        glm::vec2 texCoords[3] = {mesh.vertices[i1].TexCoords, mesh.vertices[i2].TexCoords, mesh.vertices[i3].TexCoords};

        TangentPair tangentPair = calculateTangentPair(vertices, texCoords);

        tangents[i1].tangent += tangentPair.tangent;
        tangents[i1].biTangent += tangentPair.biTangent;

        tangents[i2].tangent += tangentPair.tangent;
        tangents[i2].biTangent += tangentPair.biTangent;

        tangents[i3].tangent += tangentPair.tangent;
        tangents[i3].biTangent += tangentPair.biTangent;
    }

    for (auto &tangent : tangents){
        tangent.tangent = glm::normalize(tangent.tangent);
        tangent.biTangent = glm::normalize(tangent.biTangent);
    }

    return tangents;
}


/*
 * creates all of the VAOs that a mesh has
 * each mesh has 1 vao per material
 * also handles the number of vertices per material, and the material data itself
 */

GLuint RenderManager::createVAO(Mesh* mesh, GLuint *shadowVao) {

    std::vector<TangentPair> tangents = calculateNormalMapTangents(*mesh);

    GLuint VAO, VBO, EBO;
    GLuint vboTangents;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &vboTangents);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, mesh->vertices.size() * sizeof(mesh->vertices[0]), &mesh->vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vboTangents);
    glBufferData(GL_ARRAY_BUFFER, tangents.size() * sizeof(tangents[0]), &tangents[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices.size() * sizeof(mesh->indices[0]), &mesh->indices[0], GL_STATIC_DRAW);


    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);

    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, Normal));

    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, TexCoords));

    // bone ids
    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, BoneIds));
    // weights
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Weights));

    glBindBuffer(GL_ARRAY_BUFFER, vboTangents);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(TangentPair), nullptr);

    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(TangentPair), (void*) offsetof(TangentPair, biTangent));

    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    *shadowVao = createShadowVAO(mesh, VBO, EBO);

    return VAO;
}

GLuint RenderManager::createShadowVAO(Mesh* mesh, GLuint VBO, GLuint EBO) {
    GLuint VAO;
    glGenVertexArrays(1, &VAO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    // vertex texture coords
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

    // bone ids
    glEnableVertexAttribArray(2);
    glVertexAttribIPointer(2, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, BoneIds));

    // weights
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Weights));

    glBindVertexArray(0);

    return VAO;
}

size_t RenderManager::addNewModel(std::vector<Mesh> &meshes, std::vector<Material> &materials, BVH *bvh, bool mirror) {
    if (!core::getThreadManager()->hasRenderContext()) {
        core::getThreadManager()->seizeRenderContext();
    }
    

    //create a new mesh data struct
    LoadedMesh newLoadedMesh{};
    newLoadedMesh.meshID = meshIDs++;
    reloadTextures = true;

    for (Mesh &mesh : meshes) {
        auto &mat = materials[mesh.materialIndex];
        LoadedSubMesh newSubMesh;

        newSubMesh.vaoID = createVAO(&mesh, &newSubMesh.shadowVaoID);
        newSubMesh.numVertices = mesh.indices.size();
        newSubMesh.material = &materials[mesh.materialIndex];

        if (mirror){
            //if this is a mirror material properties don't matter
            newSubMesh.bvh = new BVH(mesh);
            createBvhVaos(newSubMesh.bvhVaoIDs, newSubMesh.numBvhVertices, &newSubMesh.bvh->root);
            newLoadedMesh.subMeshes.push_back(newSubMesh);
            break; //assume only 1 mesh in mirror...
        }

        if (mat.diffuseTexture.textureInfo.bucketID == 0) {
            mat.diffuseTexture.textureInfo = textureManager->loadTexture(mat.diffuseTexture);
        }
        newSubMesh.surfaceTextureInfo = mat.diffuseTexture.textureInfo;

        if (mat.normalTexture.textureData != nullptr) {
            if (mat.normalTexture.textureInfo.bucketID == 0) {
                mat.normalTexture.textureInfo = textureManager->loadTexture(mat.normalTexture);
            }
            newSubMesh.normalTextureInfo = mat.normalTexture.textureInfo;
        }

        if (mat.roughnessTexture.textureData != nullptr) {
            if (mat.roughnessTexture.textureInfo.bucketID == 0) {
                mat.roughnessTexture.textureInfo = textureManager->loadTexture(mat.roughnessTexture);
            }
            newSubMesh.roughnessTextureInfo = mat.roughnessTexture.textureInfo;
        }

        if (mat.metallicnessTexture.textureData != nullptr) {
            if (mat.metallicnessTexture.textureInfo.bucketID == 0) {
                mat.metallicnessTexture.textureInfo = textureManager->loadTexture(mat.metallicnessTexture);
            }
            newSubMesh.metallicnessTextureInfo = mat.metallicnessTexture.textureInfo;
        }

        if (mat.parallaxTexture.textureData != nullptr) {
            if (mat.parallaxTexture.textureInfo.bucketID == 0) {
                mat.parallaxTexture.textureInfo = textureManager->loadTexture(mat.parallaxTexture);
            }
            newSubMesh.parallaxTextureInfo = mat.parallaxTexture.textureInfo;
        }

        if (mat.ambientOcclusionTexture.textureData != nullptr) {
            if (mat.ambientOcclusionTexture.textureInfo.bucketID == 0) {
                mat.ambientOcclusionTexture.textureInfo = textureManager->loadTexture(mat.ambientOcclusionTexture);
            }
            newSubMesh.ambientOcclusionTextureInfo = mat.ambientOcclusionTexture.textureInfo;
        }

        newSubMesh.bvh = new BVH(mesh);
        createBvhVaos(newSubMesh.bvhVaoIDs, newSubMesh.numBvhVertices, &newSubMesh.bvh->root);
        newLoadedMesh.subMeshes.push_back(newSubMesh);

        for (size_t i = 0; i < mesh.vertices.size(); i++){            
            glm::vec3 vertex = mesh.vertices[i].Position;
            assert(vertex.x >= newSubMesh.bvh->root.b1.x);
            assert(vertex.y >= newSubMesh.bvh->root.b1.y);
            assert(vertex.z >= newSubMesh.bvh->root.b1.z);

            assert(vertex.x <= newSubMesh.bvh->root.b2.x);
            assert(vertex.y <= newSubMesh.bvh->root.b2.y);
            assert(vertex.z <= newSubMesh.bvh->root.b2.z);

        }

    }

    newLoadedMesh.bvh = bvh;
    createBvhVaos(newLoadedMesh.bvhVaoIDs, newLoadedMesh.numBvhVertices, &bvh->root);
    reloadTextures = true;

    //and add it to the loaded mesh's set
    if (mirror) loadedMirrors.insert(newLoadedMesh);
    else loadedMeshes.insert(newLoadedMesh);
    return newLoadedMesh.meshID;
}

void RenderManager::createBvhVaos(std::vector<GLuint> &vaos, std::vector<GLsizei> &vertices, AABB *root, size_t depth){
    std::vector<AABB *> queues[2];
    size_t activeQueue = 0;

    queues[activeQueue].push_back(root);
    size_t verticesPerBox = 36;

    size_t i = 0;
    while (!queues[activeQueue].empty() && i++ < depth) {

        size_t numBoxes = queues[activeQueue].size();
        std::vector<float> cubeCoords(numBoxes * verticesPerBox * 3); //3 floats (x, y, z) per vertex
        std::vector<float> cubeValues(numBoxes * verticesPerBox); //1 float (0 or 1) per vertex

        size_t index = 0;
        while (index < numBoxes) {
            AABB *box = queues[activeQueue][index];
            if (box->c1 != nullptr) queues[!activeQueue].push_back(box->c1);
            if (box->c2 != nullptr) queues[!activeQueue].push_back(box->c2);

            calculateCubeCoords(&cubeCoords[index * verticesPerBox * 3], &cubeValues[index * verticesPerBox], box->b1, box->b2);
            ++index;
        }

        vaos.push_back(createPlainVao(cubeCoords.data(), numBoxes * verticesPerBox * 3, cubeValues.data()));
        vertices.push_back(numBoxes);
        queues[activeQueue].clear();
        activeQueue = !activeQueue;
    }

}


/*
 * adds a mirror mesh
 */
//size_t RenderManager::addMirrorMesh(OldMesh *mesh, BVH *bvh) {
//    //create a new mesh data struct
//    MeshData newMirror{};
//    newMirror.meshID = meshIDs++;
//    newMirror.bvh = bvh;
//    newMirror.bvhVao = createSkyBoxVao(bvh->root.b1, bvh->root.b2);
//
//    createVAOs(mesh, &newMirror);
//    newMirror.origin = mesh->vertices[0];
//
//    //and add it to the loaded mesh's set
//    loadedMirrors.insert(newMirror);
//
//    return newMirror.meshID;
//}



/*
 * updates a mesh
 * it reloads the mesh data onto the gpu and internally gives it a new vbo
 * but it keeps all the instances of the old mesh and the old mesh ID
 */

//void RenderManager::updateMesh(size_t meshID, OldMesh*mesh) {
//
//    auto meshData = findMesh(meshID);
//    glDeleteVertexArrays(meshData->vaoIDs.size(), meshData->vaoIDs.data());
//    meshData->vaoIDs.clear();
//    meshData->numVertices.clear();
//    meshData->materials.clear();
//
//    createVAOs(mesh, &(*meshData));
//}


/*
 * sets the active skybox
 */
void RenderManager::setSkybox(Skybox *skybox) {
    glm::vec3 b1 = glm::vec3(-1, -1, -1) * skybox->scale;
    glm::vec3 b2 = glm::vec3(1, 1, 1) * skybox->scale;

    float cubeCoords[108];
    calculateCubeCoords(cubeCoords, nullptr, b1, b2);

    skyboxTextureID = textureManager->loadSkyBoxTexture(skybox);
    GLuint skyboxIrradianceTextureID = textureManager->loadSkyBoxIrradianceData(skybox);
    skyboxVao = createPlainVao(cubeCoords, 108, nullptr);

    textureManager->loadCubeMapRenderTexture(&irradianceTextureID, &irradianceTextureRenderObject, irradianceTextureFramebuffer, 32, 32);

    glm::mat4x4 irradianceProjectionMatrix = glm::perspective(glm::radians(90.f), 1.f, 0.1f, 2.f * skybox->scale);
    glm::vec3 viewPoints[6] = {
            glm::vec3(1, 0, 0), glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0),
            glm::vec3(0, -1, 0), glm::vec3(0, 0, 1), glm::vec3(0, 0, -1)
    };
    glm::vec3 upDirections[6] = {
            glm::vec3(0, -1, 0), glm::vec3(0, -1, 0), glm::vec3(0, 0, 1),
            glm::vec3(0, 0, -1), glm::vec3(0, -1, 0), glm::vec3(0, -1, 0)
    };

    glDisable(GL_CULL_FACE);

    glViewport(0, 0, 32, 32);
    glUseProgram(diffuseIrradianceProgram);

    GLint numSamplesID = glGetUniformLocation(diffuseIrradianceProgram, "numSamples");
    glUniform1i(numSamplesID, 64);

    glBindVertexArray(skyboxVao);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxIrradianceTextureID);
    glBindFramebuffer(GL_FRAMEBUFFER, irradianceTextureFramebuffer);

    for (int i = 0; i < 6; i++){
        glm::mat4x4 view = glm::lookAt(glm::vec3(0, 0, 0), viewPoints[i], upDirections[i]);
        glm::mat4x4 transformationMatrix = irradianceProjectionMatrix * view;
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceTextureID, 0);

        GLint transformationMatrixID = glGetUniformLocation(diffuseIrradianceProgram, "transformationMatrix");
        glUniformMatrix4fv(transformationMatrixID, 1, GL_FALSE, &transformationMatrix[0][0]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ++drawCalls;
        glDrawArrays(GL_TRIANGLES, 0, 48);
    }


    //use a projection matrix with a large enough far plane for rendering the skybox
    skyboxProjection = glm::perspective(glm::radians(45.f), 4.0f / 3.0f, 0.1f, 2.f * skybox->scale);
    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_TEXTURE_CUBE_MAP, 0);
    glEnable(GL_CULL_FACE);

}


/*
 * creates a new instance for a given mesh (referenced by its ID)
 * initialises instance matrices to identity matrix
 */
size_t RenderManager::addInstance(const std::string &name, size_t meshID, float minLOD, float maxLOD) {
    //search the mesh set for the given meshID
    LoadedMesh searchMesh{};
    searchMesh.meshID = meshID;
    auto search = loadedMeshes.find(searchMesh);

    if (search == loadedMeshes.end()) { //mesh not loaded in loadedMeshes

        //check the mirrors
        search = loadedMirrors.find(searchMesh);
        if (search == loadedMirrors.end()) {
            Logger::Print("Error - meshID \"%lu\" is not loaded\n" + std::to_string(meshID), LOGCAT::CAT_RENDERING, LOGSEV::SEV_ERROR);
            exit(EXIT_FAILURE);
        }
    }

    //create a new instance
    LoadedInstance newInstance{};
    newInstance.instanceName = name;
    newInstance.instanceID = instanceIDs++;

    //initialise its properties
    InstanceProperties properties{};
    newInstance.properties = properties;
    newInstance.properties.minLOD = minLOD;
    newInstance.properties.maxLOD = maxLOD;


    search->instances.insert(newInstance);
    return newInstance.instanceID;
}



std::vector<size_t> RenderManager::addInstance(const std::string &name, std::vector<size_t> &meshIDs, std::vector<float> &lodPartitions){


    if (lodPartitions.size() != meshIDs.size()){
        Logger::Print("Error - trying to create an instance with " + std::to_string(meshIDs.size()) + " mesh's, but"
                       + std::to_string(lodPartitions.size()) + "LOD partitions\n", LOGCAT::CAT_RENDERING, LOGSEV::SEV_ERROR);

        return std::vector<size_t>{};
    }

    for (size_t i = 0; i < lodPartitions.size(); i++){
        if (lodPartitions[i] < 0 || lodPartitions[i] > 1){
            Logger::Print("Error - attempting to create instance with out of bounds LOD partition (partition "
                           + std::to_string(i) + " = " + std::to_string(lodPartitions[i]) + ")",
                          LOGCAT::CAT_RENDERING, LOGSEV::SEV_ERROR);
        }
    }

    std::vector<size_t> instances;

    for (size_t i = 0; i < meshIDs.size(); i++){
        size_t meshID = meshIDs[i];
        float prevLOD = (i == 0) ? 0 : lodPartitions[i - 1];
        size_t ID = addInstance(name, meshID, prevLOD, lodPartitions[i]);
        instances.push_back(ID);
    }

    return instances;
}





/*
 * updates a given instance with the provided properties
 */
void RenderManager::updateInstance(InstanceProperties &newProperties, size_t meshID, size_t instanceID) {

    //try and find mesh and instance
    auto mesh = findMesh(meshID);
    auto instance = findInstance(mesh, instanceID);

    float min = instance->properties.minLOD;
    float max = instance->properties.maxLOD;

    //all good - update instance properties
    instance->properties = newProperties;
    instance->properties.minLOD = min;
    instance->properties.maxLOD = max;
}

/*
 * searches the loadedMeshes set for a mesh with the given ID
 */

std::set<LoadedMesh>::const_iterator RenderManager::findMesh(size_t meshID) {
    LoadedMesh searchMesh{};
    searchMesh.meshID = meshID;
    auto search = loadedMeshes.find(searchMesh);

    if (search == loadedMeshes.end()) {

        search = loadedMirrors.find(searchMesh);

        if (search == loadedMirrors.end()) {
            Logger::Print("Error - mirror with meshID \"%lu\" is not loaded\n" + std::to_string(meshID), LOGCAT::CAT_RENDERING, LOGSEV::SEV_ERROR);
            exit(EXIT_FAILURE);
        }
    }

    return search;
}

/*
 * searches the loadedMeshes set for a mesh with the given ID,
 * then searches its instances for an instance with the given ID
 */

std::set<LoadedInstance>::iterator RenderManager::findInstance(std::set<LoadedMesh>::const_iterator mesh, size_t instanceID) {
    LoadedInstance searchInstance{};
    searchInstance.instanceID = instanceID;
    auto search = mesh->instances.find(searchInstance);

    if (search == mesh->instances.end()) {
        Logger::Print("Error - instanceID \"" + std::to_string(instanceID) + "\" for mesh \""
                + std::to_string(mesh->meshID) + "\" is not registered\n", LOGCAT::CAT_RENDERING, LOGSEV::SEV_ERROR);
        exit(EXIT_FAILURE);
    }
    return search;
}

/*
 * deletes a given instance
 */

void RenderManager::deleteInstance(size_t meshID, size_t instanceID) {
    auto mesh = findMesh(meshID);
    auto instance = findInstance(mesh, instanceID);

    mesh->instances.erase(instance);
}

/*
 * deletes a given mesh, and all of its instances
 */

void RenderManager::deleteMesh(size_t meshID) {
//    auto mesh = findMesh(meshID);
//    mesh->instances.clear();
//
//    for (GLuint vao : mesh->vaoIDs) {
//        glDeleteVertexArrays(1, &vao);
//    }
//
//    for (TextureInfo info : mesh->textureBuckets){
//        textureManager->deleteTexture(info);
//    }
//
//    loadedMeshes.erase(mesh);

}

/*
 * adds a directional light to the directionalLights set
 */
size_t RenderManager::addDirectionalLight(DirectionalLight* directionalLight) {
    if (directionalLights.size() >= maxBasicLights){
        Logger::Print("Error - cannot add another directional light, maximum light capacity has been reached (%lu)" +
                       std::to_string(maxBasicLights) + "\n", LOGCAT::CAT_RENDERING, LOGSEV::SEV_ERROR);
        return -1;
    }

    DirectionalLightData newData{
        newData.light = directionalLight,
        newData.ID = directionalLightIDs++
    };

    directionalLights.insert(newData);
    return newData.ID;
}

/*
 * adds a point light to the pointLights set
 */
size_t RenderManager::addPointLight(PointLight* pointLight) {
    if (pointLights.size() >= maxBasicLights){
        Logger::Print("Error - cannot add another point light, maximum light capacity has been reached (%lu)" +
                      std::to_string(maxBasicLights) + "\n", LOGCAT::CAT_RENDERING, LOGSEV::SEV_ERROR);
        return -1;
    }

    PointLightData newData{
            newData.light = pointLight,
            newData.ID = pointLightIDs++
    };

    pointLights.insert(newData);
    return newData.ID;
}


void RenderManager::fillPoissonSphere() {

    std::random_device randomDevice;
    std::mt19937 generator(randomDevice());


    std::uniform_real_distribution<float> randomDistribution(-1.f, 1.f);

    size_t count = 0;
    size_t attempts = 0;
    float minDistance = 0.30;

    while (count < renderParameters.numAmbientOcclusionSamples){

        reject_sample:
        attempts++;

        if (attempts > (renderParameters.numAmbientOcclusionSamples * renderParameters.numAmbientOcclusionSamples) + 1000){
            //we've been trying this too long, safe to assume we have impossible conditions (too many samples / not enough space)
            Logger::Print("Error - unable to fill ambient occlusion poisson sphere in a reasonable time, so decreasing min spacing\n",
                          LOGCAT::CAT_RENDERING, LOGSEV::SEV_WARNING);
            attempts = 0;
            minDistance -= 0.04;
            count = 0;
            poissonSphere.clear();
            continue;
        }

        glm::vec4 newSample;
        newSample.x = randomDistribution(generator);
        newSample.y = (1.f + randomDistribution(generator)) / 2.f; //in range [0, 1]
        newSample.y = acos(sqrt(newSample.y)) / (0.5 * 3.14159265);
        newSample.z = randomDistribution(generator);
        newSample.w = 0;

        //check if generated sample is outside of the sphere
        if (glm::length(newSample) > 1){
            continue;
        }


        //check if generated sample is too close to an existing one
        for (auto sample : poissonSphere){
            if (glm::distance(sample, newSample) < minDistance){
                goto reject_sample;
            }
        }

        poissonSphere.push_back(newSample);
        ++count;
    }

}


/*
 * copies the point and directional lights into a vector in the data layout that the shaders expect
 */

void RenderManager::fillLightVectors() {

    directionalLightsVector.clear();
    pointLightsVector.clear();

    for (auto const& light : directionalLights){
        glm::vec4 dir = {light.light->direction.x, light.light->direction.y, light.light->direction.z, 1};
        glm::vec4 colour = {light.light->colour.x, light.light->colour.y, light.light->colour.z, light.light->intensity};
        directionalLightsVector.push_back({dir, colour});
    }

    for (auto const& light : pointLights){
        glm::vec4 pos = {light.light->pos.x, light.light->pos.y, light.light->pos.z, 1};
        glm::vec4 colour = {light.light->colour.x, light.light->colour.y, light.light->colour.z, light.light->intensity};
        pointLightsVector.push_back({pos, colour});
    }
}


/*
 * loads the uniform data that changes per render pass
 */
void RenderManager::passRenderUniforms() {
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, ambientOcclusionTextureID);
}



/*
 * loads the uniform data that changes per mesh, currently none
 */

void RenderManager::passMeshUniforms(const LoadedMesh &mesh, const LoadedInstance &instance, size_t i) const {

}


/*
 * loads the uniform data that changes per instance
 */

void RenderManager::passInstanceUniforms(const LoadedMesh &mesh, const LoadedInstance &instance, glm::mat4x4 &transform, bool mirror) const {

    glm::mat4x4 transformationMatrix = cameraMatrix * transform * instance.properties.translationMatrix * instance.properties.rotationMatrix;
    GLint transformationMatrixID = glGetUniformLocation(mainProgram, "transformationMatrix");
    glUniformMatrix4fv(transformationMatrixID, 1, GL_FALSE, &transformationMatrix[0][0]);

    glm::vec3 scale = glm::vec3(10.f, 10.f, 10.f);
    glm::mat4x4 modelMatrix = transform * instance.properties.translationMatrix * instance.properties.rotationMatrix;
    GLint modelMatrixID = glGetUniformLocation(mainProgram, "modelMatrix");
    glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &modelMatrix[0][0]);

    GLint cameraID = glGetUniformLocation(mainProgram, "cameraMatrix");
    glUniformMatrix4fv(cameraID, 1, GL_FALSE, &cameraMatrix[0][0]);

    GLint rotationID = glGetUniformLocation(mainProgram, "rotationMatrix");
    glUniformMatrix4fv(rotationID, 1, GL_FALSE, &instance.properties.rotationMatrix[0][0]);

    GLint instanceID = glGetUniformLocation(mainProgram, "instanceMatrix");
    glUniformMatrix4fv(instanceID, 1, GL_FALSE, &transform[0][0]);

    GLint useMirrorID = glGetUniformLocation(mainProgram, "useMirror");
    glUniform1i(useMirrorID, (GLint) mirror);


    GLint lodAlphaBlendID = glGetUniformLocation(mainProgram, "lodAlphaBlend");
    glUniform1f(lodAlphaBlendID, lodAlphaBlend);


    GLint boneCountID = glGetUniformLocation(mainProgram, "boneCount");
    glUniform1i(boneCountID, instance.properties.animBoneMatrices.size());

    if (instance.properties.animBoneMatrices.size() > 0) {

        GLuint boneMatrixBufferID;
        glGenBuffers(1, &boneMatrixBufferID);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, boneMatrixBufferID);
        size_t boneMatricesSize = instance.properties.animBoneMatrices.size() * sizeof(instance.properties.animBoneMatrices[0]);

        glBufferData(GL_SHADER_STORAGE_BUFFER, boneMatricesSize, instance.properties.animBoneMatrices.data(), GL_STATIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, boneMatrixBufferID);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    if (mirror) {
        glm::vec4 orientation = glm::vec4(mesh.orientation, 1.f);
        GLint mirrorNormalID = glGetUniformLocation(mainProgram, "mirrorNormal");
        glUniform3fv(mirrorNormalID, 1, &orientation[0]);

        glm::vec4 pos = glm::vec4(mesh.origin, 1.f);
        pos = instance.properties.translationMatrix * instance.properties.rotationMatrix * pos;
        GLint mirrorPointID = glGetUniformLocation(mainProgram, "mirrorPoint");
        glUniform3fv(mirrorPointID, 1, &pos[0]);
    }

}

/*
 * loads the uniform data that changes per material
 */
void RenderManager::passMaterialUniforms(const LoadedSubMesh &mesh) const {
    //and the blinn phong uniforms
    GLint diffuseID = glGetUniformLocation(mainProgram, "diffuseColour");
    glUniform3fv(diffuseID, 1, &mesh.material->diffuse[0]);

    GLint specularID = glGetUniformLocation(mainProgram, "specularColour");
    glUniform3fv(specularID, 1, &mesh.material->specular[0]);

    GLint ambientID = glGetUniformLocation(mainProgram, "ambientColour");
    glUniform3fv(ambientID, 1, &mesh.material->ambient[0]);

    GLint emissiveID = glGetUniformLocation(mainProgram, "emissiveColour");
    glUniform3fv(emissiveID, 1, &mesh.material->emissive[0]);

    GLint shininessID = glGetUniformLocation(mainProgram, "shininess");
    glUniform1f(shininessID, mesh.material->shininess);


    //and the texture settings

    if (mesh.material->diffuseTexture.textureData != nullptr){
        GLint useTextureID = glGetUniformLocation(mainProgram, "useTexture");
        glUniform1i(useTextureID, GL_TRUE);

        GLint levelID = glGetUniformLocation(mainProgram, "textureLevel");
        glUniform1f(levelID, (float) mesh.surfaceTextureInfo.level);

        glActiveTexture(GL_TEXTURE2);
        GLuint textureID = textureManager->getTextureID(mesh.surfaceTextureInfo.bucketID);
        glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);
    }

    else{
        GLint useTextureID = glGetUniformLocation(mainProgram, "useTexture");
        glUniform1i(useTextureID, GL_FALSE);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    }

    if (mesh.material->normalTexture.textureData != nullptr){
        GLint levelID = glGetUniformLocation(mainProgram, "normalTextureLevel");
        glUniform1f(levelID, (float) mesh.normalTextureInfo.level);

        GLint useID = glGetUniformLocation(mainProgram, "useNormalMap");
        glUniform1i(useID, GL_TRUE);

        glActiveTexture(GL_TEXTURE8);
        GLuint textureID = textureManager->getTextureID(mesh.normalTextureInfo.bucketID);
        glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);
    }

    else{
        GLint useID = glGetUniformLocation(mainProgram, "useNormalMap");
        glUniform1i(useID, GL_FALSE);
    }

    if (mesh.material->parallaxTexture.textureData != nullptr && parallaxEnabled){
        GLint levelID = glGetUniformLocation(mainProgram, "parallaxTextureLevel");
        glUniform1f(levelID, (float) mesh.parallaxTextureInfo.level);

        GLint useID = glGetUniformLocation(mainProgram, "useParallaxMap");
        glUniform1i(useID, GL_TRUE);

        glActiveTexture(GL_TEXTURE11);
        GLuint textureID = textureManager->getTextureID(mesh.parallaxTextureInfo.bucketID);
        glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);
    }

    else{
        GLint useID = glGetUniformLocation(mainProgram, "useParallaxMap");
        glUniform1i(useID, GL_FALSE);
    }


    //a material needs a roughness and metallicnes texture for cook torrance, so only bind if we have both
    if (mesh.material->roughnessTexture.textureData != nullptr && mesh.material->metallicnessTexture.textureData != nullptr){
        GLint levelID = glGetUniformLocation(mainProgram, "roughnessTextureLevel");
        glUniform1f(levelID, (float) mesh.roughnessTextureInfo.level);

        glActiveTexture(GL_TEXTURE9);
        GLuint textureID = textureManager->getTextureID(mesh.roughnessTextureInfo.bucketID);
        glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);

        levelID = glGetUniformLocation(mainProgram, "metallicnessTextureLevel");
        glUniform1f(levelID, (float) mesh.metallicnessTextureInfo.level);

        glActiveTexture(GL_TEXTURE10);
        textureID = textureManager->getTextureID(mesh.metallicnessTextureInfo.bucketID);
        glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);

        GLint brdfID = glGetUniformLocation(mainProgram, "brdf");
        glUniform1i(brdfID, 1);
    }
    else{
        GLint brdfID = glGetUniformLocation(mainProgram, "brdf");
        glUniform1i(brdfID, 0);
    }


    if (mesh.material->ambientOcclusionTexture.textureData != nullptr){
        GLint levelID = glGetUniformLocation(mainProgram, "ambientOcclusionTextureLevel");
        glUniform1f(levelID, (float) mesh.ambientOcclusionTextureInfo.level);

        GLint useID = glGetUniformLocation(mainProgram, "useAmbientOcclusionTexture");
        glUniform1i(useID, GL_TRUE);

        glActiveTexture(GL_TEXTURE12);
        GLuint textureID = textureManager->getTextureID(mesh.ambientOcclusionTextureInfo.bucketID);
        glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);
    }

    else{
        GLint useID = glGetUniformLocation(mainProgram, "useAmbientOcclusionTexture");
        glUniform1i(useID, GL_FALSE);
    }
}

/*
 * passes all uniforms that do not change within a frame
 */

void RenderManager::passFrameUniforms(glm::mat4x4 &viewMatrix, glm::vec3 cameraPos){


    //first pass the single object uniforms
    GLint viewID = glGetUniformLocation(mainProgram, "viewMatrix");
    glUniformMatrix4fv(viewID, 1, GL_FALSE, &viewMatrix[0][0]);

    GLint cameraPosID = glGetUniformLocation(mainProgram, "cameraPos");
    glUniform3fv(cameraPosID, 1, &cameraPos[0]);

    GLint numCascadePartitionsID = glGetUniformLocation(mainProgram, "numCascadePartitions");
    glUniform1i(numCascadePartitionsID, renderParameters.cascadePartitions.size() + 1);

    GLint poissonDiscID = glGetUniformLocation(mainProgram, "poissonDisc");
    glUniform2fv(poissonDiscID, renderParameters.shadowPCFNumSamples, &shadowManager->poissonDisc[0][0]);

    GLint poissonSphereID = glGetUniformLocation(mainProgram, "poissonSphere");
    glUniform4fv(poissonSphereID, renderParameters.numAmbientOcclusionSamples, &poissonSphere[0][0]);

    GLint ambientIntensityID = glGetUniformLocation(mainProgram, "ambientIntensity");
    glUniform1f(ambientIntensityID, renderParameters.ambientLightIntensity);

    GLint pcfRadiusID = glGetUniformLocation(mainProgram, "pcfRadius");
    glUniform1f(pcfRadiusID, renderParameters.pcfRadius);

    GLint renderModeID = glGetUniformLocation(mainProgram, "renderMode");
    glUniform1i(renderModeID, renderMode);

    GLint parallaxHeightID = glGetUniformLocation(mainProgram, "parallaxHeight");
    glUniform1f(parallaxHeightID, renderParameters.parallaxHeight);

    GLint parallaxAccuracyID = glGetUniformLocation(mainProgram, "parallaxAccuracy");
    glUniform1f(parallaxAccuracyID, renderParameters.parallaxOcclusionAccuracy);

    GLint screenSizeID = glGetUniformLocation(mainProgram, "screenSize");
    glUniform2iv(screenSizeID, 1, &renderParameters.screenResolution[0]);

    GLint useSkyboxID = glGetUniformLocation(mainProgram, "useSkybox");
    glUniform1i(useSkyboxID, skyboxVao != 0);

    // bone weights
    GLint boneId = glGetUniformLocation(mainProgram, "renderBoneId");
    glUniform1i(boneId, renderBoneId);



    //then the buffer uniform objects, all std140 layout for now
    const GLuint directionalLightsBinding = 0;
    const GLuint pointLightsBinding = 1;
    const GLuint directionalLightTransformsBinding = 4;
    const GLuint shadowCascadeDepthBinding = 5;

    //add the uniform buffer objects for the directional lights
    GLint directionalLightsID = glGetUniformLocation(mainProgram, "numDirectionalLights");
    glUniform1i(directionalLightsID, directionalLightsVector.size());


    GLuint directionalLightBuffer;
    glGenBuffers(1, &directionalLightBuffer);

    GLsizeiptr dirLightSize = directionalLightsVector.size() * sizeof(DirectionalLightGPUObject);
    glBindBuffer(GL_UNIFORM_BUFFER, directionalLightBuffer);
    glBufferData(GL_UNIFORM_BUFFER, dirLightSize, nullptr, GL_STATIC_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, directionalLightsBinding, directionalLightBuffer, 0, dirLightSize);
    glBindBuffer(GL_UNIFORM_BUFFER, directionalLightBuffer);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, dirLightSize, directionalLightsVector.data());

    //and the point lights...
    GLint pointLightsID = glGetUniformLocation(mainProgram, "numPointLights");
    glUniform1i(pointLightsID, pointLightsVector.size());

    GLuint pointLightBuffer;
    glGenBuffers(1, &pointLightBuffer);
    GLsizeiptr pointLightSize = pointLightsVector.size() * sizeof(PointLightGPUObject);

    if (pointLightSize > 0) {
        glBindBuffer(GL_UNIFORM_BUFFER, pointLightBuffer);
        glBufferData(GL_UNIFORM_BUFFER, pointLightSize, nullptr, GL_STATIC_DRAW);

        glBindBufferRange(GL_UNIFORM_BUFFER, pointLightsBinding, pointLightBuffer, 0, pointLightSize);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, pointLightSize, pointLightsVector.data());
    }

    //and the directional light transformation matrices for shadow computation
    GLuint directionalLightTransformsBuffer;
    glGenBuffers(1, &directionalLightTransformsBuffer);

    GLsizeiptr dirLightTransformsSize = shadowManager->lightTransforms.size() * sizeof(glm::mat4x4);
    glBindBuffer(GL_UNIFORM_BUFFER, directionalLightTransformsBuffer);
    glBufferData(GL_UNIFORM_BUFFER, dirLightTransformsSize, nullptr, GL_STATIC_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, directionalLightTransformsBinding, directionalLightTransformsBuffer, 0, dirLightTransformsSize);


    glBindBuffer(GL_UNIFORM_BUFFER, directionalLightTransformsBuffer);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, dirLightTransformsSize, shadowManager->lightTransforms.data());

    //the floats that represent the view space depths of the frustum partition boundaries need converting to vec4s to store on the gpu
    //1 float per vec4 is wasteful, and needs improving in the future
    std::vector<glm::vec4> alignedViewDepths;
    for (auto f : shadowManager->viewSpaceDepths){
        alignedViewDepths.emplace_back(f, 0, 0, 0);
    }

    //and the depths of the shadow cascades
    GLuint shadowCascadeDepthBuffer;
    glGenBuffers(1, &shadowCascadeDepthBuffer);

    GLsizeiptr cascadeBufferSize = alignedViewDepths.size() * sizeof(glm::vec4);
    glBindBuffer(GL_UNIFORM_BUFFER, shadowCascadeDepthBuffer);
    glBufferData(GL_UNIFORM_BUFFER, cascadeBufferSize, nullptr, GL_STATIC_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, shadowCascadeDepthBinding, shadowCascadeDepthBuffer, 0, cascadeBufferSize);

    glBindBuffer(GL_UNIFORM_BUFFER, shadowCascadeDepthBuffer);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, cascadeBufferSize, alignedViewDepths.data());


    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceTextureID);


}

/*
 * calculates a matrix to transform a vector from R3 to the planar coordinate system with the normal as the up vector
 */

glm::mat3x3 RenderManager::calculatePlanarBasis(glm::vec3 normal) {
    normal = glm::normalize(normal);
    glm::vec3 temp(1, 1, 1);
    temp = glm::normalize(temp);

    if (fabs(glm::dot(normal, temp)) > 0.99){
        temp = glm::normalize(glm::vec3(1, 1, 0));
    }

    glm::vec3 v1 = glm::cross(normal, temp);
    glm::vec3 v2 = glm::cross(normal, v1);

    glm::mat3x3 planarBasis;
    planarBasis[0] = glm::normalize(v1);
    planarBasis[1] = glm::normalize(v2);
    planarBasis[2] = glm::normalize (normal);

    return planarBasis;

}



/*
 * calculates the reflection matrix used to create a mirror instance of an object
 */

glm::mat4x4 RenderManager::calculateReflectionMatrix(glm::vec3 normal, glm::vec3 planarOrigin) {

    glm::mat3x3 planarBasis3 = calculatePlanarBasis(normal);
    glm::mat4x4 planarBasis(1.f);
    for (int i = 0; i < 3; i++){
        for (int j = 0; j < 3; j++){
            planarBasis[i][j] = planarBasis3[i][j];
        }
    }


    glm::mat4x4 mirrorZ(1.f);
    mirrorZ[2].z = -1.f;

    glm::mat4x4 transMatrix(1.f);
    for (int i = 0; i < 3; i++){
        transMatrix[3][i] = -planarOrigin[i]; //opengl is column major!
    }

    return glm::inverse(transMatrix) * glm::inverse(planarBasis) * mirrorZ * planarBasis * transMatrix;
}


/*
 * renders the mirrors in the scene
 *
 * it first renders the mirror's triangles, and during this render pass it fills the stencil buffer in the fragment
 * positions that the triangles are present - this essentially tags what pixels are covered by the mirror
 *
 * it then renders the scene using the reflection matrix, and discards any fragments not tagged by the stencil buffer
 *
 * rendering multiple mirrors that reflect onto each other is not implemented
 */


void RenderManager::renderMirrors(glm::mat4x4 &viewMatrix) {

    //clear all the buffers
    glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (const LoadedMesh &mirror : loadedMirrors){
        for (const LoadedInstance &instance: mirror.instances){

            if (!instance.properties.active) continue;


            glm::mat4x4 trans = instance.properties.translationMatrix * instance.properties.rotationMatrix;
            if (!viewFrustumTest(&mirror.bvh->root, trans, cameraMatrix)) continue; //mirror not in view frustum, we can skip it

            glm::vec4 mirrorOrigin = instance.properties.translationMatrix * instance.properties.rotationMatrix * glm::vec4(mirror.origin, 1.f);

            glm::vec4 mirrorOrientation = instance.properties.rotationMatrix * glm::vec4(mirror.orientation, 1.f);
            glm::mat4x4 reflectionMatrix = calculateReflectionMatrix(glm::vec3(mirrorOrientation), glm::vec3(mirrorOrigin));

            glCullFace(GL_FRONT);
            glm::mat4x4 projViewMatrix = cameraMatrix * reflectionMatrix;
            glViewport(0, 0, renderParameters.screenResolution.x, renderParameters.screenResolution.y);
            shadowRender(projViewMatrix, glm::mat4x4(1.f), depthBufferFrameBufferID, false);
            ambientOcclusionRender(reflectionMatrix, cameraMatrix);
            glCullFace(GL_BACK);

            //we need to calculate shadows for the objects in the mirror world
            shadowManager->renderShadows(reflectionMatrix);

            //set stencil buffer to mark the fragments covered by the mirror to 1
            glEnable(GL_STENCIL_TEST);
            glStencilFunc(GL_ALWAYS, 1, 0xFF);
            glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
            glStencilMask(0xFF);

            //render the mirror's geometry to set the stencil buffer
            renderPass(false, loadedMirrors);

            //disable writing to the stencil buffer
            glStencilFunc(GL_EQUAL, 1, 0xFF);
            glStencilOp(GL_ZERO, GL_KEEP, GL_KEEP);

            //prepare state for rendering mirror world
            glCullFace(GL_FRONT);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            //render the mirror world
            renderPass(false, loadedMeshes, reflectionMatrix, true);
            renderPass(true, loadedMeshes, reflectionMatrix, true);

            glCullFace(GL_BACK);

            //and now the mirror skybox
            glm::mat4x4 skyboxViewMatrix = viewMatrix;
            skyboxViewMatrix[3] = glm::vec4(0, 0, 0, 1);
            glm::mat4x4 transformationMatrix = skyboxProjection * skyboxViewMatrix;
            renderSkybox(transformationMatrix, reflectionMatrix);


            //the depth buffer is currently filled with the depths of the objects in the mirror world
            //this will cause issues with rendering the real world (object behind the mirror will be rendered on top of it)
            //so we need to render the mirror's geometry to fill the depth buffer, but we disable writing to the colour buffer

            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            renderPass(false, loadedMirrors);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        }
    }

    //clear out the stencil buffer and disable the stencil test
    glClear(GL_STENCIL_BUFFER_BIT);
    glDisable(GL_STENCIL_TEST);

}


void RenderManager::ambientOcclusionRender(glm::mat4x4 &instanceMatrix, glm::mat4x4 &projViewMatrix) {

    glViewport(0, 0, renderParameters.screenResolution.x, renderParameters.screenResolution.y);

    glBindFramebuffer(GL_FRAMEBUFFER, ambientOcclusionTextureFramebuffer);
    glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(ambientOcclusionProgram);
    glm::mat4x4 lightMatrix = projViewMatrix;

    GLint poissonSphereID = glGetUniformLocation(ambientOcclusionProgram, "poissonSphere");
    glUniform4fv(poissonSphereID, renderParameters.numAmbientOcclusionSamples, &poissonSphere[0][0]);

    glm::mat4x4 foo = cameraMatrix;
    GLint cameraMatrixID = glGetUniformLocation(ambientOcclusionProgram, "cameraMatrix");
    glUniformMatrix4fv(cameraMatrixID, 1, GL_FALSE, &foo[0][0]);

    GLint screenSizeID = glGetUniformLocation(ambientOcclusionProgram, "screenSize");
    glUniform2iv(screenSizeID, 1, &renderParameters.screenResolution[0]);

    bool mirror = instanceMatrix != glm::mat4x4(1.f);
    GLint mirrorID = glGetUniformLocation(ambientOcclusionProgram, "mirror");
    glUniform1i(mirrorID, mirror ? GL_TRUE : GL_FALSE);


    for (const LoadedMesh &mesh: loadedMeshes) {
        for (const LoadedInstance &instance: mesh.instances) {

            if (!instance.properties.active) continue;

            glm::mat4x4 modelMatrix = instanceMatrix * instance.properties.translationMatrix * instance.properties.rotationMatrix;
            glm::mat4x4 transformation = lightMatrix * modelMatrix;


            GLint transformationMatrixID = glGetUniformLocation(ambientOcclusionProgram, "transformationMatrix");
            glUniformMatrix4fv(transformationMatrixID, 1, GL_FALSE, &transformation[0][0]);

            GLint modelMatrixID = glGetUniformLocation(ambientOcclusionProgram, "modelMatrix");
            glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &modelMatrix[0][0]);

            GLint boneCountID = glGetUniformLocation(ambientOcclusionProgram, "boneCount");
            glUniform1i(boneCountID, instance.properties.animBoneMatrices.size());

            if (instance.properties.animBoneMatrices.size() > 0) {

                GLuint boneMatrixBufferID;
                glGenBuffers(1, &boneMatrixBufferID);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, boneMatrixBufferID);
                size_t boneMatricesSize = instance.properties.animBoneMatrices.size() * sizeof(instance.properties.animBoneMatrices[0]);

                glBufferData(GL_SHADER_STORAGE_BUFFER, boneMatricesSize, instance.properties.animBoneMatrices.data(), GL_STATIC_DRAW);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, boneMatrixBufferID);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
            }


            for (auto &subMesh : mesh.subMeshes) {

                if (subMesh.numVertices == 0) continue;

                glm::mat4x4 instanceTransform = instanceMatrix * instance.properties.translationMatrix * instance.properties.rotationMatrix;
                if (!viewFrustumTest(&subMesh.bvh->root, instanceTransform, cameraMatrix)) continue;

                GLint textureLevelID = glGetUniformLocation(ambientOcclusionProgram, "textureLevel");
                glUniform1f(textureLevelID, subMesh.surfaceTextureInfo.level);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, depthBufferTextureID);

                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D_ARRAY, textureManager->getTextureID(subMesh.surfaceTextureInfo.bucketID));


                glBindVertexArray(subMesh.vaoID);
                ++drawCalls;
                glDrawElements(GL_TRIANGLES, subMesh.numVertices, GL_UNSIGNED_INT, nullptr);
            }
        }
    }


    glUseProgram(ambientOcclusionBlurProgram);
    glBindVertexArray(quadVao);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthBufferTextureID);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, ambientOcclusionTextureID);

    screenSizeID = glGetUniformLocation(ambientOcclusionBlurProgram, "screenSize");
    glUniform2iv(screenSizeID, 1, &renderParameters.screenResolution[0]);

    GLuint sampleDirID = glGetUniformLocation(ambientOcclusionBlurProgram, "sampleDir");
    glm::vec2 sampleDir = glm::vec2(1, 0);
    glUniform2fv(sampleDirID, 1, &sampleDir.x);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    sampleDir = glm::vec2(0, 1);
    glUniform2fv(sampleDirID, 1, &sampleDir.x);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}


/*
 * does a render pass to fill out the depth buffer for the shadow map
 */
void RenderManager::shadowRender(glm::mat4x4 lightProjection, glm::mat4x4 instanceMatrix, GLuint frameBufferID, bool viewSwap) {

    bool programSwitch = (activeProgram != passThroughProgram);

    if (programSwitch){
        glUseProgram(passThroughProgram);
    }

    //bind the framebuffer we're rendering to
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID);
    glClear(GL_DEPTH_BUFFER_BIT);

    if (viewSwap) {
        glDisable(GL_CULL_FACE);
        glViewport(0, 0, renderParameters.shadowMapResolution.x, renderParameters.shadowMapResolution.y);
    }

    glm::mat4x4 lightMatrix = lightProjection * instanceMatrix;

    for (const LoadedMesh &mesh: loadedMeshes) {
        for (const LoadedInstance &instance: mesh.instances) {

            if (!instance.properties.active) continue;

            glm::mat4x4 transformation = lightMatrix * instance.properties.translationMatrix *
                                         instance.properties.rotationMatrix;

            GLint transformationMatrixID = glGetUniformLocation(passThroughProgram, "transformationMatrix");
            glUniformMatrix4fv(transformationMatrixID, 1, GL_FALSE, &transformation[0][0]);

            GLint boneCountID = glGetUniformLocation(passThroughProgram, "boneCount");
            glUniform1i(boneCountID, instance.properties.animBoneMatrices.size());

            if (instance.properties.animBoneMatrices.size() > 0) {

                GLuint boneMatrixBufferID;
                glGenBuffers(1, &boneMatrixBufferID);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, boneMatrixBufferID);
                size_t boneMatricesSize = instance.properties.animBoneMatrices.size() * sizeof(instance.properties.animBoneMatrices[0]);

                glBufferData(GL_SHADER_STORAGE_BUFFER, boneMatricesSize, instance.properties.animBoneMatrices.data(), GL_STATIC_DRAW);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, boneMatrixBufferID);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
            }

            for (auto &subMesh : mesh.subMeshes) {

                if (subMesh.numVertices == 0) continue;

                glm::mat4x4 instanceTransform = instance.properties.translationMatrix * instance.properties.rotationMatrix;
                if (!viewFrustumTest(&subMesh.bvh->root, instanceTransform, lightProjection)) continue;

                GLint levelID = glGetUniformLocation(passThroughProgram, "textureLevel");
                glUniform1f(levelID, (float) subMesh.surfaceTextureInfo.level);

                glActiveTexture(GL_TEXTURE2);
                GLuint textureID = textureManager->getTextureID(subMesh.surfaceTextureInfo.bucketID);
                assert(textureID);
                glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);

                glBindVertexArray(subMesh.shadowVaoID);
                ++drawCalls;
                glDrawElements(GL_TRIANGLES, subMesh.numVertices, GL_UNSIGNED_INT, nullptr);
            }
        }
    }

    //undo the state changes we made (if applicable)
    if (programSwitch) {
        glUseProgram(activeProgram);
        glEnable(GL_CULL_FACE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, renderParameters.screenResolution.x, renderParameters.screenResolution.y);
    }

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);


}


/*
 * renders the skybox
 */
void RenderManager::renderSkybox(glm::mat4x4 &transformationMatrix, glm::mat4x4 instanceMatrix) {

    if (skyboxVao == 0) return; //no loaded skybox

    bool programSwitch = (activeProgram != skyboxProgram);

    if (programSwitch){
        glUseProgram(skyboxProgram);
        glViewport(0, 0, renderParameters.screenResolution.x, renderParameters.screenResolution.y);
        glDisable(GL_CULL_FACE);
    }



    glBindVertexArray(skyboxVao);

    GLint transformationID = glGetUniformLocation(skyboxProgram, "transformationMatrix");
    glUniformMatrix4fv(transformationID, 1, GL_FALSE, &transformationMatrix[0][0]);

    GLint instanceMatrixID = glGetUniformLocation(skyboxProgram, "instanceMatrix");
    glUniformMatrix4fv(instanceMatrixID, 1, GL_FALSE, &instanceMatrix[0][0]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, renderMode == SHADOWS_ONLY ? irradianceTextureID : skyboxTextureID);

    ++drawCalls;
    glDrawArrays(GL_TRIANGLES, 0, 48);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    if (programSwitch){
        glUseProgram(activeProgram);
        glEnable(GL_CULL_FACE);
    }
}



void RenderManager::renderPass(bool renderTranslucent, std::set<LoadedMesh, meshComp> &renderTargets, glm::mat4x4 transform, bool mirror) {

    bool programSwitch = (activeProgram != mainProgram);

    if (programSwitch){
        glUseProgram(mainProgram);
        glViewport(0, 0, renderParameters.screenResolution.x, renderParameters.screenResolution.y);
    }

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D_ARRAY, shadowManager->textureID);
    assert(shadowManager->textureID);

    passRenderUniforms();

    for (const LoadedMesh& mesh : renderTargets) {
        for (const LoadedInstance& instance : mesh.instances) {

            if (!instance.properties.active) continue;

            lodAlphaBlend = 1;
            if (instance.properties.minLOD != -1){
                glm::vec4 centre = glm::vec4((mesh.bvh->root.b1 + mesh.bvh->root.b2) / 2.f, 1.f);
                centre = instance.properties.translationMatrix * centre;
                centre = viewMatrix * centre;

                const float farPlaneDistance = 200.f; //TODO -- get actual value
                float pos = -centre.z / farPlaneDistance;

                const float blendZone = 0.05;
                if (pos < instance.properties.minLOD || pos > instance.properties.maxLOD + blendZone) continue;
                if (pos > instance.properties.maxLOD){
                    if (!renderTranslucent) continue;
                    lodAlphaBlend = 1.f - ((1.f / blendZone) * (pos - instance.properties.maxLOD));
                }
            }

            passInstanceUniforms(mesh, instance, transform, mirror);

            for (auto &subMesh : mesh.subMeshes) {

                if (subMesh.material->translucent != renderTranslucent){
                    if (renderTranslucent){
                        if (lodAlphaBlend == 1) continue;
                    }
                }

                glm::mat4x4 instanceTransform = transform * instance.properties.translationMatrix * instance.properties.rotationMatrix;
                if (!viewFrustumTest(&subMesh.bvh->root, instanceTransform, cameraMatrix)) continue;

                GLint numVertices = subMesh.numVertices;
                if (numVertices == 0) continue;

                //load the mesh data
                glBindVertexArray(subMesh.vaoID);

                //pass variable uniform data
                passMaterialUniforms(subMesh);

                numVerticesRendered += subMesh.numVertices;

                //and draw
                ++drawCalls;
                glDrawElements(GL_TRIANGLES, subMesh.numVertices, GL_UNSIGNED_INT, nullptr);
            }
        }
    }

    if (programSwitch){
        glUseProgram(activeProgram);
    }


    //unbind vertex array and texture
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}


void RenderManager::swapProgram(GLuint program) {
    activeProgram = program;
    glUseProgram(program);
}

/*
 * render loop
 * iterates through each instance of each mesh and renders
 */

void RenderManager::render(glm::mat4x4 projectionMatrix, glm::mat4x4 viewMatrix, glm::vec3 viewDir, glm::vec3 cameraPos) {

    drawCalls = 0; //new frame, reset the draw calls counter
    viewFrustumCulls = 0;
    viewFrustumTests = 0;

    if (reloadTextures){
        textureManager->fillBuckets();
        reloadTextures = false;
    }

    cameraMatrix = projectionMatrix * viewMatrix;
    this->viewMatrix = viewMatrix;
    //make sure view direction is normalised
    viewDir = glm::normalize(viewDir);

    //create the point and directional light vectors
    fillLightVectors();

    //update the shadow manager with the current projection matrix and view matrix
    //note that if these change mid frame (ie we render from another view point) we'll need to call this again
    shadowManager->updateProjectionMatrix(projectionMatrix);
    shadowManager->calculateCascades(viewMatrix);

    glViewport(0, 0, renderParameters.screenResolution.x, renderParameters.screenResolution.y);
//    shadowRender(cameraMatrix, glm::mat4x4(1.f), depthBufferFrameBufferID, false);
//    glm::mat4x4 I(1.f);
//    ambientOcclusionRender(I, cameraMatrix);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    activeProgram = mainProgram;
    glUseProgram(mainProgram);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


    //load all the constant uniforms onto the gpu, ie the uniforms that don't change during the frame
    passFrameUniforms(viewMatrix, cameraPos);

    //render any mirrors that exist
    renderMirrors(viewMatrix);

    glViewport(0, 0, renderParameters.screenResolution.x, renderParameters.screenResolution.y);
    shadowRender(cameraMatrix, glm::mat4x4(1.f), depthBufferFrameBufferID, false);
    glm::mat4x4 I(1.f);
    ambientOcclusionRender(I, cameraMatrix);


    shadowManager->renderShadows();

    renderPass(false, loadedMeshes); //opaque objects first
    renderPass(true, loadedMeshes); //then translucent

    glm::mat4x4 skyboxViewMatrix = viewMatrix;
    skyboxViewMatrix[3] = glm::vec4(0, 0, 0, 1); //remove translation component of view matrix
    glm::mat4x4 transformationMatrix = skyboxProjection * skyboxViewMatrix;

    //manually swap to the skybox program, this is so we don't swap back to the main program in renderSkybox()
    //this is only ok because we don't render anything else after, if we do we might not be able to do this
    activeProgram = skyboxProgram;
    glUseProgram(skyboxProgram);
    renderSkybox(transformationMatrix);

    glActiveTexture(0);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    //if (debugClick) {

      //  RayInfo info = core::getCollisionManager()->rayCast(cameraPos, getRayDir(cameraPos));

        //if (info.hit) {
          //  setDebugMessage(info.entity, info.modelID, info.meshID);
        //}

        //else debugMessage = "No object detected!";

    //}
    // UI new frame
    uiManager->newFrame();

    // UI render
    uiManager->render();


    //GLTtext *text = gltCreateText();
    //gltSetText(text, debugMessage.c_str());

    //gltBeginDraw();

    //gltColor(1.0f, 1.0f, 1.0f, 1.0f);
    //gltDrawText2D(text, 1, 1, 1);

    //gltEndDraw();

   // debugClick = false;

    //gltDeleteText(text);

    numVerticesRendered = 0;

}

void RenderManager::setDebugMessage(Entity *entity, size_t objectID, size_t meshID) {
    LoadedMesh search;
    search.meshID = objectID;
    auto foo = loadedMeshes.find(search);
    assert(foo != loadedMeshes.end());

    Material *material = foo->subMeshes[meshID].material;

    debugMessage = "Entity name = " + entity->getName() + "\n";

    debugMessage += "Material name = " + material->materialName + "\n";

    if (material->diffuseTexture.textureData != nullptr) {
        debugMessage += "Diffuse texture path = " + material->diffuseTexture.texturePath + "\n";
    }

    else {
        debugMessage +=
                "Diffuse colour = "
                + std::to_string(material->diffuse.x) + ","
                + std::to_string(material->diffuse.y) + ","
                + std::to_string(material->diffuse.z) + "\n";
    }

    if (material->metallicnessTexture.textureData != nullptr) {
        debugMessage += "Metallicness texture path = " + material->metallicnessTexture.texturePath + "\n";
    }

    else {
        debugMessage +=
                "Specular colour = "
                + std::to_string(material->specular.x) + ","
                + std::to_string(material->specular.y) + ","
                + std::to_string(material->specular.z) + "\n";
        debugMessage +=
                "Shininess = "
                + std::to_string(material->shininess) + "\n";

    }

    if (material->roughnessTexture.textureData != nullptr) {
        debugMessage += "Roughness texture path = " + material->roughnessTexture.texturePath + "\n";
    }

    if (material->ambientOcclusionTexture.textureData != nullptr) {
        debugMessage += "Ambient occlusion texture path = " + material->ambientOcclusionTexture.texturePath + "\n";
    }

    if (material->parallaxTexture.textureData != nullptr) {
        debugMessage += "Parallax / heightmap texture path = " + material->parallaxTexture.texturePath + "\n";
    }


    if (material->diffuseTexture.textureData == nullptr) {

        debugMessage +=
                "Ambient colour = "
                + std::to_string(material->ambient.x) + ","
                + std::to_string(material->ambient.y) + ","
                + std::to_string(material->ambient.z) + "\n";
    }

    debugMessage +=
                   "Emissive colour = "
                   + std::to_string(material->emissive.x) + ","
                   + std::to_string(material->emissive.y) + ","
                   + std::to_string(material->emissive.z) + "\n";



}


void RenderManager::renderModelBoundingBoxes(glm::mat4x4 projectionMatrix, glm::mat4x4 viewMatrix, size_t depth) {

    drawCalls = 0;
    cameraMatrix = projectionMatrix * viewMatrix;
    glUseProgram(plainDrawProgram);
    glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    GLint colourID = glGetUniformLocation(plainDrawProgram, "colour");
    GLint transformationID = glGetUniformLocation(plainDrawProgram, "transformationMatrix");

    for (const LoadedMesh& mesh : loadedMeshes) {

        size_t end = std::min(depth, mesh.bvhVaoIDs.size());
        for (size_t i = 0; i < end; i++) {

            float level = 1.f - (i / (float) (end - 1));

            glBindVertexArray(mesh.bvhVaoIDs[i]);
            GLint numVertices = 36 * mesh.numBvhVertices[i];

            for (const LoadedInstance &instance: mesh.instances) {

                glm::mat4x4 transformationMatrix = cameraMatrix *
                                                   instance.properties.translationMatrix *
                                                   instance.properties.rotationMatrix;

                glUniformMatrix4fv(transformationID, 1, GL_FALSE, &transformationMatrix[0][0]);

                glm::vec4 colour = glm::vec4(level, level, level, 1);
                glUniform4fv(colourID, 1, &colour[0]);

                ++drawCalls;
                glDrawArrays(GL_TRIANGLES, 0, numVertices);
            }
        }
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glUseProgram(activeProgram);
    glEnable(GL_CULL_FACE);
}



void RenderManager::renderMeshBoundingBoxes(glm::mat4x4 projectionMatrix, glm::mat4x4 viewMatrix, size_t depth){
    drawCalls = 0;
    cameraMatrix = projectionMatrix * viewMatrix;
    glUseProgram(plainDrawProgram);
    glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    GLint colourID = glGetUniformLocation(plainDrawProgram, "colour");
    GLint transformationID = glGetUniformLocation(plainDrawProgram, "transformationMatrix");

    for (auto &mesh : loadedMeshes) {

        for (auto &subMesh : mesh.subMeshes) {

            size_t end = std::min(depth, subMesh.bvhVaoIDs.size());
            for (size_t i = 0; i < end; i++) {

                float level = 1.f - (i / (float) (end - 1));

                glBindVertexArray(subMesh.bvhVaoIDs[i]);
                GLint numVertices = 36 * subMesh.numBvhVertices[i];

                for (const LoadedInstance &instance: mesh.instances) {

                    glm::mat4x4 transformationMatrix = cameraMatrix *
                                                       instance.properties.translationMatrix *
                                                       instance.properties.rotationMatrix;

                    glUniformMatrix4fv(transformationID, 1, GL_FALSE, &transformationMatrix[0][0]);

                    glm::vec4 colour = glm::vec4(level, level, level, 1);
                    glUniform4fv(colourID, 1, &colour[0]);

                    ++drawCalls;
                    glDrawArrays(GL_TRIANGLES, 0, numVertices);
                }
            }
        }
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glUseProgram(activeProgram);
    glEnable(GL_CULL_FACE);

}





void RenderManager::updateScreenSize(glm::ivec2 screenSize) {
    renderParameters.screenResolution = screenSize;

    depthBufferTextureID = textureManager->createDepthBufferTexture(renderParameters.screenResolution.x, renderParameters.screenResolution.y);
    glBindFramebuffer(GL_FRAMEBUFFER, depthBufferFrameBufferID);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthBufferTextureID, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    textureManager->loadRenderTexture(&ambientOcclusionTextureID, &ambientOcclusionTextureRenderObject,
                                      ambientOcclusionTextureFramebuffer, renderParameters.screenResolution.x, renderParameters.screenResolution.y);

    glBindFramebuffer(GL_FRAMEBUFFER, ambientOcclusionTextureFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ambientOcclusionTextureID, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


}


size_t RenderManager::getDrawCalls() const{
    return drawCalls;
}

bool RenderManager::viewFrustumTest(AABB *box, glm::mat4x4 &transform, glm::mat4x4 &projection) {

    ++viewFrustumTests;

    //transpose because opengl/glm is column major...
    glm::mat4x4 renderTransform = glm::transpose(projection);
    glm::vec4 planes[4] = {-(renderTransform[3] + renderTransform[0]),
                           -(renderTransform[3] - renderTransform[0]),
                           -(renderTransform[3] + renderTransform[1]),
                           -(renderTransform[3] - renderTransform[1])
    };

    for (int i = 0; i < 4; i++){
        if (!planeAABB_intersectionTest(box, transform, planes[i])){
            ++viewFrustumCulls;
            return false;
        }
    }

    return true;
}

glm::vec3 RenderManager::getRayDir(glm::vec3 origin){

    int x = mouseX;
    int y = renderParameters.screenResolution.y - mouseY;

    glm::vec4 corners[4] = {
            glm::vec4 (-1, -1, -1, 1),
            glm::vec4 (-1,  1, -1, 1),
            glm::vec4 ( 1, -1, -1, 1),
            glm::vec4 ( 1,  1, -1, 1)
    };

    glm::mat4x4 inverseMat = glm::inverse(cameraMatrix);
    for (auto &corner : corners){
        corner = inverseMat * corner;
        corner /= corner.w;
    }

    float ratioX = x / (float) renderParameters.screenResolution.x;
    float ratioY = y / (float) renderParameters.screenResolution.y;

    glm::vec3 point1 = ((1.f - ratioX) * glm::vec3(corners[0])) + (ratioX * glm::vec3(corners[2]));
    glm::vec3 point2 = ((1.f - ratioX) * glm::vec3(corners[1])) + (ratioX * glm::vec3(corners[3]));

    glm::vec3 point = ((1.f - ratioY) * point1) + (ratioY * point2);

    return glm::normalize(point - origin);
}


void elmt::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
        core::getRenderManager()->debugClick = true;
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        core::getRenderManager()->mouseX = x;
        core::getRenderManager()->mouseY = y;
    }
}

RenderManager::~RenderManager() {
    delete shadowManager;
    delete textureManager;
    gltTerminate();

}


