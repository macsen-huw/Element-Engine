#pragma once

#include "../external/glm-0.9.7.1/glm/glm.hpp"
#include "../external/glm-0.9.7.1/glm/gtc/matrix_transform.hpp"
#include "../external/glm-0.9.7.1/glm/gtx/euler_angles.hpp"
#include "Entity.hpp"
#include "BillboardMesh.hpp"
#include "RenderManager.hpp"
#include "Group.hpp"

namespace elmt {
	class BillboardInstance : public Entity
	{
	private:
		BillboardMesh* billboardMesh;
		size_t instanceID;

	public:
		glm::vec3 position;
		glm::mat4 modelTransform;

		BillboardInstance(const char* name, Entity* parent, glm::vec3 pos, BillboardMesh* billboardMesh);
		void update();
		bool Render();
	};
}