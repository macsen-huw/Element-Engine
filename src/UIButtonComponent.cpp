#include "UIButtonComponent.hpp"

using namespace elmt;

UIButtonComponent::UIButtonComponent()
{
	typeName = "UIButtonComponent";
}

UIButtonComponent::UIButtonComponent(const char* name, Entity* entity, std::string label, bool isFixed) : UIComponent(name, entity, isFixed)
{
	typeName = "UIButtonComponent";
	text = label;
}

void UIButtonComponent::FixedUpdate(RectTransformComponent* rectTransform)
{
	if (ImGui::Button(text.c_str(), ImVec2(rectTransform->size.x, rectTransform->size.y))) {
		if (callbackClick) 
			callbackClick(this);
	}
}

void UIButtonComponent::DynamicUpdate()
{
	if (ImGui::Button(text.c_str())) {
		if (callbackClick)
			callbackClick(this);
	}
}
