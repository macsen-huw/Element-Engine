#include "UIComboComponent.hpp"

using namespace elmt;
UIComboComponent::UIComboComponent()
{
	typeName = "UIComboComponent";
}

UIComboComponent::UIComboComponent(const char* name, Entity* entity, bool isFixed) :UIComponent(name, entity, isFixed)
{
	typeName = "UIComboComponent";
	text = name;
}

void UIComboComponent::FixedUpdate(RectTransformComponent* rectTransform)
{
	if (ImGui::Combo(text.c_str(), &currentItem, textItems.c_str())) {
		if (callbackItemSelected) 
			callbackItemSelected(this);
	}
}

void UIComboComponent::DynamicUpdate()
{
	if (ImGui::Combo(text.c_str(), &currentItem, textItems.c_str())) {
		if (callbackItemSelected) 
			callbackItemSelected(this);
	}
}

void	UIComboComponent::SetItems(const char* items[], int itemCount)
{
	textItems = "";
	for (int i = 0; i < itemCount; i++) {
		textItems += items[i];
		textItems += '\0';
	}
	textItems += '\0';
}


