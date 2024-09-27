#include "UITextColorEditorComponent.hpp"
#include "UIManager.hpp"

using namespace elmt;

UITextColorEditorComponent::UITextColorEditorComponent()
{
	typeName = "UITextColorEditorComponent";
}

UITextColorEditorComponent::UITextColorEditorComponent(const char* name, Entity* entity, bool isFixed) : UIComponent(name, entity, isFixed)
{
	typeName = "UITextColorEditorComponent";
	text = name;
}

void UITextColorEditorComponent::FixedUpdate(RectTransformComponent* rectTransform)
{
	ImGui::ColorEdit4("##TextColor", (float*)&UIManager::globalTextColor);
}

void UITextColorEditorComponent::DynamicUpdate()
{
	ImGui::ColorEdit4("##TextColor", (float*)&UIManager::globalTextColor);
}

