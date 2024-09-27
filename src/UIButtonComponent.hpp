#pragma once
#include "UIComponent.hpp"
#include <functional>
namespace elmt {

	class UIButtonComponent : public UIComponent
	{
	public:
		UIButtonComponent();
		UIButtonComponent(const char* name, Entity* entity, std::string label, bool isFixed);
		virtual void FixedUpdate(RectTransformComponent* rectTransform);
		virtual void DynamicUpdate();
		std::function<void(UIButtonComponent* btn)> callbackClick;

	};

}