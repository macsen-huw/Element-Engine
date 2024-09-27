#pragma once
#include "UIComponent.hpp"
namespace elmt {

	class UIInputTextComponent : public UIComponent
	{
	public:
		UIInputTextComponent();
		UIInputTextComponent(const char* name, Entity* entity, bool isFixed);
		virtual void FixedUpdate(RectTransformComponent* rectTransform);
		virtual void DynamicUpdate();
		void (*callbackTextChanged)(UIInputTextComponent* btn) = nullptr;

		void	SetText(const char* inputText);
		std::string GetText();
	private:
		char textBuffer[1024] = "";

	};

}