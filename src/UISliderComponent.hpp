#pragma once
#include "UIComponent.hpp"
#include <functional>

namespace elmt {

	class UISliderComponent : public UIComponent
	{
	public:
		UISliderComponent();
		UISliderComponent(const char* name, Entity* entity, bool isFixed);
		virtual void FixedUpdate(RectTransformComponent* rectTransform);
		virtual void DynamicUpdate();
	public:
		int currentValue = 50;
		int minValue = 0;
		int maxValue = 100;
		std::function<void(UISliderComponent* slider)> callbackValueChanged;
	};

}
