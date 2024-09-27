#pragma once
#include "string"
#include "vector"
#include "glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"
#include <assimp/scene.h>


namespace elmt {
    class AnimBone {
    private:
        struct VectorKey
        {
            glm::vec3 vector;
            float timeStamp;
        };

        struct QuaternionKey
        {
            glm::quat quaternion;
            float timeStamp;
        };

        std::vector<VectorKey> positionKeys;
        std::vector<QuaternionKey> rotationKeys;
        std::vector<VectorKey> scaleKeys;

    public:
        std::string name;
        std::uint32_t boneId;

        glm::mat4 localTransform;

    public:
        AnimBone(const std::string& name, std::uint32_t ID, const aiNodeAnim* channel) : name(name), boneId(ID), localTransform(1.0f) {
            for (int positionIndex = 0; positionIndex < channel->mNumPositionKeys; ++positionIndex)
            {
                aiVector3D position = channel->mPositionKeys[positionIndex].mValue;
                float timeStamp = channel->mPositionKeys[positionIndex].mTime;
                VectorKey positionKey;
                positionKey.vector = glm::vec3(position.x, position.y, position.z);
                positionKey.timeStamp = timeStamp;
                positionKeys.push_back(positionKey);
            }

            for (int rotationIndex = 0; rotationIndex < channel->mNumRotationKeys; ++rotationIndex)
            {
                aiQuaternion rotation = channel->mRotationKeys[rotationIndex].mValue;
                float timeStamp = channel->mRotationKeys[rotationIndex].mTime;
                QuaternionKey rotationKey;
                rotationKey.quaternion = glm::quat(rotation.w, rotation.x, rotation.y, rotation.z);
                rotationKey.timeStamp = timeStamp;
                rotationKeys.push_back(rotationKey);
            }

            for (int keyIndex = 0; keyIndex < channel->mNumScalingKeys; ++keyIndex)
            {
                aiVector3D scale = channel->mScalingKeys[keyIndex].mValue;
                float timeStamp = channel->mScalingKeys[keyIndex].mTime;
                VectorKey scaleKey;
                scaleKey.vector = glm::vec3(scale.x, scale.y, scale.z);
                scaleKey.timeStamp = timeStamp;
                scaleKeys.push_back(scaleKey);
            }
        }

        void Update(float animationTime) {
            glm::mat4 translation = InterpolatePosition(animationTime);
            glm::mat4 rotation = InterpolateRotation(animationTime);
            glm::mat4 scale = InterpolateScaling(animationTime);
            localTransform = translation * rotation * scale;
        }

        int GetPositionIndex(float animationTime) {
            for (int index = 0; index < positionKeys.size() - 1; ++index) {
                if (animationTime < positionKeys[index + 1].timeStamp)
                    return index;
            }
            assert(0);
        }

        int GetRotationIndex(float animationTime) {
            for (int index = 0; index < rotationKeys.size() - 1; ++index) {
                if (animationTime < rotationKeys[index + 1].timeStamp)
                    return index;
            }
            assert(0);
        }

        int GetScaleIndex(float animationTime) {
            for (int index = 0; index < scaleKeys.size() - 1; ++index) {
                if (animationTime < scaleKeys[index + 1].timeStamp)
                    return index;
            }
            assert(0);
        }

    private:
        float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime) {
            float scaleFactor = 0.0f;
            float midWayLength = animationTime - lastTimeStamp;
            float framesDiff = nextTimeStamp - lastTimeStamp;
            scaleFactor = midWayLength / framesDiff;
            return scaleFactor;
        }

        glm::mat4 InterpolatePosition(float animationTime) {
            if (1 == positionKeys.size())
                return glm::translate(glm::mat4(1.0f), positionKeys[0].vector);

            int p0Index = GetPositionIndex(animationTime);
            int p1Index = p0Index + 1;
            float scaleFactor = GetScaleFactor(positionKeys[p0Index].timeStamp, positionKeys[p1Index].timeStamp, animationTime);
            glm::vec3 finalPosition = glm::mix(positionKeys[p0Index].vector, positionKeys[p1Index].vector, scaleFactor);
            return glm::translate(glm::mat4(1.0f), finalPosition);
        }

        glm::mat4 InterpolateRotation(float animationTime) {
            if (1 == rotationKeys.size()) {
                auto rotation = glm::normalize(rotationKeys[0].quaternion);
                return glm::toMat4(rotation);
            }

            int p0Index = GetRotationIndex(animationTime);
            int p1Index = p0Index + 1;
            float scaleFactor = GetScaleFactor(rotationKeys[p0Index].timeStamp, rotationKeys[p1Index].timeStamp, animationTime);
            glm::quat finalRotation = glm::slerp(rotationKeys[p0Index].quaternion, rotationKeys[p1Index].quaternion, scaleFactor);
            finalRotation = glm::normalize(finalRotation);
            return glm::toMat4(finalRotation);
        }

        glm::mat4 InterpolateScaling(float animationTime) {
            if (1 == scaleKeys.size())
                return glm::scale(glm::mat4(1.0f), scaleKeys[0].vector);

            int p0Index = GetScaleIndex(animationTime);
            int p1Index = p0Index + 1;
            float scaleFactor = GetScaleFactor(scaleKeys[p0Index].timeStamp, scaleKeys[p1Index].timeStamp, animationTime);
            glm::vec3 finalScale = glm::mix(scaleKeys[p0Index].vector, scaleKeys[p1Index].vector, scaleFactor);
            return glm::scale(glm::mat4(1.0f), finalScale);
        }
    };
}