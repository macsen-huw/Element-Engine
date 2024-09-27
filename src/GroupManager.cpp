#include "Group.hpp"
#include "GroupManager.hpp"

using namespace elmt;


/*
Add an Entity to a named group
If the group does not exist and createIfRequired is set,
then create the group
*/
bool GroupManager::addEntityToGroup(const char* groupName, Entity* entity, bool createIfRequired)
{
    Group* group = getGroup(groupName);

    // Create group if it doesn't exist
    if (!group) {
        if (createIfRequired) {
            group = addGroup(groupName);
        }
        else {
            return false;
        }
    }

    group->addEntity(entity);
    
    return true;
}

bool GroupManager::addEntityToGroup(const char* groupName, Entity* entity)
{
    bool res = addEntityToGroup(groupName, entity, true);

    if (!res) return false;

    return true;
}


/*
Print info about a group and it's contents
For debugging purposes
*/
void GroupManager::debugPrintInfo(const Group& g)
{
    std::cout << "Group: " << g.name << " (" << g.entities.size() << " Entities)" << std::endl << "{" << std::endl;
    int i = 0;
    for (Entity* e : g.entities) {
        std::cout << i << ": " << *e << std::endl;
        i++;
    }
    std::cout << "}" << std::endl;
}



GroupManager::GroupManager()
{
}

GroupManager::~GroupManager()
{
    for (auto g = groups.begin(); g != groups.end(); ++g) {
        delete g->second;
    }
}

/*
Create a new group
*/
Group* GroupManager::addGroup(const char* name)
{

    Group* newGroup = new Group(name);

    groups[name] = newGroup;

    return newGroup;
}

/*
Get a group with a given name
If the group does not exist, nullptr is returned
*/
Group* GroupManager::getGroup(const char* name)
{
    auto groupExists = groups.find(name);
    if (groupExists != groups.end()) {
        return groups[name];
    }
    else {
        return nullptr;
    }

}

/*
Remove an Entity from all the groups that contain it
*/
bool GroupManager::removeEntityFromGroups(Entity* entity)
{
    for (auto g = groups.begin(); g != groups.end(); ++g) {
        g->second->removeEntity(entity);
    }
    return true;
}


/*
Print info for ALL groups
For debugging purposes
*/
void GroupManager::debugPrintAllInfo()
{
    for (auto g = groups.begin(); g != groups.end(); ++g) {
        debugPrintInfo(*(g->second));
    }
}

