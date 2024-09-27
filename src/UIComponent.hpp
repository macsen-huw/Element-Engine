#pragma once
#include "Component.hpp"
#include "RectTransformComponent.hpp"
#include <imgui.h>

namespace elmt {
	struct UIFont;
	class UIComponent : public Component
	{
	public:
		UIComponent();
		UIComponent(const char* name, Entity* entity, bool isFixed);
		virtual bool Update();
		virtual void FixedUpdate(RectTransformComponent* rect) {};
		virtual void DynamicUpdate() {};
		void	SetFont(const char* fontFile, float fontSize);
		void	SetTextColor(const ImVec4& col);
	protected:
		UIFont* uiFont = NULL;
		ImVec4 textColor = ImVec4(1, 1, 1, 1);
		bool isFixed = false;
	public:
		ImVec2 pos;
		ImVec2 size;
		ImVec4 bgColor;
		std::string text;
	};

}