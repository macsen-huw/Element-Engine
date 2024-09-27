#include "ShadowManager.hpp"
#include "Logger.hpp"
#include "../external/glm-0.9.7.1/glm/gtc/matrix_transform.hpp"
#include <algorithm>
#include <random>

using namespace elmt;

ShadowManager::ShadowManager(TextureManager *textureManager, RenderManager *renderManager) {

    this->textureManager = textureManager;
    this->renderManager = renderManager;
    this->width = renderManager->renderParameters.shadowMapResolution.x;
    this->height = renderManager->renderParameters.shadowMapResolution.y;
    this->depth = renderManager->renderParameters.maxDirectionalShadows;
    this->cascadePartitions = renderManager->renderParameters.cascadePartitions;

    std::sort(this->cascadePartitions.begin(), this->cascadePartitions.end());
    verifyCascadePartitions();


    //fill the poisson disc for pcf in the fragment shader
    fillPoissonDisc(renderManager->renderParameters.shadowPCFNumSamples, renderManager->renderParameters.pcfPoissonSampleSpacing);


    textureManager->createShadowBucket(width, height, depth);
    textureID = textureManager->getShadowTextureID();


    for (size_t i = 0; i < depth; i++){
        GLuint framebufferID;
        glGenFramebuffers(1, &framebufferID);
        glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);

        glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);
        glNamedFramebufferTextureLayer(framebufferID, GL_DEPTH_ATTACHMENT, textureID, 0, i);

        framebufferIDs.push_back(framebufferID);
    }

    glDrawBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//performs some verification on the cascade partitions
void ShadowManager::verifyCascadePartitions() {
    if (cascadePartitions.size() > depth){
        Logger::Print("Error - more cascade partitions than space to store shadow maps\n", LOGCAT::CAT_RENDERING, LOGSEV::SEV_ERROR);
        exit(EXIT_FAILURE);
    }

    for (float &f : cascadePartitions){
        if (f < 0 || f > 1){
            Logger::Print("Error - all cascade partitions must be in range 0 to 1. (0 is the near plane, 1 is the far plane)\n",
                          LOGCAT::CAT_RENDERING, LOGSEV::SEV_ERROR);
            exit(EXIT_FAILURE);
        }
    }
}


//function calculates the light projection + view matrix for vanilla shadow mapping (no cascading)
glm::mat4x4 ShadowManager::createDirectionalLightTransform(glm::vec3 dir) {

    glm::mat4x4 orthographicMatrix = glm::ortho(-125.0f, 125.0f, -125.0f, 125.0f, 1.f, 300.f);

    glm::vec3 cameraPos = (-200.f * dir);

    glm::vec3 temp = glm::normalize(glm::vec3(1, 1, 1));
    if (dir == temp){ //make sure temp != dir
        temp.x += 1.f;
    }

    glm::vec3 right = glm::cross(dir, temp);
    glm::vec3 up = glm::cross(right, dir);

    glm::mat4x4 viewMatrix = glm::lookAt(
            cameraPos,
            glm::vec3(0, 0, 0),
            up
    );

    glm::mat4x4 lightTransform = orthographicMatrix * viewMatrix;
    return lightTransform;

}

/*
 * partitions the frustums into sub frustums for use in cascaded shadow mapping
 * it also adds the view depths, but only once - if we update it every frame it leads to small but noticeable shimmering
 */

