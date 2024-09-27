#pragma once
#include "UIComponent.hpp"
#include <functional>

namespace elmt {

	class UICollapsingComponent : public UIComponent
	{
	public:
		UICollapsingComponent();
		UICollapsingComponent(const char* name, Entity* entity, std::string label, bool isFixed);
		UICollapsingComponent(const char* name, Entity* entity, std::string label, Entity* winEntity, bool isFixed);
		UICollapsingComponent(const char* name, Entity* entity, std::string label, Component* comp, bool isFixed);
		virtual void FixedUpdate(RectTransformComponent* rectTransform);
		virtual void DynamicUpdate();
		std::function<void(UICollapsingComponent* collap, Entity* currentWindow)> callbackOnToggle;
		std::function<void(UICollapsingComponent* collap, Entity* currentWindow, Component* currentComp)> callbackShowComponents;

		std::string getText() const;
	private:
		std::string text;
		Entity* winEntity;
		Component* currentComp;
	};

}