#pragma once
#include "UIComponent.hpp"
#include <vector>
#include <string>
#include <functional>

namespace elmt {

	class UITreeNodeComponent : public UIComponent {
	public:
		UITreeNodeComponent();
		UITreeNodeComponent(const char* name, Entity* entity, bool isFixed);

		UITreeNodeComponent(const char* name, Entity* entity, bool isFixed, Entity* root);
		virtual void FixedUpdate(RectTransformComponent* rectTransform);
		virtual void DynamicUpdate();

		std::function<void(UITreeNodeComponent* treeNode, Entity* currentEntity)> callbackClick;
		void CreateEntityTree(Entity* entity, ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None);

	private:
		Entity* rootEntity;
	};

}
