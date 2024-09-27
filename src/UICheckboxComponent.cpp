#include "UICheckboxComponent.hpp"

using namespace elmt;
UICheckboxComponent::UICheckboxComponent()
{
	typeName = "UICheckboxComponent";
}
UICheckboxComponent::UICheckboxComponent(const char* name, Entity* entity, bool isFixed) : UIComponent(name, entity, isFixed)
{
	typeName = "UICheckboxComponent";
	text = name;
}

void UICheckboxComponent::FixedUpdate(RectTransformComponent* rectTransform)
{
	if (ImGui::Checkbox(text.c_str(), &isCheck)) {
		if (callbackCheck) 
			callbackCheck(this);
	}
}

void UICheckboxComponent::DynamicUpdate()
{
	if (ImGui::Checkbox(text.c_str(), &isCheck)) {
		if (callbackCheck) 
			callbackCheck(this);
	}
}
