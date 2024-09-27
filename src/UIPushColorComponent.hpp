#pragma once
#include "UIComponent.hpp"

namespace elmt {

	class UIPushColorComponent : public UIComponent
	{
	public:
		UIPushColorComponent();
		UIPushColorComponent(const char* name, Entity* entity, bool isFixed);
		virtual void FixedUpdate(RectTransformComponent* rectTransform);
		virtual void DynamicUpdate();
	};

}