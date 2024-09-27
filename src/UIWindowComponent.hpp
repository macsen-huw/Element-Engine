#pragma once
#include "UIComponent.hpp"

namespace elmt {

	class UIWindowComponent : public UIComponent {
	public:
		UIWindowComponent();
		UIWindowComponent(const char* name, Entity* entity, bool isFixed, std::string label);
		UIWindowComponent(const char* name, Entity* entity, bool isFixed, 
			std::string label, ImVec2 pos, ImVec2 size, ImVec4 bgColor);

		void SetPos(const ImVec2& newPos);
		void SetSize(const ImVec2& newSize);

		void	Begin();
		void	End();
		virtual bool Update() { return true; }
	};

}

