#pragma once
#include "UIComponent.hpp"

namespace elmt {

	class UISpacingComponent : public UIComponent
	{
	public:
		UISpacingComponent();
		UISpacingComponent(const char* name, Entity* entity, bool isFixed);
		virtual void FixedUpdate(RectTransformComponent* rectTransform);
		virtual void DynamicUpdate();
	};

}