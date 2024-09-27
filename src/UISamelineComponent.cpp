#include "UISamelineComponent.hpp"

elmt::UISamelineComponent::UISamelineComponent()
{
	typeName = "UISpacingComponent";
}

elmt::UISamelineComponent::UISamelineComponent(const char* name, Entity* entity, bool isFixed) : UIComponent(name, entity, isFixed)
{
	typeName = "UISamelineComponent";
}

void elmt::UISamelineComponent::FixedUpdate(RectTransformComponent* rectTransform)
{
	ImGui::SameLine();
}

void elmt::UISamelineComponent::DynamicUpdate()
{
	ImGui::SameLine();
}
