#pragma once
#include <vector>
#include <GL/glew.h>
#include "glm.hpp"

#include "SkeletonRig.hpp"

namespace elmt {
#define MAX_BONE_VERTEX_INFLUENCE 4

	struct Vertex {
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoords;

		// Bone Data
        int BoneIds[MAX_BONE_VERTEX_INFLUENCE] = { -1, -1, -1, -1 };
        float Weights[MAX_BONE_VERTEX_INFLUENCE] = { 0.f };
	};

	class Mesh{
        public:
            std::vector<Vertex> vertices;
            std::vector<unsigned int> indices;

            std::uint32_t materialIndex;

            //TODO improve
            std::vector<glm::vec3> verts;

            Mesh() = default;

            Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::uint32_t materialIndex) {
                this->vertices = vertices;
                this->indices = indices;
                this->materialIndex = materialIndex;

                for (Vertex& v : vertices) {
                    verts.push_back(v.Position);
                }

            }
        };
}

