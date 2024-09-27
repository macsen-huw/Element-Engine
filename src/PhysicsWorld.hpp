#pragma once

#define dSINGLE
#include <ode/ode.h>
#include "glm/common.hpp"

namespace elmt{
	class PhysicsWorld
	{

	public:
		PhysicsWorld();
		~PhysicsWorld();

		void setGravity(glm::vec3 gravity);
		void setStepSize(dReal step);
		void takeStep();
		void clearJoints();

		void destroy();

		dWorldID getWorldID();
		dSpaceID getSpaceID();
		dJointGroupID getContactGroupID();
		dJointGroupID getGeneralGroupID();

	private:
		dWorldID worldID;
		dSpaceID spaceID;

		//2 Joint groups - 1 specifically for contacts, the other for every other kind
		dJointGroupID contactGroupID;
		dJointGroupID generalGroupID;

		dReal step = 1.0 / 60.0;
	};
}