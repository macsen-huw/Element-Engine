#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <glm/glm.hpp>

namespace elmt {
	struct Bone
	{
		std::uint16_t boneId;
		glm::mat4 offsetMatrix;
	};

	class SkeletonRig {
	public:
		std::map<std::string, Bone> boneMap;
		int numBones = 0;
	};
}