void ShadowManager::calculateFrustums() {

    //first, convert NDC space (cube with bounding box (-1, -1, -1) to (1, 1, 1)) to view space
    glm::mat4x4 projectionInverse = glm::inverse(activeProjectionMatrix);

    FrustumPlane nearPlane;
    nearPlane.bounds[0] = projectionInverse * glm::vec4(-1, -1, -1, 1);
    nearPlane.bounds[1] = projectionInverse * glm::vec4(-1, 1, -1, 1);
    nearPlane.bounds[2] = projectionInverse * glm::vec4(1, 1, -1, 1);
    nearPlane.bounds[3] = projectionInverse * glm::vec4(1, -1, -1, 1);


    FrustumPlane farPlane;
    farPlane.bounds[0] = projectionInverse * glm::vec4(-1, -1, 1, 1);
    farPlane.bounds[1] = projectionInverse * glm::vec4(-1, 1, 1, 1);
    farPlane.bounds[2] = projectionInverse * glm::vec4(1, 1, 1, 1);
    farPlane.bounds[3] = projectionInverse * glm::vec4(1, -1, 1, 1);

    //perform perspective divide
    for (int i = 0; i < 4; i++){
        nearPlane.bounds[i] /= nearPlane.bounds[i].w;
        farPlane.bounds[i] /= farPlane.bounds[i].w;
    }

    //we now have the coordinates of the view frustum in world space
    //so we need to partition it

    //calculate the depth of the start and end of the frustum in view space
    float nearViewDepth = (nearPlane.bounds[0]).z;
    float farViewDepth = (farPlane.bounds[0]).z;

    nearFrustumPlanes.push_back(nearPlane);

    //partition the frustum based on linear interpolation of world space coordinates and view space depth

    for (float currentStep : cascadePartitions){


        float viewSpaceDepth = ((1.f - currentStep) * nearViewDepth) + (currentStep * farViewDepth);
        viewSpaceDepths.push_back(viewSpaceDepth);

        float shiftedDepth = std::min(1.f, blendSize * currentStep);

        FrustumPlane newNearPlane, newFarPlane;
        for (int j = 0; j < 4; j++){
            newNearPlane.bounds[j] = ((1.f - currentStep) * nearPlane.bounds[j]) + (currentStep * farPlane.bounds[j]);
            newFarPlane.bounds[j] = ((1.f - shiftedDepth) * nearPlane.bounds[j]) + (shiftedDepth * farPlane.bounds[j]);
        }

        nearFrustumPlanes.push_back(newNearPlane);
        farFrustumPlanes.push_back(newFarPlane);
    }

    farFrustumPlanes.push_back(farPlane);
    viewSpaceDepths.push_back(farViewDepth);

}


/*
 * function calculates a light transformation matrix for a given sub frustum and light
 */

glm::mat4x4 ShadowManager::calculateLightFrustumTransform(FrustumPlane &p1, FrustumPlane &p2, DirectionalLightGPUObject &light, glm::mat4x4 &worldTransform) {

    //we want to create a projection matrix for the frustum for the given light
    //to do this we need to build a view matrix and an orthographic matrix
    //the view matrix needs to know where to look (centre of frustum is a simple choice) and the rest of the data we can get from the light

    //the orthographic matrix needs to know the bounds of the frustum (for it to render over), so we calculate a bounding
    //box of the frustum (from the light's view point)


    //create the view matrix
    glm::vec4 centre(0.f, 0.f, 0.f, 0.f);
    for (int i = 0; i < 4; i++){
        centre += p1.bounds[i] + p2.bounds[i];
    }
    centre /= 8.f;
    centre = worldTransform * centre;


    glm::vec3 lightDir(light.dir.x, light.dir.y, light.dir.z);
    lightDir *= -1;

    glm::vec3 temp = glm::normalize(glm::vec3(1, 1, 1));
    if (lightDir == temp){ //make sure temp != dir
        temp.x += 1.f;
    }

    glm::vec3 right = glm::cross(lightDir, temp);
    glm::vec3 up = glm::cross(right, lightDir);

    glm::mat4x4 viewMatrix = glm::lookAt(
            lightDir + glm::vec3(centre.x, centre.y, centre.z),
            glm::vec3(centre.x, centre.y, centre.z),
            up
    );



    //create the orthographic matrix
    //first calculate the bounding box
    glm::vec4 minBounds(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), 1);
    glm::vec4 maxBounds(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), 1);

    for (int i = 0; i < 4; i++){
        for (int j = 0; j < 3; j++) {

            glm::vec4 corner1 = viewMatrix * worldTransform * p1.bounds[i];
            glm::vec4 corner2 = viewMatrix * worldTransform * p2.bounds[i];

            minBounds[j] = std::min(minBounds[j], corner1[j]);
            minBounds[j] = std::min(minBounds[j], corner2[j]);

            maxBounds[j] = std::max(maxBounds[j], corner1[j]);
            maxBounds[j] = std::max(maxBounds[j], corner2[j]);
        }
    }


    //there is another thing to do, our view matrix is pointing in the negative z direction, but glm::ortho expects
    //to be pointing in the positive z direction
    //so we swap around the maxBounds.z and minBounds.z, and flip their sign

    std::swap(maxBounds.z, minBounds.z);

    minBounds.z *= -1;
    maxBounds.z *= -1;

    //finally, decrease min bounds by some amount in-case theres a high up occluder - because its a directional light it still needs shadowing

    minBounds.z -= renderManager->renderParameters.cascadeHeight;


    glm::mat4x4 orthographicMatrix = glm::ortho(minBounds.x, maxBounds.x, minBounds.y, maxBounds.y, minBounds.z, maxBounds.z);
    return orthographicMatrix * viewMatrix;

}



