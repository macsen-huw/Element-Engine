#pragma once
#include "UIComponent.hpp"
namespace elmt {

	class UIInputFloat3Component : public UIComponent
	{
	public:
		UIInputFloat3Component();
		UIInputFloat3Component(const char* name, Entity* entity, float* values, bool isFixed);
		virtual void FixedUpdate(RectTransformComponent* rectTransform);
		virtual void DynamicUpdate();
	private:
		float* values;
	};

}