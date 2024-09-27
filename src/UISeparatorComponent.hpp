#pragma once
#include "UIComponent.hpp"

namespace elmt {

	class UISeparatorComponent : public UIComponent
	{
	public:
		UISeparatorComponent();
		UISeparatorComponent(const char* name, Entity* entity, std::string label, bool isFixed);
		virtual void FixedUpdate(RectTransformComponent* rectTransform);
		virtual void DynamicUpdate();
	};

}