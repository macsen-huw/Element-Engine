#include "Component.hpp"
#include "core.hpp"
#include "Logger.hpp"

using namespace elmt;

// Only used for serialisation TODO fix
Component::Component()
{
    name = "";
    entity = nullptr;
    uuid = "";
    callbackUpdate = nullptr;
    callbackDelete = nullptr;
    typeName = "Component";
}


Component::Component(const char* name, Entity* entity) : name(name), entity(entity)
{

    callbackUpdate = nullptr;
    callbackDelete = nullptr;

    // Create UUID
    uuid = core::generateUUID();

    // Could probably get away with __func__, but better to be explicit
    typeName = "Component";

    setEntity(entity);
}

void Component::setEntity(Entity* ent) {
    // Removed old entity
    if (entity) {
        entity->removeComponent(this);
    }
    entity = ent;
    if (entity) {
        entity->addComponent(this);
    }
    
}

/*
This is used to finish serialisation with a second pass
*/
int Component::finishSerialisation(const std::vector<Component*>& components)
{
    // This isn't used rn
    if (!serialisedData) {
        // TODO proper error handling
        return -1;
    }

    delete serialisedData;
    return 0;
}

/*
Update the component
*/
bool Component::Update()
{
    if (callbackUpdate) callbackUpdate(this);
	return true;
}

void Component::clone(Component*& clonePointer, Entity* entityToAttach)
{
    clonePointer = new Component(this->name.c_str(), entityToAttach);
    Logger::Print("Cloned Component " + name + ", UUID " + clonePointer->uuid, LOGCAT::CAT_CORE, LOGSEV::SEV_INFO | LOGSEV::SEV_TRACE);
    
}

namespace elmt {
/*
Print the Component
*/
    std::ostream &operator<<(std::ostream &os, const Component &c) {

        const char *entityString;
        if (c.entity) {
            const std::string &entityName = c.entity->getName();
            entityString = entityName.c_str();
        } else {
            entityString = "[No Entity]";
        }

        os << "Component(" << c.name << " of " << entityString << ")";
        return os;
    }
}

Component::~Component()
{
    if (deleted) {
        return;
    }

    // Do callback
    if (callbackDelete) callbackDelete(this);

    if (entity) {
        entity->removeComponent(this);
    }

    deleted = true;
}


