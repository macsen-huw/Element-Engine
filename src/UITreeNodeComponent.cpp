#include "UITreeNodeComponent.hpp"

using namespace elmt;

UITreeNodeComponent::UITreeNodeComponent()
{
	typeName = "UITreeNodeComponent";
}

UITreeNodeComponent::UITreeNodeComponent(const char* name, Entity* entity, bool isFixed) : UIComponent(name, entity, isFixed)
{
	typeName = "UITreeNodeComponent";
	text = name;
}

UITreeNodeComponent::UITreeNodeComponent(const char* name, Entity* entity, bool isFixed, Entity* root) : UIComponent(name, entity, isFixed)
{
	typeName = "UITreeNodeComponent";
	rootEntity = root;
	text = name;
}

void UITreeNodeComponent::DynamicUpdate()
{
	ImGuiTreeNodeFlags flag = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow;
	CreateEntityTree(rootEntity, flag);
}

void UITreeNodeComponent::FixedUpdate(RectTransformComponent* rectTransform)
{
	ImGuiTreeNodeFlags flag = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow;
	CreateEntityTree(rootEntity, flag);
}

void UITreeNodeComponent::CreateEntityTree(Entity* entity, ImGuiTreeNodeFlags flags)
{
	ImGuiTreeNodeFlags flag = ImGuiTreeNodeFlags_None;
	if (entity->getChildren().size() == 0) flag |= ImGuiTreeNodeFlags_Leaf;
	else flag |= ImGuiTreeNodeFlags_OpenOnArrow;
	
	bool nodeOpen = ImGui::TreeNodeEx(entity->getName().c_str(), flag | flags);

	if (ImGui::IsItemClicked())
	{
		if (callbackClick)
		{
			callbackClick(this, entity);
		}	
	}

	if (nodeOpen)
	{
		for (unsigned int i = 0; i < entity->getChildren().size(); i++)
		{
			CreateEntityTree(entity->getChildren()[i], flags);
		}
		ImGui::TreePop();
	}
}


