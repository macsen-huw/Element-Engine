#pragma once
#include "UIComponent.hpp"

namespace elmt {

	class UIPopColorComponent : public UIComponent
	{
	public:
		UIPopColorComponent();
		UIPopColorComponent(const char* name, Entity* entity, bool isFixed);
		virtual void FixedUpdate(RectTransformComponent* rectTransform);
		virtual void DynamicUpdate();
	};

}