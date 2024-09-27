#pragma once

#include "Component.hpp"
#include "Entity.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace elmt {

	// The RectTransformComponent class is used to define the position, rotationAngle, and size (width, height) of UI entities in screen space.
	class RectTransformComponent : public Component {
	public:
		glm::vec2 position;
		float     rotationAngle = 0;
		glm::vec2 size;

		RectTransformComponent();
		RectTransformComponent(const glm::vec2& position, float rotationAngle);
		RectTransformComponent(const glm::vec2& position, float rotationAngle, const glm::vec2& size);

		virtual void clone(Component*& clonePointer, Entity* entityToAttach);
	};

}