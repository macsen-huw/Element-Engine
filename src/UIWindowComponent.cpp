#include "UIWindowComponent.hpp"
using namespace elmt;

UIWindowComponent::UIWindowComponent()
{
	typeName = "UIWindowComponent";
}
UIWindowComponent::UIWindowComponent(const char* name, Entity* entity, bool isFixed, std::string label) : UIComponent(name, entity, isFixed)
{
	typeName = "UIWindowComponent";
	this->text = label;
	this->name = name;
}

UIWindowComponent::UIWindowComponent(const char* name, Entity* entity, bool isFixed, std::string label, ImVec2 pos, ImVec2 size,
	ImVec4 bgColor) : UIComponent(name, entity, isFixed)
{
	typeName = "UIWindowComponent";
	this->pos = pos;
	this->size = size;
	this->bgColor = bgColor;
	this->text = label;
	this->name = name;
}

void UIWindowComponent::Begin()
{
	if (isFixed)
	{
		Entity* e = getEntity();
		
		auto rectTransform = dynamic_cast<RectTransformComponent*>(e->getComponentOfType("RectTransformComponent"));
		ImGui::SetNextWindowPos(ImVec2(rectTransform->position.x, rectTransform->position.y), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(rectTransform->size.x, rectTransform->size.y), ImGuiCond_FirstUseEver);
		ImGui::PushStyleColor(ImGuiCol_Text, textColor);

		ImGui::Begin(text.c_str(), 0/*, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove*/);
	}
	else
	{
		ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
		ImGui::SetNextWindowSize(size, ImGuiCond_Always);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, bgColor);
		ImGui::PushStyleColor(ImGuiCol_Text, textColor);

		ImGui::Begin(text.c_str(), 0/*, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove*/);
	}

}

void UIWindowComponent::End()
{
	if (isFixed)
	{
		ImGui::End();
		ImGui::PopStyleColor(1);
	}
	else
	{
		ImGui::End();
		ImGui::PopStyleColor(2);
	}
}

void UIWindowComponent::SetPos(const ImVec2& newPos)
{
	this->pos = newPos;
}

void UIWindowComponent::SetSize(const ImVec2& newSize)
{
	this->size = newSize;
}
