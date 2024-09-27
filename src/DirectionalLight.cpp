
#include "DirectionalLight.h"
#include <glm/common.hpp>
#include "GroupManager.hpp"
#include "core.hpp"

using namespace elmt;

DirectionalLight::DirectionalLight(const char* name, Entity* parent, glm::vec3 direction, glm::vec3 colour, float intensity) 
    : Entity(name, parent, glm::vec3(0.0, 0.0, 0.0) )
{
    core::getGroupManager()->addEntityToGroup("DirectionalLight", this);
    this->direction = direction;
    this->colour = colour;
    this->intensity = intensity;

    typeName = "DirectionalLight";
}
