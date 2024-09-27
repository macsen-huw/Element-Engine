#include "FallingRock.hpp"
#include "Level.hpp"
#include "globals.hpp"

FallingRock::FallingRock()
{
}

FallingRock::FallingRock(glm::vec3 pos) : GameObject( ("Rock " + g::getObjectID()).c_str(), pos, g::level->getModel("rock"), true)
{
	setParent(nullptr);
}
