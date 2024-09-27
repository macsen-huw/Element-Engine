#include "Trigger.hpp"
#include "Level.hpp"

Trigger::Trigger(glm::vec3 pos, glm::vec3 dim, std::map<std::string, TriggerFunction> callbacks) : elmt::Entity("Trigger", g::triggersBase, pos)
{
	this->callbacks = callbacks;
	this->bounds = bounds;


	//auto dbgCube = new elmt::DebugRenderObject(pos + (dim*0.5f), dim, glm::vec3(1.0, 0.0, 0.0), elmt::CUBE);
	
	this->scale = dim;
	physComp = new elmt::PhysicsComponent("Trigger Physics", this, elmt::PhysicsPrimitives::PRIM_TRIMESH, g::modelList["cube"]); //dbgCube->model
	physComp->setOverrideEntityPos(false);
	physComp->setKinematic();
}

bool Trigger::Update() {
	bool res = elmt::Entity::Update();
	if (!res) return false;

	for (std::pair<std::string, TriggerFunction> callbackPair : callbacks) {
		elmt::Group* group = g::groupMan->getGroup( (callbackPair.first).c_str() );
		if (!group) {
			elmt::Logger::Print("Couldn't find group: \"" + callbackPair.first + "\", skipping", elmt::LOGCAT::CAT_CORE, elmt::LOGSEV::SEV_WARNING);
		}

		// Check to see if any of the objects intersected with are no longer intersected with
		for (elmt::Entity* iObj : intersectingObjects) {
			elmt::PhysicsComponent* objPhysComp = (elmt::PhysicsComponent*)iObj->getComponentOfType("PhysicsComponent");
			
			elmt::IntersectionInfo collidingWith = g::colMan->isIntersecting<elmt::PhysicsComponent, elmt::PhysicsComponent>(physComp, objPhysComp);
			if (!collidingWith.entity) {
				auto it = std::find(intersectingObjects.begin(), intersectingObjects.end(), iObj);
				intersectingObjects.erase(it);
				elmt::Logger::Print("Removed Entity \"" + iObj->getName() + "\" from intersection list", elmt::LOGCAT::CAT_LOGIC, elmt::LOGSEV::SEV_TRACE | elmt::LOGSEV::SEV_INFO);
			}
		}

		for (elmt::Entity* obj : group->getEntities()) {
			elmt::PhysicsComponent* objPhysComp = (elmt::PhysicsComponent * )obj->getComponentOfType("PhysicsComponent");
			if (!objPhysComp) {
				continue;
			}

			if (std::find(intersectingObjects.begin(), intersectingObjects.end(), obj) == intersectingObjects.end()) {
				if (g::colMan->isIntersecting<elmt::PhysicsComponent, elmt::PhysicsComponent>(physComp, objPhysComp).entity) {
						// Do callback
						auto className = obj->getTypeName();
						TriggerFunction func = callbacks[className];
						intersectingObjects.push_back(obj);
						func(obj, this);
			
						elmt::Logger::Print("Added Entity \"" + obj->getName() + "\" to intersection list", elmt::LOGCAT::CAT_LOGIC, elmt::LOGSEV::SEV_TRACE | elmt::LOGSEV::SEV_INFO);
				}
			}
		}
	}

	
	return true;
}

Trigger::~Trigger() {
	
}