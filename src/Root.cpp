#include "Root.hpp"
#include "core.hpp"
#include "GroupManager.hpp"

using namespace elmt;

Root::Root() : Entity("root")
{

	// Since there is only one "Root" object in the tree
	// This isn't really required, but we are doing it for all other
	// entities, so best to do it here anyway
	core::getGroupManager()->addEntityToGroup("Root", this);
	typeName = "Root";
}
