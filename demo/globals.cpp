#include "globals.hpp"

#include "core.hpp"

using namespace globals;

void g::Setup()
{
	rendMan = elmt::core::getRenderManager();
	inpMan = elmt::core::getInputManager();
	assMan = elmt::core::getAssetManager();
	procMan = elmt::core::getProcManager();
	groupMan = elmt::core::getGroupManager();
	colMan = elmt::core::getCollisionManager();
	audMan = elmt::core::getAudioManager();
	uiMan = elmt::core::getRenderManager()->getUIManager();

	turnLeft = elmt::InputAction(std::vector<int>{glfwGetKeyScancode(GLFW_KEY_A)});
	turnRight = elmt::InputAction(std::vector<int>{glfwGetKeyScancode(GLFW_KEY_D)});

	moveForward = elmt::InputAction(std::vector<int>{glfwGetKeyScancode(GLFW_KEY_W)});
	moveBackward = elmt::InputAction(std::vector<int>{glfwGetKeyScancode(GLFW_KEY_S)});

	// Setup game-specific entities
	world = new elmt::Entity("World", elmt::core::rootEntity);

	platformsBase = new elmt::Entity("Platform Base", g::world, { 0.0,0.0,0.0 });;
	collectablesBase = new elmt::Entity("Collections Base", g::world, { 0.0,0.0,0.0 });
	triggersBase = new elmt::Entity("Triggers Base", g::world, { 0.0,0.0,0.0 });
}

void globals::g::Cleanup()
{
	for (auto mp : modelList) {
		elmt::Model* m = mp.second;
		delete m;
	}
}

std::string globals::g::getObjectID()
{
	auto id = std::to_string(idNext++);
	return id;
}

void globals::g::addModel(std::string modelName, std::string modelPath)
{
	auto model = g::assMan->importAsset(BASE_PATH + modelPath,  modelName + ".obj");
	modelList.insert({ modelName, model });
}

elmt::Model* globals::g::getModel(std::string modelName)
{
	auto it = modelList.find(modelName);
	if (it == modelList.end()) {
		elmt::Logger::Print("Model name \"" + modelName + "\" was not found in global model list", elmt::LOGCAT::CAT_LOADING, elmt::LOGSEV::SEV_ERROR);
		return nullptr;
	}

	return modelList[modelName];
}
