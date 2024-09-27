#include "BillboardMesh.hpp"
#include <cstdio>
#include <cstdlib>
#include <unordered_set>
#include <cassert>
#include "../external/rapidobj.hpp"

#define STB_IMAGE_STATIC
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

using namespace elmt;

BillboardMesh::BillboardMesh(float width, float height, RenderManager* renderManager)
{
//	// x, y, z, u, v, nx, ny, nz
//	attributes = {
//		0.0f, -width / 2.0f,  height / 2.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f,
//		0.0f, -width / 2.0f, -height / 2.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
//		0.0f,  width / 2.0f, -height / 2.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f,
//		0.0f, -width / 2.0f,  height / 2.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f,
//		0.0f,  width / 2.0f, -height / 2.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f,
//		0.0f,  width / 2.0f,  height / 2.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
//	};
//
//	vCount = 6;
//
//	for (unsigned int i = 0; i < attributes.size(); i + 8)
//	{
//		glm::vec3 vertex = {
//					attributes[i + 0],
//					attributes[i + 1],
//					attributes[i + 2],
//		};
//		billboardMesh.vertices.push_back(vertex);
//
//		glm::vec2 texture = {
//					attributes[i + 3],
//					attributes[i + 4],
//		};
//		billboardMesh.texCoords.push_back(texture);
//
//		glm::vec3 normal = {
//					attributes[i + 5],
//					attributes[i + 6],
//					attributes[i + 7],
//		};
//		billboardMesh.normals.push_back(normal);
//	}
}




