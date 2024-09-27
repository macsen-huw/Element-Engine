#pragma once
#include "string"
#include "vector"
#include "glm.hpp"
#include "gtc/quaternion.hpp"

#include "AnimBone.hpp"
#include "SkeletonRig.hpp"

namespace elmt {

	class Animation {
	public:	
		struct NodeData {
			glm::mat4 transformation;
			std::string name;
			int childrenCount;
			std::vector<NodeData> children;

		};

	public:
		float animDuration;
		int ticsPerSecond;

		glm::mat4 globalInverse;

		std::vector<AnimBone> bones;

		NodeData assimpNodeData;
		SkeletonRig skeletonRig;

		AnimBone* FindBone(const std::string& name) {
			auto iter = std::find_if(bones.begin(), bones.end(),
				[&](const AnimBone& bone)
				{
					return bone.name == name;
				}
			);
			if (iter == bones.end()) return nullptr;
			else return &(*iter);
		}

		float getDurationInSeconds() {
			return animDuration / ticsPerSecond;
		}
	};
}