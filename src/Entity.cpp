#include "Entity.hpp"
#include "GroupManager.hpp"
#include "Logger.hpp"
#include "core.hpp"

#include <iostream>
#include <algorithm>
#include <string>
#include <variant>
#include "glm/ext.hpp"
#include "cereal/cereal.hpp"

#include "../external/glm-0.9.7.1/glm/gtc/matrix_transform.hpp"
#include "../external/glm-0.9.7.1/glm/gtx/euler_angles.hpp"





using namespace elmt;

// Only used for serialisation TODO fix
Entity::Entity()
{
	name = "";
	parent = nullptr;
	uuid = "";
	typeName = "Entity";

}

Entity::Entity(const char* name, Entity* parent) : name(name)
{
	// Create UUID
	uuid = core::generateUUID();

	// Do callback
	if (callbackPreInit) callbackPreInit(this);

	// Important!
	// Make sure to automatically add each entity to the group(s) determined by it's class and parent class(es)
	core::getGroupManager()->addEntityToGroup("Entity", this);

	this->parent = nullptr;
	setParent(parent);
	

	if (!parent) {
		inTree = false;
	}
	else if (parent->inTree == true) {
		inTree = true;
	}

	// Could probably get away with __func__, but better to be explicit
	typeName = "Entity";

	// Do callback
	if (callbackInit) callbackInit(this);
}

Entity::Entity(const char* name) : Entity(name, nullptr) {};

Entity::Entity(const char* name, Entity* parent, glm::vec3 pos) : Entity(name, parent) {
	this->pos = pos;
	this->oldPos = this->pos;
}







/*
Remove a component from this Entity
*/
bool Entity::removeComponent(Component* component)
{
	auto index = std::find(components.begin(), components.end(), component);
	if (index != components.end()) {
		components.erase(index);
		return true;
	}
	else {
		// TODO add this when debugging is implemented
		return false;
	}
}


/*
Update all of the components of this Entity
*/
bool Entity::updateComponents()
{
	for (Component* component : components) {
		component->Update();
	}
	return true;
}

/*
Print the Entity
*/
std::ostream& elmt::operator <<(std::ostream& os, const elmt::Entity& e)
{

	os << "Entity(" << e.name << ", " << glm::to_string(e.pos) << ", (" << e.components.size() << ") )";
	return os;
}

/*
Give this entity a new child
Better to call this from Entity->setParent
*/
bool Entity::addChild(Entity* newChild) {
	children.push_back(newChild);

	// TODO add child replacing
	// newChild's parent is this Entity now
	newChild->parent = this;

	return true;
}

bool Entity::removeChild(Entity* child)
{
	auto index = std::find(children.begin(), children.end(), child);
	if (index != children.end()) {
		children.erase(index);
		return true;
	}
	else {
		// TODO real error handling
		return false;
	}
	
}

bool Entity::setParent(Entity* newParent)
{

	// Remove self from old parent
	if (parent) {
		parent->removeChild(this);
	}

	parent = newParent;
	if (newParent) {
		newParent->addChild(this);

		glm::vec3 posDiff = pos - parent->pos;
		localPos = glm::transpose(glm::mat3(parent->rotation)) * posDiff;
	}
	

	return true;
}

/*
Print information about this object
For debugging purposes
*/
void Entity::debugPrint() {
	// Find indentation level
	int i = 0;
	Entity* currentEntity = this;
	while (currentEntity->parent) {
		i++;
		currentEntity = currentEntity->parent;
		std::cout << "-";
	}

	std::cout << "Updating: " << *this << " dt(" << core::getDeltaTime() << ")" << std::endl;
}

/*
* Get the children of this object, and the children of it's children, etc
*/
std::vector<Entity*> Entity::getChildrenRecursive()
{

	std::vector<Entity*> recursiveChildren;
	recursiveChildren.push_back(this);
	for (Entity* child : children) {
		child->getChildrenRecursive(recursiveChildren);
	}

	return recursiveChildren;
}

/*
* Get the children of this object, and the children of it's children, etc
* This version is used recursively
*/
void Entity::getChildrenRecursive(std::vector<Entity*>& existingChildren)
{

	// Already added
	if (std::find(existingChildren.begin(), existingChildren.end(), this) != existingChildren.end()) {
		return;
	}

	existingChildren.push_back(this);
	for (Entity* child : children) {
		child->getChildrenRecursive(existingChildren);
	}
}

/*
Attempt to find a child of this Entity via either name or uuid
If name or uuid are left blank, they are ignored
*/
Entity* Entity::findChild(const char* name, const char* uuid, bool recursive)
{
	// TODO optimise
	std::vector<Entity*> entityChildren;
	if (recursive) {
		entityChildren = getChildrenRecursive();
	}
	else {
		entityChildren = children;
	}

	// See if we have a match
	for (Entity* child : entityChildren) {
		if (name && !child->getName().compare(name)) {
			return child;
		} else if (uuid && !child->getUUID().compare(uuid)) {
			return child;
		}
		
	}

	// Otherwise return null
	return nullptr;
}


glm::vec3 Entity::getForward() {
	glm::vec3 direction = glm::vec3(rotation[2]);
	return direction;
}

