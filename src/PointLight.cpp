
#include "PointLight.h"
#include "core.hpp"
#include "GroupManager.hpp"

using namespace elmt;

PointLight::PointLight(const char* name, Entity* parent, glm::vec3 pos, glm::vec3 colour, float intensity) : Entity(name, parent, pos)
{
    core::getGroupManager()->addEntityToGroup("PointLight", this);
    typeName = "PointLight";

    this->colour = colour;
    this->intensity = intensity;
}
