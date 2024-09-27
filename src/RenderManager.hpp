#pragma once
#include "../external/glm-0.9.7.1/glm/glm.hpp"
#include <vector>
#include <set>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iterator>
#include "DirectionalLight.h"
#include "PointLight.h"
#include "TextureManager.hpp"
#include "ShadowManager.hpp"
#include "UIManager.hpp"
#include "RenderStructs.hpp"
#include "Skybox.hpp"
#include "Mesh.hpp"
#include "Material.hpp"
#include "BoundingVolumeHierarchy.hpp"
#include "Entity.hpp"


namespace elmt {

    void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);


    //struct contains all of the user controllable parameters involved in rendering
    //these include values used by RenderManager, ShadowManager, TextureManage, any future rendering classes and shaders
    //all fields are intialised to sensible default values where appropriate
    struct RenderParameters{
        std::vector<float> cascadePartitions;
        glm::vec4 backGroundColour = glm::vec4(0.2, 0.2, 0.2, 0.2);
        size_t maxDirectionalShadows = 16;

        glm::ivec2 shadowMapResolution = glm::ivec2(1024, 1024);
        glm::ivec2 screenResolution = glm::ivec2(1920, 1080);

        size_t shadowPCFNumSamples = 12;
        float pcfPoissonSampleSpacing = 0.4;
        float cascadeHeight = 100.f;

        size_t numAmbientOcclusionSamples = 16;

        size_t maxTextureBucketSize = 128;

        float ambientLightIntensity = 0.1f;
        float pcfRadius = 1.f / 256.f;

        float parallaxHeight = 0.035;
        float parallaxOcclusionAccuracy = 2;
    };

    struct TangentPair{
        glm::vec3 tangent;
        glm::vec3 biTangent;
    };


    enum RenderDebugMode{NO_DEBUG, NORMALS, AMBIENT_OCCLUSION, ENVIRONMENT_LIGHT, RGBA_TEXTURE, SHADOWS_ONLY,
            MIPMAP_LEVELS, BLINN_PHONG_SHADING, DIFFUSE_ONLY, SPECULAR_ONLY, AMBIENT_ONLY, ROUGHNESS_ONLY, METALLICNESS_ONLY,
            PARALLAX_ONLY};


    class DirectionalLight;
    class PointLight;
    class TextureManager;
    class ShadowManager;
    class BVH;
    class ImageRendererComponent;

    struct TextureInfo;
    struct OldMesh;
    struct AABB;

    //instance properties -- this is the data that is specific to each instance
    struct InstanceProperties{
        glm::mat4x4 translationMatrix = glm::mat4x4(1.f);
        glm::mat4x4 rotationMatrix = glm::mat4x4(1.f);
        
        std::vector<glm::mat4x4> animBoneMatrices;
        float minLOD = -1;
        float maxLOD = -1;
        bool active = true;
    };

    //struct containing an instance ID and an instance's properties
    struct LoadedInstance{
        size_t instanceID{};
        std::string instanceName;
        mutable InstanceProperties properties;
    };

    //used for set comparisons
    struct instanceComp{
        bool operator()(const LoadedInstance &a, const LoadedInstance &b) const{
            return a.instanceID < b.instanceID;
        }
    };

    struct LoadedSubMesh{

        BVH *bvh;

        Material *material;
        TextureInfo surfaceTextureInfo{};
        TextureInfo normalTextureInfo{};
        TextureInfo roughnessTextureInfo{};
        TextureInfo metallicnessTextureInfo{};
        TextureInfo parallaxTextureInfo{};
        TextureInfo ambientOcclusionTextureInfo{};


        GLuint vaoID{};
        GLuint shadowVaoID{};

        GLsizei numVertices{};

        std::vector<GLuint> bvhVaoIDs;
        std::vector<GLsizei> numBvhVertices;

    };

    //struct containing mesh info
    struct LoadedMesh{
        mutable std::set<LoadedInstance, instanceComp> instances;
        mutable std::vector<LoadedSubMesh> subMeshes;
        mutable BVH *bvh = nullptr;
        mutable glm::vec3 origin = glm::vec3(0, 0, 0);
        mutable glm::vec3 orientation = glm::vec3(0, 0, -1); //starting orientation of mesh

        mutable std::vector<GLuint> bvhVaoIDs;
        mutable std::vector<GLsizei> numBvhVertices;


        size_t meshID{};
    };

    //used for set comparisons
    struct meshComp{
        bool operator()(const LoadedMesh &a, const LoadedMesh &b) const{
            return a.meshID < b.meshID;
        }
    };




    struct PointLightData{
        mutable PointLight* light;
        size_t ID;
    };

    struct pointLightComp{
        bool operator()(const PointLightData &a, const PointLightData &b) const{
            return a.ID < b.ID;
        }
    };

    struct DirectionalLightData{
        mutable DirectionalLight* light;
        size_t ID;
    };

    struct directionalLightComp{
        bool operator()(const DirectionalLightData &a, const DirectionalLightData &b) const{
            return a.ID < b.ID;
        }
    };

    struct DirectionalLightGPUObject{
        glm::vec4 dir;
        glm::vec4 colour;
    };

    struct PointLightGPUObject{
        glm::vec4 dir;
        glm::vec4 colour;
    };



    class RenderManager {
        public:
            explicit RenderManager(RenderParameters *suppliedRenderParameters);
            ~RenderManager();

            size_t addMesh(OldMesh *mesh, BVH *bvh);
            size_t addInstance(const std::string &name, size_t meshID, float minLOD = -1, float maxLOD = -1);
            std::vector<size_t> addInstance(const std::string &name, std::vector<size_t> &meshIDs, std::vector<float> &lodPartitions);


            size_t addNewModel(std::vector<Mesh> &mesh, std::vector<Material> &material, BVH *bvh, bool mirror = false);

            size_t addMirrorMesh(OldMesh *mesh, BVH *bvh);

            void updateMesh(size_t meshID, OldMesh *mesh);
            void updateInstance(InstanceProperties &newProperties, size_t meshID, size_t instanceID);

            void deleteInstance(size_t meshID, size_t instanceID);
            void deleteMesh(size_t meshID);

            size_t addPointLight(PointLight *pointLight);
            size_t addDirectionalLight(DirectionalLight *directionalLight);

            void render(glm::mat4x4 projectionMatrix, glm::mat4x4 viewMatrix, glm::vec3 viewDir, glm::vec3 cameraPos);
            void setSkybox(Skybox *skybox);

            void renderModelBoundingBoxes(glm::mat4x4 projectionMatrix, glm::mat4x4 viewMatrix, size_t depth = 1);
            void renderMeshBoundingBoxes(glm::mat4x4 projectionMatrix, glm::mat4x4 viewMatrix, size_t depth = 1);

			inline TextureManager* getTextureManager() const { return textureManager; }
			inline UIManager* getUIManager() const { return uiManager; }

            RenderDebugMode renderMode = NO_DEBUG;
            void updateScreenSize(glm::ivec2 screenSize);

            glm::ivec2 getScreenSize() { return renderParameters.screenResolution; }
            size_t bufferAllocSize = 0;
            size_t getDrawCalls() const;
            std::uint32_t renderBoneId = 0;

            bool parallaxEnabled = true; //debug purposes, if this is still here when we get to submission shout at me (oscar)
            bool debugClick = false;
            int mouseX;
            int mouseY;

        protected:
            friend class TextureManager;
            friend class ShadowManager;
            friend class UIManager;
            friend class ImageRendererComponent;

            TextureManager *textureManager;
            ShadowManager *shadowManager;
            UIManager* uiManager;

            RenderParameters renderParameters;

            std::vector<DirectionalLightGPUObject> directionalLightsVector;
            std::vector<PointLightGPUObject> pointLightsVector;

            void shadowRender(glm::mat4x4 lightProjection, glm::mat4x4 instanceMatrix, GLuint frameBufferID, bool viewSwap = true);

            void swapProgram(GLuint program);
            GLuint mainProgram;
            GLuint passThroughProgram;
            GLuint plainDrawProgram;
            GLuint skyboxProgram;
            GLuint diffuseIrradianceProgram;
            GLuint ambientOcclusionProgram;
            GLuint ambientOcclusionBlurProgram;
            GLuint activeProgram;


        private:
            std::set<LoadedMesh>::const_iterator findMesh(size_t meshID);
            static std::set<LoadedInstance>::const_iterator findInstance(std::set<LoadedMesh>::const_iterator mesh, size_t instanceID);
            static void loadShader(char *shaderPath, GLuint shaderID);

            // Old VAO methods
            static GLuint createPlainVao(float *coords, size_t coordsSize, float *values);

            // new VAO methods
            GLuint createVAO(Mesh* mesh, GLuint *shadowVao);
            GLuint createShadowVAO(Mesh* mesh, GLuint VBO, GLuint EBO);

            void fillLightVectors();

            void passFrameUniforms(glm::mat4x4 &viewMatrix, glm::vec3 viewDir);
            void passRenderUniforms();
            void passMeshUniforms(const LoadedMesh &mesh, const LoadedInstance &instance, size_t i) const;
            void passInstanceUniforms(const LoadedMesh &mesh, const LoadedInstance &instance, glm::mat4x4 &transform, bool mirror) const;
            void passMaterialUniforms(const LoadedSubMesh &mesh) const;

            void renderMirrors(glm::mat4x4 &viewMatrix);
            static glm::mat3x3 calculatePlanarBasis(glm::vec3 normal);
            static glm::mat4x4 calculateReflectionMatrix(glm::vec3 normal, glm::vec3 planarOrigin);

            std::vector<glm::vec4> poissonSphere; //ambient occlusion samples
            void fillPoissonSphere();

            static GLuint setupProgram(char *vertexShader, char *fragmentShader);

            void renderSkybox(glm::mat4x4 &transformationMatrix, glm::mat4x4 instanceMatrix = glm::mat4x4(1.f));
            void renderPass(bool renderTranslucent, std::set<LoadedMesh, meshComp> &renderTargets, glm::mat4x4 transform = glm::mat4x4(1.f), bool mirror = false);

            static void calculateCubeCoords(float *ret, float *values, glm::vec3 b1, glm::vec3 b2);
            std::vector<TangentPair> calculateNormalMapTangents(Mesh &mesh);
            TangentPair calculateTangentPair(glm::vec3 *vertices, glm::vec2 *uvs);

            void createBvhVaos(std::vector<GLuint> &vaos, std::vector<GLsizei> &vertices, AABB *root, size_t depth = (size_t) -1);

            void ambientOcclusionRender(glm::mat4x4 &instanceMatrix, glm::mat4x4 &projViewMatrix);
            bool viewFrustumTest(AABB *box, glm::mat4x4 &transform, glm::mat4x4 &projection);
            glm::vec3 getRayDir(glm::vec3 origin);

            void setDebugMessage(Entity *entity, size_t objectID, size_t meshID);
            std::string debugMessage;

            std::set<LoadedMesh, meshComp> loadedMeshes;

            std::set<LoadedMesh, meshComp> loadedMirrors;

            std::set<PointLightData, pointLightComp> pointLights;
            std::set<DirectionalLightData, directionalLightComp> directionalLights;

            glm::mat4x4 skyboxProjection = glm::mat4x4(1.f);
            glm::mat4x4 cameraMatrix = glm::mat4x4(1.f);
            glm::mat4x4 viewMatrix = glm::mat4x4(1.f);

            size_t meshIDs = 0;
            size_t instanceIDs = 0;
            size_t directionalLightIDs = 0;
            size_t pointLightIDs = 0;
            size_t drawCalls = 0;
            size_t numVerticesRendered = 0;

            size_t viewFrustumCulls = 0;
            size_t viewFrustumTests = 0;

            GLuint skyboxTextureID = 0;
            GLuint skyboxVao = 0;

            GLuint irradianceTextureID = 0;
            GLuint irradianceTextureRenderObject = 0;
            GLuint irradianceTextureFramebuffer = 0;

            GLuint ambientOcclusionTextureID = 0;
            GLuint ambientOcclusionTextureRenderObject = 0;
            GLuint ambientOcclusionTextureFramebuffer = 0;


            GLuint depthBufferTextureID = 0;
            GLuint depthBufferFrameBufferID = 0;
            GLuint quadVao;

            float lodAlphaBlend;

            TextureInfo dummyTextureInfo{};

            const size_t maxBasicLights = 512;

            bool reloadTextures = true;

    };

}



