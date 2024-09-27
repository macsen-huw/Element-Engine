#include "Animator.hpp"

using namespace elmt;

Animator::Animator(const char* name, Entity* entity, Animation* animation) : Component(name, entity) {
    meshRenderer = dynamic_cast<MeshRenderer*>(entity->getComponentOfType("MeshRenderer"));
    if (!meshRenderer) {
        Logger::Print("[Error/Animator] " + this->getName() + " does not have a MeshRenderer Component.", LOGCAT::CAT_RENDERING, LOGSEV::SEV_ERROR);
        return;
    }

    currentTime = 0.0f;
    currentAnimation = animation;

    boneMatrices.reserve(meshRenderer->model->skeletonRig.numBones);
    for (int i = 0; i < meshRenderer->model->skeletonRig.numBones; i++)
        boneMatrices.push_back(glm::mat4(1.0f));

    instanceId = meshRenderer->getInstanceId();
    modelId = meshRenderer->getModelId();

    core::getAnimationManager()->AddAnimator(this);

    typeName = "Animator";
}

void Animator::SwitchAnimation(Animation* targetAnimation, float blendTime) {
    if (targetAnimation == currentAnimation) {
        Logger::Print("[Animator/SwitchAnimation]: Target Animation same as current animation", LOGCAT::CAT_RENDERING, LOGSEV::SEV_WARNING);
        return;
    }

    if (!isBlending) {
        if (blendTime > 0) {
            this->targetAnimation = targetAnimation;
            this->totalBlendTime = blendTime;
            this->isBlending = true;
            this->blendTime = 0.0f;
        }
        else {
            this->currentAnimation = targetAnimation;
            this->currentTime = 0.0f;
        }
    }
}

void Animator::UpdateAnimation(float deltaTime) {
    if (currentAnimation) {
        if (!isBlending) {
            currentTime += currentAnimation->ticsPerSecond * deltaTime;
            currentTime = fmod(currentTime, currentAnimation->animDuration);
            CalculateBoneTransform(&currentAnimation->assimpNodeData, glm::mat4(1.0f));
            updateInfo();
        }
        else {
            BlendTargetAnimation(deltaTime);
        }
    }
}

void Animator::BlendTargetAnimation(float dt)
{
    // Speed multipliers to correctly transition from one animation to another
    float a = 1.0f;
    float b = currentAnimation->animDuration / targetAnimation->animDuration;
    const float animSpeedMultiplierUp = (1.0f - blendFactor) * a + b * blendFactor; // Lerp

    a = targetAnimation->animDuration / currentAnimation->animDuration;
    b = 1.0f;
    const float animSpeedMultiplierDown = (1.0f - blendFactor) * a + b * blendFactor; // Lerp

    // Current time of each animation, "scaled" by the above speed multiplier variables
    static float currentTimeBase = 0.0f;
    currentTimeBase += currentAnimation->ticsPerSecond * dt * animSpeedMultiplierUp;
    currentTimeBase = fmod(currentTimeBase, currentAnimation->animDuration);

    static float currentTimeLayered = 0.0f;
    currentTimeLayered += targetAnimation->ticsPerSecond * dt * animSpeedMultiplierDown;
    currentTimeLayered = fmod(currentTimeLayered, targetAnimation->animDuration);

    CalculateBlendedBoneTransform(&currentAnimation->assimpNodeData, &targetAnimation->assimpNodeData, currentTimeLayered, glm::mat4(1.0f));
    updateInfo();

    // calculate blend timings
    blendTime += dt;
    blendFactor = blendTime / totalBlendTime;

    if (blendFactor >= 1.0f) {
        currentAnimation = targetAnimation;
        currentTime = currentTimeLayered;
        isBlending = false;

        targetAnimation = nullptr;
        blendFactor = 0.0f;
        blendTime = 0.0f;
    }
}

