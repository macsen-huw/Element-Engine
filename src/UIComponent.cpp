#include "UIComponent.hpp"
#include "core.hpp"
#include "RenderManager.hpp"
#include "UIManager.hpp"

using namespace elmt;

UIComponent::UIComponent()
{
	typeName = "UIComponent";
}

UIComponent::UIComponent(const char* name, Entity* entity, bool isFixed) :Component(name, entity)
{
	typeName = "UIComponent";
	this->isFixed = isFixed;
}

bool UIComponent::Update()
{
	if (isFixed)
	{
		auto rectTransform = dynamic_cast<RectTransformComponent*>(getEntity()->getComponentOfType("RectTransformComponent"));
		ImGui::SetCursorPos(ImVec2(rectTransform->position.x, rectTransform->position.y));


		ImGui::PushFont(uiFont ? uiFont->imFont : NULL);
		ImGui::PushStyleColor(ImGuiCol_Text, textColor);
		ImGui::SetNextItemWidth(rectTransform->size.x);
		FixedUpdate(rectTransform);
		ImGui::PopStyleColor(1);
		ImGui::PopFont();

		return true;
	}
	else
	{
		ImGui::PushFont(uiFont ? uiFont->imFont : NULL);
		DynamicUpdate();
		ImGui::PopFont();
		return true;
	}
}

void	UIComponent::SetFont(const char* fontFile, float fontSize)
{
	elmt::UIManager* uiMgr = elmt::core::getRenderManager()->getUIManager();
	uiFont = uiMgr->GetFont(fontFile, fontSize);

}
void	UIComponent::SetTextColor(const ImVec4& col)
{
	textColor = col;
}
