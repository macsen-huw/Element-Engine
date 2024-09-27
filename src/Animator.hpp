#pragma once
#include "vector"
#include "glm.hpp"

#include "Entity.hpp"
#include "Component.hpp"
#include "Animation.hpp"
#include "AnimBone.hpp"
#include "RenderManager.hpp"
#include "MeshRenderer.hpp"
#include "AnimationManager.hpp"

namespace elmt {
	class MeshRenderer;
	class Animator : public Component
	{
	private:
		size_t instanceId = -1;
		size_t modelId = -1;
		MeshRenderer* meshRenderer = nullptr;

		Animation* targetAnimation = nullptr;
		float blendFactor = 0.0f;
		double blendTime = 0.0f;
		double totalBlendTime = 0.0f;

		std::vector<glm::mat4> boneMatrices;

		float currentTime = 0.0f;
		bool isBlending = false;

	public:
		Animation* currentAnimation = nullptr;

		Animator(const char* name, Entity* entity, Animation* animation);

		void SwitchAnimation(Animation* targetAnimation, float blendTime = 0.0f);

		double getPlaybackPercentage();

		~Animator();

	protected:
		void UpdateAnimation(float dt);

		friend class AnimationManager;

	private:
		void updateInfo();

		void CalculateBoneTransform(const Animation::NodeData* node, glm::mat4 parentTransform);

		void CalculateBlendedBoneTransform(
			Animation::NodeData* node,
			const Animation::NodeData* targetNode,
			const float targetCurrentTime,
			const glm::mat4 parentTransform
		);

		void BlendTargetAnimation(float dt);
	};
}