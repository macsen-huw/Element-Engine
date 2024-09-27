#include "UIPushColorComponent.hpp"
#include "UIManager.hpp"

using namespace elmt;

extern ImVec4 globalTextColor;

UIPushColorComponent::UIPushColorComponent()
{
	typeName = "UIPushColorComponent";
}

UIPushColorComponent::UIPushColorComponent(const char* name, Entity* entity, bool isFixed) : UIComponent(name, entity, isFixed)
{
	typeName = "UIPushColorComponent";
	text = name;
}

void UIPushColorComponent::FixedUpdate(RectTransformComponent* rectTransform)
{
	ImGui::PushStyleColor(ImGuiCol_Text, UIManager::globalTextColor);
}

void UIPushColorComponent::DynamicUpdate()
{
	ImGui::PushStyleColor(ImGuiCol_Text, UIManager::globalTextColor);
}