/*
 * fills a poisson disc through dart throwing
 * not efficient at all, but the number of samples expected is incredibly low (ie 12), so it wont be a problem
 */

void ShadowManager::fillPoissonDisc(size_t numSamples, float minDistance) {


    std::random_device randomDevice;
    std::mt19937 generator(randomDevice());


    std::uniform_real_distribution<float> randomDistribution(-1.f, 1.f);

    size_t count = 0;
    size_t attempts = 0;
    while (count < numSamples){

        reject_sample:
        attempts++;

        if (attempts > (numSamples * numSamples) + 1000){
            //we've been trying this too long, safe to assume we have impossible conditions (too many samples / not enough space)
            Logger::Print("Error - unable to fill shadow map poisson disc in a reasonable time, so decreasing min spacing\n",
                          LOGCAT::CAT_RENDERING, LOGSEV::SEV_WARNING);
            minDistance -= 0.04;
            attempts = 0;
            poissonDisc.clear();
            count = 0;
            continue;
        }

        glm::vec2 newSample;
        newSample.x = randomDistribution(generator);
        newSample.y = randomDistribution(generator);

        //check if generated sample is outside of disc
        if (glm::length(newSample) > 1){
            continue;
        }


        //check if generated sample is too close to an existing one
        for (auto sample : poissonDisc){
            if (glm::distance(sample, newSample) < minDistance){
                goto reject_sample;
            }
        }

        poissonDisc.push_back(newSample);
        ++count;
    }

}




/*
 * function "calculates" the shadows for the directional lights in the scene
 * it does this by partitioning the view frustum into sub frustums, then rendering from the pov of the light for
 * each frustum, and saves the created depth buffer to a texture
 * this texture can then be accessed for use in the main render pass
 */


void ShadowManager::calculateCascades(glm::mat4x4 viewMatrix) {

    if (renderManager->directionalLightsVector.size() * (cascadePartitions.size() + 1) > depth){
        Logger::Print("Error - trying to calculate shadows for more directional lights than space is allocated\n"
                      "Either add more depth, remove some lights, or reduce the amount of frustum partitions\n",
                      LOGCAT::CAT_RENDERING, LOGSEV::SEV_ERROR);
        exit(EXIT_FAILURE);
    }

    if (viewSpaceDepths.empty()){
        Logger::Print("Error - trying to calculate shadows without first setting the projection matrix\n"
                      "updateProjectionMatrix() must be called before shadows can be rendered\n",
                      LOGCAT::CAT_RENDERING, LOGSEV::SEV_ERROR);
        exit(EXIT_FAILURE);
    }

    lightTransforms.clear();

    size_t numCascades = cascadePartitions.size() + 1; //how many regions the frustum is split into
    glm::mat4x4 worldTransform = glm::inverse(viewMatrix);

    for (size_t i = 0; i < renderManager->directionalLightsVector.size(); i++) {

        for (size_t j = 0; j < numCascades; j++) {

            size_t index = (i * numCascades) + j;

            glm::mat4x4 lightTransform = calculateLightFrustumTransform(nearFrustumPlanes[index],
                                                                        farFrustumPlanes[index],
                                                                        renderManager->directionalLightsVector[i],
                                                                        worldTransform);

            lightTransforms.push_back(lightTransform);
        }
    }
}

void ShadowManager::renderShadows(glm::mat4x4 instanceMatrix) {

    GLuint oldProgram = renderManager->activeProgram;
    renderManager->swapProgram(renderManager->passThroughProgram);

    size_t i;
    for (i = 0; i < lightTransforms.size() - 1; i++){
        renderManager->shadowRender(lightTransforms[i], instanceMatrix, framebufferIDs[i]);
    }

    renderManager->activeProgram = oldProgram;
    renderManager->shadowRender(lightTransforms[i], instanceMatrix, framebufferIDs[i]);

}


void ShadowManager::updateProjectionMatrix(glm::mat4x4 &projectionMatrix) {

    if (projectionMatrix == activeProjectionMatrix) return;

    activeProjectionMatrix = projectionMatrix;
    viewSpaceDepths.clear();
    calculateFrustums();
}