glm::vec3 Entity::getRight() {
	glm::vec3 right = glm::vec3(rotation[0]);
	return right;
}

glm::vec3 Entity::getUp() {
	glm::vec3 up = glm::vec3(rotation[1]);
	return up;
}

glm::vec3 Entity::getRotationEuler()
{
	glm::vec3 euler;

	glm::extractEulerAngleXYZ(rotation,
		euler.x,
		euler.y,
		euler.z);


	return euler;
}

void Entity::setRotationEuler(glm::vec3 euler)
{
	
	rotation = glm::eulerAngleYXZ(euler.y, euler.x, euler.z);

}

void Entity::rotateHorizontal(float amount)
{
	glm::vec3 axis = getUp();
	glm::mat4x4 hRotate = glm::rotate(glm::mat4x4(),
		-amount,
		axis
	);
	// TODO optimise
	rotation = hRotate * rotation;
}

void Entity::rotateVertical(float amount)
{
	glm::vec3 axis = getRight();
	glm::mat4x4 hRotate = glm::rotate(glm::mat4x4(),
		-amount,
		axis
	);
	// TODO optimise
	rotation = hRotate * rotation;
}



Component* Entity::getComponentOfType(const std::string& typeName)
{
	for (Component* component : components) {
		if (component->getTypeName() == typeName) {
			return component;
		}
	}
	return nullptr;
}


Component* Entity::getComponentOfName(const std::string& compName) {
	for (Component* component : components) {
		if (component->getName() == compName) {
			return component;
		}
	}
	return nullptr;
}

void Entity::updateLocalPos()
{
	glm::vec3 posDiff = pos - parent->pos;
	localPos = glm::transpose(glm::mat3(parent->rotation)) * posDiff;
}

void Entity::updateInTree(bool val, bool updateChildren)
{
	oldInTree = inTree;
	inTree = val;

	if (updateChildren) {
		for (Entity* child : children) {
			child->updateInTree(val, true);
		}
	}
}

/*
Update this Entity
*/
bool Entity::Update() {

	// Do callback
	if (callbackUpdate) callbackUpdate(this);

	updateComponents();


	posChange = pos - oldPos;
	rotChange = glm::transpose(oldRot) * rotation;

    for (int i = 0; i < 4; i++){
        for (int j = 0; j < 4; j++){
            assert(!std::isnan(rotation[i][j]));
        }
    }

    if (std::isnan(pos.x)){
        volatile int *x = nullptr;
        *x = 0;
    }
    assert(!std::isnan(pos.x));
    assert(!std::isnan(pos.y));
    assert(!std::isnan(pos.z));


    // Update all children
	bool res;
	for (Entity* child : children) {
		glm::vec3 predPos;
		if (child->rotateWithParent) {
			predPos = oldPos + glm::mat3(oldRot) * child->localPos;
		}
		else {
			predPos = oldPos + child->localPos;
		}

		if (predPos != child->pos) {
			child->updateLocalPos();
		}

		

		if (child->rotateWithParent) {
			child->pos = pos + (glm::mat3(rotation) * child->localPos);
		}
		else {
			child->pos = child->pos + posChange;// +child->localPos;
		}
		

		

		res = child->Update();
		if (!res) {
			return false;
		}
	}

	oldPos = pos;
	oldRot = rotation;



	//debugPrint();

	return true;
}

/*
Render this Entity
*/
bool Entity::Render() {
	// No requirement for an Entity to be drawn,
	// but Render() still needs to be called in case children
	// need to be drawn

	// Do callback
	if (callbackRender) callbackRender(this);

	// Render all children
	bool res;
	for (Entity* child : children) {
		res = child->Render();
		if (!res) return false;
	}
	return true;
}

/*
This is used to finish serialisation with a second pass
*/
int Entity::finishSerialisation(const std::vector<Entity*>& entities)
{
	if (!serialisedData) {
		// TODO proper error handling
		return -1;
	}

	// Set parent
	if ( !serialisedData->parentID.empty() ){
	
		bool foundParent = false;
		for (Entity* entity : entities) {
			if (!entity->getUUID().compare(serialisedData->parentID)) {
				setParent(entity);
				foundParent = true;
				break;
			}
		}

		if (!foundParent) {
			// TODO proper error handling
			return -2;
		}
	}

	delete serialisedData;
	return 0;
}


Entity::~Entity()
{
	if (deleted) {
		return;
	}

	// Do callback
	if (callbackDelete) callbackDelete(this);

	// Clean up all components
	unsigned int componentIndex = 0;
	while (components.size() > 0) {
		Component* comp = components[0];
		if (comp->getDeleted()) {
			// Already deleted, so just remove it here
			components.erase(components.begin());
			Logger::Print("Skipped deleting Component " + std::to_string(componentIndex) + " (of Entity \"" + name + "\"), as it was already deleted", LOGCAT::CAT_CORE, LOGSEV::SEV_WARNING);
		}
		else {
			// Removal from our components vector occurs in Component destructor
			Component* componentToDelete = components[0];
			delete componentToDelete;
		}
		componentIndex++;
	}

	if (core::getIsSetup()) {
		core::getGroupManager()->removeEntityFromGroups(this);
	}
	

	if (parent) {
		parent->removeChild(this);
	}

	deleted = true;
	inTree = false;
}

