#include "Group.hpp"

using namespace elmt;

Group::Group() : Group("")
{
}

Group::Group(std::string name) : name(name)
{
}

/*
Add a new Entity to this group
*/
bool Group::addEntity(Entity* entity)
{
    entities.insert(entity);
    return false;
}


/*
Check if a group contains an Entity
*/
bool Group::hasEntity(Entity* entity)
{
    if (entities.find(entity) != entities.end()) {
        return true;
    }
    return false;
}

/*
Remove Entity from group
*/
bool Group::removeEntity(Entity* entity)
{
    entities.erase(entity);
    return false;
}

/*
* Print info about the Group
*/
std::ostream& elmt::operator <<(std::ostream& os, const Group& g)
{
    os << "Group(" << g.name << ", size: " << g.entities.size() << ")";
    return os;
}
