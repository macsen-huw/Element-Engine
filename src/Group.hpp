#pragma once
#include <string>
#include <set>
#include "Entity.hpp"

namespace elmt {

	class Group
	{

		// Properties
	private:

		// Group name
		std::string name;

		// Actual entities in the group
		std::set<Entity*> entities;

		// Methods
	private:
		friend class GroupManager;
		Group();
		Group(std::string name);

	public:
		bool addEntity(Entity* entity);
		bool hasEntity(Entity* entity);
		bool removeEntity(Entity* entity);
		const std::set<Entity*>& getEntities() { return entities; }


		friend std::ostream& operator<< (std::ostream& os, const Group& g);
	};

};