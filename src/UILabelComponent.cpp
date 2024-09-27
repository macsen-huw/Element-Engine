#include "UILabelComponent.hpp"

using namespace elmt;

UILabelComponent::UILabelComponent()
{
	typeName = "UILabelComponent";
}

UILabelComponent::UILabelComponent(const char* name, Entity* entity, std::string label, bool isFixed) : UIComponent(name, entity, isFixed)
{
	typeName = "UILabelComponent";
	text = label;
}

void UILabelComponent::FixedUpdate(RectTransformComponent* rectTransform)
{
	ImGui::Text(text.c_str());
}

void UILabelComponent::DynamicUpdate()
{
	ImGui::Text(text.c_str());
}
