#include "UISpacingComponent.hpp"

elmt::UISpacingComponent::UISpacingComponent()
{
	typeName = "UISpacingComponent";
}

elmt::UISpacingComponent::UISpacingComponent(const char* name, Entity* entity, bool isFixed) : UIComponent(name, entity, isFixed)
{
	typeName = "UISpacingComponent";
}

void elmt::UISpacingComponent::FixedUpdate(RectTransformComponent* rectTransform)
{
	ImGui::Spacing();
	ImGui::Spacing();
}

void elmt::UISpacingComponent::DynamicUpdate()
{
	ImGui::Spacing();
	ImGui::Spacing();
}
