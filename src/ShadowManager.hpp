#pragma once
#include "RenderManager.hpp"

namespace elmt {

    class DirectionalLight;
    class TextureManager;
    class RenderManager;

    struct DirectionalLightGPUObject;

    struct FrustumPlane{
        glm::vec4 bounds[4]; //starts at bottom left corner and moves clockwise
    };

    class ShadowManager {

        friend class RenderManager;

        public:
            ShadowManager(TextureManager *textureManager, RenderManager *renderManager);
            ~ShadowManager() = default;

            void calculateCascades(glm::mat4x4 viewMatrix);
            void renderShadows(glm::mat4x4 instanceMatrix = glm::mat4x4(1.f));
            void updateProjectionMatrix(glm::mat4x4 &projectionMatrix);

            GLuint textureID;


        protected:
            std::vector<glm::mat4x4> lightTransforms;
            std::vector<float> viewSpaceDepths;
            std::vector<glm::vec2> poissonDisc;


        private:
            glm::mat4x4 createDirectionalLightTransform(glm::vec3 dir);
            void calculateFrustums();
            glm::mat4x4 calculateLightFrustumTransform(FrustumPlane &p1, FrustumPlane &p2, DirectionalLightGPUObject &light, glm::mat4x4 &worldTransform);
            void verifyCascadePartitions();

            void fillPoissonDisc(size_t numSamples, float minDistance);

            glm::mat4x4 activeProjectionMatrix;

            RenderManager *renderManager;
            TextureManager *textureManager;

            std::vector<GLuint> framebufferIDs;
            std::vector<FrustumPlane> nearFrustumPlanes;
            std::vector<FrustumPlane> farFrustumPlanes;

            std::vector<float> cascadePartitions;

            size_t width;
            size_t height;
            size_t depth;

            float blendSize = 1.2;


    };
}