void Animator::CalculateBoneTransform(const Animation::NodeData* node, glm::mat4 parentTransform) {
    std::string nodeName = node->name;
    glm::mat4 nodeTransform = node->transformation;

    AnimBone* bone = currentAnimation->FindBone(nodeName);

    if (bone) {
        bone->Update(currentTime);
        nodeTransform = bone->localTransform;
    }

    // Fix scale
    const glm::quat rot = glm::quat_cast(nodeTransform);
    glm::mat4 fixedMat = glm::mat4_cast(rot);
    fixedMat[3] = nodeTransform[3];

    glm::mat4 globalTransformation = parentTransform * fixedMat;

    auto boneInfoMap = meshRenderer->model->skeletonRig.boneMap;
    if (boneInfoMap.find(nodeName) != boneInfoMap.end()) {
        int index = boneInfoMap[nodeName].boneId;
        glm::mat4 offset = boneInfoMap[nodeName].offsetMatrix;
        boneMatrices[index] = globalTransformation * offset;
    }

    for (int i = 0; i < node->childrenCount; i++)
        CalculateBoneTransform(&node->children[i], globalTransformation);
}

void Animator::CalculateBlendedBoneTransform(
    Animation::NodeData* node,
    const Animation::NodeData* targetNode,
    const float targetCurrentTime,
    const glm::mat4 parentTransform)
{
    const std::string& nodeName = node->name;

    glm::mat4 nodeTransform = node->transformation;
    AnimBone* pBone = currentAnimation->FindBone(nodeName);
    if (pBone)
    {
        pBone->Update(currentTime);
        nodeTransform = pBone->localTransform;
    }

    glm::mat4 layeredNodeTransform = targetNode->transformation;
    pBone = targetAnimation->FindBone(nodeName);
    if (pBone)
    {
        pBone->Update(targetCurrentTime);
        layeredNodeTransform = pBone->localTransform;
    }

    // Blend two matrices
    const glm::quat rot0 = glm::quat_cast(nodeTransform);
    const glm::quat rot1 = glm::quat_cast(layeredNodeTransform);
    const glm::quat finalRot = glm::slerp(rot0, rot1, blendFactor);
    glm::mat4 blendedMat = glm::mat4_cast(finalRot);
    blendedMat[3] = (1.0f - blendFactor) * nodeTransform[3] + layeredNodeTransform[3] * blendFactor;

    glm::mat4 globalTransformation = parentTransform * blendedMat;

    auto boneInfoMap = meshRenderer->model->skeletonRig.boneMap;
    if (boneInfoMap.find(nodeName) != boneInfoMap.end())
    {
        int index = boneInfoMap[nodeName].boneId;
        glm::mat4 offset = boneInfoMap[nodeName].offsetMatrix;
        boneMatrices[index] = globalTransformation * offset;
    }

    for (size_t i = 0; i < std::max(node->childrenCount, targetNode->childrenCount); i++) {
        if (i >= node->childrenCount) CalculateBoneTransform(&targetNode->children[i], globalTransformation);
        else if (i >= targetNode->childrenCount) CalculateBoneTransform(&node->children[i], globalTransformation);
        else CalculateBlendedBoneTransform(&node->children[i], &targetNode->children[i], targetCurrentTime, globalTransformation);
    }
}

void Animator::updateInfo() {
    InstanceProperties newProperties{};
    newProperties.translationMatrix = glm::mat4x4(1.f);
    newProperties.translationMatrix[3][0] = this->getEntity()->pos.x;
    newProperties.translationMatrix[3][1] = this->getEntity()->pos.y;
    newProperties.translationMatrix[3][2] = this->getEntity()->pos.z;

    glm::mat4x4 scaleMatrix = glm::mat4x4(1.f);
    scaleMatrix[0][0] = this->getEntity()->scale.x;
    scaleMatrix[1][1] = this->getEntity()->scale.y;
    scaleMatrix[2][2] = this->getEntity()->scale.z;

    newProperties.rotationMatrix = this->getEntity()->rotation * scaleMatrix;

    newProperties.animBoneMatrices = boneMatrices;

    newProperties.active = true;
    core::getRenderManager()->updateInstance(newProperties, modelId, instanceId);
}

double Animator::getPlaybackPercentage() {
    if (currentAnimation) {
        if (isBlending) return 0.0f;
        return fmod((currentTime / currentAnimation->animDuration), 1.0f);
    }
    else {
        return 0.0f;
    }
}

Animator::~Animator() {
    Component::~Component();
}