#include "UIPopColorComponent.hpp"

elmt::UIPopColorComponent::UIPopColorComponent()
{
	typeName = "UIPopColorComponent";
}

elmt::UIPopColorComponent::UIPopColorComponent(const char* name, Entity* entity, bool isFixed) : UIComponent(name, entity, isFixed)
{
	typeName = "UIPopColorComponent";
}

void elmt::UIPopColorComponent::FixedUpdate(RectTransformComponent* rectTransform)
{
	ImGui::PopStyleColor(1);
}

void elmt::UIPopColorComponent::DynamicUpdate()
{
	ImGui::PopStyleColor(1);
}
