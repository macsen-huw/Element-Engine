#include "UISeparatorComponent.hpp"

using namespace elmt;

UISeparatorComponent::UISeparatorComponent()
{
	typeName = "UISeparatorComponent";
}

UISeparatorComponent::UISeparatorComponent(const char* name, Entity* entity, std::string label, bool isFixed) : UIComponent(name, entity, isFixed)
{
	typeName = "UISeparatorComponent";
	text = label;
}

void UISeparatorComponent::FixedUpdate(RectTransformComponent* rectTransform)
{
	ImGui::SeparatorText(text.c_str());
}

void UISeparatorComponent::DynamicUpdate()
{
	ImGui::SeparatorText(text.c_str());
}
