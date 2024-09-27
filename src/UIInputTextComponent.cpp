#include "UIInputTextComponent.hpp"

using namespace elmt;

UIInputTextComponent::UIInputTextComponent()
{
	typeName = "UIInputTextComponent";
}
UIInputTextComponent::UIInputTextComponent(const char* name, Entity* entity, bool isFixed) : UIComponent(name, entity, isFixed)
{
	typeName = "UIInputTextComponent";
	text = name;
}

void UIInputTextComponent::FixedUpdate(RectTransformComponent* rectTransform)
{
	if (ImGui::InputText(text.c_str(), textBuffer, IM_ARRAYSIZE(textBuffer))) {
		if (callbackTextChanged) callbackTextChanged(this);
	}
}

void UIInputTextComponent::DynamicUpdate()
{
	if (ImGui::InputText(text.c_str(), textBuffer, IM_ARRAYSIZE(textBuffer))) {
		if (callbackTextChanged) callbackTextChanged(this);
	}
}

void	UIInputTextComponent::SetText(const char* inputText)
{
	strncpy(textBuffer, inputText, IM_ARRAYSIZE(textBuffer) - 1);
}
std::string UIInputTextComponent::GetText()
{
	std::string str = textBuffer;
	return str;
}

