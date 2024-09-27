#pragma once
#include "UIComponent.hpp"
#include <functional>

namespace elmt {
	class UICheckboxComponent : public UIComponent
	{
	public:
		UICheckboxComponent();
		UICheckboxComponent(const char* name, Entity* entity, bool isFixed);
		virtual void FixedUpdate(RectTransformComponent* rectTransform);
		virtual void DynamicUpdate();

		bool isCheck = false;	
		std::function<void(UICheckboxComponent* chk)> callbackCheck;
	};

}