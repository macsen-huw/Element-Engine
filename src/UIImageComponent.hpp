#pragma once
#include "UIComponent.hpp"
namespace elmt {
	class UIImageComponent : public UIComponent
	{
	public:
		UIImageComponent();
		UIImageComponent(const char* name, Entity* entity, bool isFixed);
		virtual void FixedUpdate(RectTransformComponent* rectTransform);
		virtual void DynamicUpdate();
		void SetImage(const char* imageFile);
	private:
		unsigned int  textureID = 0;

	};

}