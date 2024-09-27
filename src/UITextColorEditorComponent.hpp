#pragma once
#include "UIComponent.hpp"

namespace elmt {

	class UITextColorEditorComponent : public UIComponent
	{
	public:
		UITextColorEditorComponent();
		UITextColorEditorComponent(const char* name, Entity* entity, bool isFixed);
		virtual void FixedUpdate(RectTransformComponent* rectTransform);
		virtual void DynamicUpdate();
	};

}