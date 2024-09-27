#pragma once
#include "UIComponent.hpp"
#include <functional>

namespace elmt {
	class UIComboComponent : public UIComponent
	{
	public:
		UIComboComponent();
		UIComboComponent(const char* name, Entity* entity, bool isFixed);
		virtual void FixedUpdate(RectTransformComponent* rectTransform);
		virtual void DynamicUpdate();
		std::function<void(UIComboComponent* btn)> callbackItemSelected;
		void	SetItems(const char* items[], int itemCount);

	public:
		int	currentItem = 0;
	private:
		std::string textItems;
	};

}