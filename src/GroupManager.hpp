#pragma once

#include "Entity.hpp"
#include "Group.hpp"

#include <set>
#include <map>
#include <iostream>

namespace elmt {

	class GroupManager {
		// Properties
	private:
		inline static std::map<std::string, Group*> groups;

		// Methods
	public:

		GroupManager();
		~GroupManager();

		Group* addGroup(const char* name);
		Group* getGroup(const char* name);

		bool addEntityToGroup(const char* groupName, Entity* entity, bool createIfRequired);
		bool addEntityToGroup(const char* groupName, Entity* entity);
		bool removeEntityFromGroups(Entity* entity);

		void debugPrintInfo(const Group& g);
		void debugPrintAllInfo();

	};

};