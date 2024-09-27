#pragma once

#include "Entity.hpp"
#include "core.hpp"

#include "glm.hpp"
#include <GLFW/glfw3.h>
#include "../external/glm-0.9.7.1/glm/glm.hpp"


namespace elmt {
    class Entity;

	class Camera :
		public Entity
	{
		// Properties
	public:

		glm::mat4x4 projectionMatrix;
		glm::mat4x4 viewMatrix;
		glm::vec3 viewDir;
		glm::ivec2 screenSize = glm::ivec2(1920, 1080);

		// Whether not the camera can have any roll applied to it's rotation
		bool rollEnabled = false;

	private:
		bool active = true;
        float time;
        float fov;
        

		// Methods
	public:
		Camera(const char* name, Entity* parent, glm::vec3 pos, bool active);
		Camera(const char* name, Entity* parent, glm::vec3 pos);
		// Set whether or not this camera is the currently active camera
		void setActive(bool val);

		bool Update();


	};

}
