#pragma once

#include "Entity.hpp"
#include "core.hpp"
#include "RectTransformComponent.hpp"
#include <imgui.h>

namespace elmt {

	class UIEntity : public Entity {
	public:
		UIEntity(const char* name, Entity* parent);
		UIEntity(const char* name, Entity* parent, glm::vec2 pos);
		UIEntity(const char* name, Entity* parent, glm::vec2 pos, glm::vec2 size);
		virtual bool Update() override;
		void	SetPosition(float x, float y);
		void	SetSize(float w, float h);
	};

}