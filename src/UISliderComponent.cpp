#include "UISliderComponent.hpp"
using namespace elmt;
UISliderComponent::UISliderComponent()
{
	typeName = "UISliderComponent";
}
UISliderComponent::UISliderComponent(const char* name, Entity* entity, bool isFixed) : UIComponent(name, entity, isFixed)
{
	typeName = "UISliderComponent";
	text = name;
}

void UISliderComponent::FixedUpdate(RectTransformComponent* rectTransform)
{
	if (ImGui::SliderInt(text.c_str(), &currentValue, minValue, maxValue)) {
		if (callbackValueChanged) 
			callbackValueChanged(this);
	}
}

void UISliderComponent::DynamicUpdate()
{
	if (ImGui::SliderInt(text.c_str(), &currentValue, minValue, maxValue)) {
		if (callbackValueChanged) 
			callbackValueChanged(this);
	}
}
