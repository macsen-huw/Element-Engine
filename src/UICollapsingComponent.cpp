#include "UICollapsingComponent.hpp"

using namespace elmt;

UICollapsingComponent::UICollapsingComponent()
{
	typeName = "UICollapsingComponent";
}

UICollapsingComponent::UICollapsingComponent(const char* name, Entity* entity, std::string label, bool isFixed)
	: UIComponent(name, entity, isFixed)
{
	typeName = "UICollapsingComponent";
	text = label;
}

UICollapsingComponent::UICollapsingComponent(const char* name, Entity* entity, std::string label, Entity* winEntity, bool isFixed)
	: UIComponent(name, entity, isFixed)
{
	typeName = "UICollapsingComponent";
	text = label;
	this->winEntity = winEntity;
}

UICollapsingComponent::UICollapsingComponent(const char* name, Entity* entity, std::string label, Component* comp, bool isFixed)
	: UIComponent(name, entity, isFixed)
{
	typeName = "UICollapsingComponent";
	text = label;
	this->winEntity = entity;
	this->currentComp = comp;
}


void UICollapsingComponent::FixedUpdate(RectTransformComponent* rectTransform)
{
	if (ImGui::CollapsingHeader(text.c_str()))
	{
		if (callbackOnToggle)
			callbackOnToggle(this, winEntity);
		if (callbackShowComponents)
			callbackShowComponents(this, winEntity, currentComp);
	}

}

void UICollapsingComponent::DynamicUpdate()
{
	if (ImGui::CollapsingHeader(text.c_str()))
	{
		if (callbackOnToggle)
			callbackOnToggle(this, winEntity);
		if (callbackShowComponents)
			callbackShowComponents(this, winEntity, currentComp);
	}
}

std::string elmt::UICollapsingComponent::getText() const
{
	return text;
}
