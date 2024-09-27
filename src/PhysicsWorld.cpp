#include "PhysicsWorld.hpp"


using namespace elmt;

//Init World
PhysicsWorld::PhysicsWorld()
{
	dInitODE();
	
	worldID = dWorldCreate();

	//Create space
	spaceID = dHashSpaceCreate(0);

	contactGroupID = dJointGroupCreate(0);

	generalGroupID = dJointGroupCreate(0);
}

PhysicsWorld::~PhysicsWorld()
{
	dJointGroupDestroy(contactGroupID);

	dJointGroupEmpty(generalGroupID);
	dJointGroupDestroy(generalGroupID);

	dSpaceDestroy(spaceID);
	dWorldDestroy(worldID);
	dCloseODE();
}


dWorldID PhysicsWorld::getWorldID()
{
	return worldID;
}

dSpaceID PhysicsWorld::getSpaceID()
{
	return spaceID;
}

dJointGroupID PhysicsWorld::getContactGroupID()
{
	return contactGroupID;
}

dJointGroupID PhysicsWorld::getGeneralGroupID()
{
	return generalGroupID;
}

//Set the gravity of the world
void PhysicsWorld::setGravity(glm::vec3 gravity)
{
	dWorldSetGravity(worldID, gravity.x, gravity.y, gravity.z);
}

//Set the step size
void PhysicsWorld::setStepSize(dReal stepSize)
{
	step = stepSize;
}

//Take a step in the world by the number of seconds passed to it
void PhysicsWorld::takeStep()
{
	dWorldQuickStep(worldID, step);
}

void PhysicsWorld::clearJoints()
{
	dJointGroupEmpty(contactGroupID);
}


//Destroy World (at the end of the program)
void PhysicsWorld::destroy()
{
	dWorldDestroy(worldID);
}