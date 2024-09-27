#pragma once
#include "UIComponent.hpp"

namespace elmt {

	class UISamelineComponent : public UIComponent
	{
	public:
		UISamelineComponent();
		UISamelineComponent(const char* name, Entity* entity, bool isFixed);
		virtual void FixedUpdate(RectTransformComponent* rectTransform);
		virtual void DynamicUpdate();
	};

}