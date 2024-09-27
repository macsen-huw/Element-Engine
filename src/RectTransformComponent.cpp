#include "RectTransformComponent.hpp"
#include "Logger.hpp"

using namespace elmt;

RectTransformComponent::RectTransformComponent()
{
	typeName = "RectTransformComponent";
}

RectTransformComponent::RectTransformComponent(const glm::vec2& position, float rotationAngle)
	: position(position), rotationAngle(rotationAngle)
{
	typeName = "RectTransformComponent";
}

RectTransformComponent::RectTransformComponent(const glm::vec2& position, float rotationAngle, const glm::vec2& size)
	: position(position), rotationAngle(rotationAngle), size(size) 
{
	typeName = "RectTransformComponent";
}

void RectTransformComponent::clone(Component*& clonePointer, Entity* entityToAttach)
{
	clonePointer = new RectTransformComponent(position, rotationAngle, size);
	Logger::Print("Cloned RectTransformComponent", LOGCAT::CAT_CORE, LOGSEV::SEV_INFO | LOGSEV::SEV_TRACE);
}