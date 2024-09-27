#pragma once
#include "UIComponent.hpp"

namespace elmt {

	class UILabelComponent : public UIComponent
	{
	public:
		UILabelComponent();
		UILabelComponent(const char* name, Entity* entity, std::string label, bool isFixed);
		virtual void FixedUpdate(RectTransformComponent* rectTransform);
		virtual void DynamicUpdate();
	};

}