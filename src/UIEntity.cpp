#include "UIEntity.hpp"
#include "UIWindowComponent.hpp"

using namespace elmt;

UIEntity::UIEntity(const char* name, Entity* parent) :Entity(name, parent)
{

}

UIEntity::UIEntity(const char* name, Entity* parent, glm::vec2 pos, glm::vec2 size) :Entity(name, parent)
{
	auto rectTransform = dynamic_cast<RectTransformComponent*>(core::createEmptyComponent("RectTransformComponent"));
	rectTransform->position = pos;
	rectTransform->size = size;
	addComponent(rectTransform);

}

UIEntity::UIEntity(const char* name, Entity* parent, glm::vec2 pos) :Entity(name, parent)
{
	auto rectTransform = dynamic_cast<RectTransformComponent*>(core::createEmptyComponent("RectTransformComponent"));
	rectTransform->position = pos;
	addComponent(rectTransform);
}

void	UIEntity::SetPosition(float x, float y)
{
	auto rect = dynamic_cast<RectTransformComponent*>(getComponentOfType("RectTransformComponent"));
	rect->position.x = x;
	rect->position.y = y;
}
void	UIEntity::SetSize(float w, float h)
{
	auto rect = dynamic_cast<RectTransformComponent*>(getComponentOfType("RectTransformComponent"));
	rect->size.x = w;
	rect->size.y = h;

}
bool UIEntity::Update()
{
	auto uiWnd = dynamic_cast<UIWindowComponent*>(getComponentOfType("UIWindowComponent"));
	if (uiWnd)
	{
		uiWnd->Begin();
		Entity::Update();
		
		uiWnd->End();
	}
	else
	{
		Entity::Update();
	}
	return true;
}
