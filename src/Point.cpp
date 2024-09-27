#include "Point.hpp"

#include "GroupManager.hpp"

using namespace elmt;

Point::Point(const char* name, Entity* parent) : Entity(name, parent) {
	core::getGroupManager()->addEntityToGroup("Point", this);
	typeName = "Point";
};
Point::Point(const char* name, Entity* parent, glm::vec3 pos) : Entity(name, parent, pos) {
	core::getGroupManager()->addEntityToGroup("Point", this);
	typeName = "Point";
};