#include "UIInputFloat3Component.hpp"

using namespace elmt;

UIInputFloat3Component::UIInputFloat3Component()
{
	typeName = "UIInputFloat3Component";
}
UIInputFloat3Component::UIInputFloat3Component(const char* name, Entity* entity, float* values, bool isFixed) : UIComponent(name, entity, isFixed)
{
	typeName = "UIInputFloat3Component";
	text = name;
	this->values = values;
}

void UIInputFloat3Component::FixedUpdate(RectTransformComponent* rectTransform)
{
	ImGui::InputFloat3(text.c_str(), values);
}

void UIInputFloat3Component::DynamicUpdate()
{
	ImGui::InputFloat3(text.c_str(), values);
}
