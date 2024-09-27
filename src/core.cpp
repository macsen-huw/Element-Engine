#include "core.hpp"
#include "Group.hpp"
#include "Root.hpp"
#include "Point.hpp"
#include "Logger.hpp"
#include "Log.hpp"
#include "PhysicsComponent.hpp"
#include "MicrophoneComponent.hpp"
#include "SoundComponent.hpp"
#include "MouseLookComponent.hpp"
#include "MeshRenderer.hpp"
#include "CollisionManager.hpp"
#include "AssetManager.hpp"
#include "ThreadManager.hpp"
#include "AudioManager.hpp"
#include "ProcManager.hpp"
#include "InputManager.hpp"
#include "RectTransformComponent.hpp"
#include "UIManager.hpp"
#include "GroupManager.hpp"
#include "Entity.hpp"
#include "Component.hpp"
#include "PhysicsWorld.hpp"
#include "Camera.hpp"


#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <random>
#include <fstream>
#include <algorithm>
#include "assimp/version.h"

#include "cereal/types/vector.hpp"
#include "cereal/types/map.hpp"
#include <cereal/types/memory.hpp>

#include "UILabelComponent.hpp"
#include "UIImageComponent.hpp"
#include "UIButtonComponent.hpp"
#include "UICheckboxComponent.hpp"
#include "UIWindowComponent.hpp"
#include "UISliderComponent.hpp"
#include "UIInputTextComponent.hpp"
#include "UIComboComponent.hpp"
#include "UITreeNodeComponent.hpp"
#include "UISeparatorComponent.hpp"
#include "UITextColorEditorComponent.hpp"
#include "UISpacingComponent.hpp"
#include "UISameLineComponent.hpp"
#include "UICollapsingComponent.hpp"
#include "UIInputFloat3Component.hpp"
#include "UIPushColorComponent.hpp"
#include "UIPopColorComponent.hpp"

using namespace elmt;

/*
* Set up a window for rendering using GLFW
*/
bool core::setupWindow(int width, int height) {
	// From the rasterisation project
	// Initialise GLFW
	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return false;
	}


	glfwWindowHint(GLFW_SAMPLES, 1); //no anti-aliasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy;
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, "OpenGLRenderer", NULL, NULL);

	if (window == NULL) {
		std::cerr << "Failed to open GLFW window. If you have an Intel GPU, they may not be 4.5 compatible." << std::endl;
		glfwTerminate();
		return false;
	}

	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true;
	// Needed for core profile
	if (glewInit() != GLEW_OK) {
		std::cerr << "Failed to initialize GLEW" << std::endl;
		glfwTerminate();
		return false;
	}

	if (!GLEW_ARB_debug_output) return false;

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	//glfwSetInputMode(newWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwPollEvents();
	glfwSetCursorPos(window, width / 2, height / 2);

	glfwSetWindowSizeCallback(window, windowSizeCallback);
	glfwGetWindowSize(elmt::core::getWindow(), &width, &height);

	Logger::Print("Setup Window", LOGCAT::CAT_RENDERING, LOGSEV::SEV_INFO);

	return true;
}



void core::windowSizeCallback(GLFWwindow* window, int width, int height) {

	renderManager->updateScreenSize(glm::ivec2{ width, height });
	if (activeCamera) {
		activeCamera->screenSize = glm::ivec2{ width, height };
	}
	
}



/*
* Set up the engine
*/
bool core::Setup( int windowWidth, int windowHeight, bool useMultithreading) {

	// Set up some basic logging, users can add their own ones if required
	new Log("engine_standard", LOGCAT::CAT_ALL, LOGSEV::SEV_ERROR | LOGSEV::SEV_WARNING | LOGSEV::SEV_INFO, 32, "logs\\engine_standard.log");

	Logger::Print("Setup logs", LOGCAT::CAT_CORE, LOGSEV::SEV_INFO);
	bool res = setupWindow(windowWidth, windowHeight);
	if (!res) return false;

	multithreaded = useMultithreading;


	// Set up ui
	UIManager::setupWindow(window);

	setupManagers();

	// Set up root entity, all entities in use should be parented to this
	// Either directly or indirectly
	rootEntity = new Root();
	rootEntity->inTree = true;


	Logger::Print("Finished setup", LOGCAT::CAT_CORE, LOGSEV::SEV_INFO);

	return true;
}

int core::setupManagers(CollisionManager* newCollisionManager,
	AnimationManager* newAnimationManager,
	RenderManager* newRenderManager,
	AssetManager* newAssetManager,
	ThreadManager* newThreadManager,
	AudioManager* newAudioManager,
	ProcManager* newProcManager,
	InputManager* newInputManager,
	GroupManager* newGroupManager) {

	collisionManager = newCollisionManager;
	animationManager = newAnimationManager;
	renderManager = newRenderManager;
	assetManager = newAssetManager;
	threadManager = newThreadManager;
	audioManager = newAudioManager;
	procManager = newProcManager;
	inputManager = newInputManager;
	groupManager = newGroupManager;

	// Setup defaults
	if (!collisionManager) {
		collisionManager = new CollisionManager();
		Logger::Print("Added default CollisionManager", LOGCAT::CAT_CORE, LOGSEV::SEV_TRACE);
	}
	if (!animationManager) {
		animationManager = new AnimationManager();
		Logger::Print("Added default AnimationManager", LOGCAT::CAT_CORE, LOGSEV::SEV_TRACE);
	}

	if (!renderManager) {
		RenderParameters renderParameters;
		renderParameters.shadowPCFNumSamples = 12;
        renderParameters.cascadePartitions.push_back(0.01);
        renderParameters.cascadePartitions.push_back(0.03);
        renderParameters.cascadePartitions.push_back(0.1);
        renderParameters.cascadePartitions.push_back(0.2);
        renderParameters.cascadePartitions.push_back(0.3);
        renderParameters.cascadePartitions.push_back(0.4);
        renderParameters.cascadePartitions.push_back(0.5);
        renderParameters.cascadePartitions.push_back(0.6);
        renderParameters.cascadePartitions.push_back(0.7);
        renderParameters.cascadePartitions.push_back(0.8);
        renderParameters.cascadePartitions.push_back(0.9);


        renderParameters.numAmbientOcclusionSamples = 8;
		renderManager = new RenderManager(&renderParameters);
		Logger::Print("Added default RenderManager", LOGCAT::CAT_CORE, LOGSEV::SEV_TRACE);
	}


	if (!assetManager) {
		assetManager = new AssetManager(renderManager);
		auto aiVersionM = aiGetVersionMajor();
		auto aiVersion = aiGetVersionMinor();
		auto aiVersionR = aiGetVersionRevision();
		Logger::Print("Assimp Version: " + std::to_string(aiVersionM) + "." + std::to_string(aiVersion) + "." + std::to_string(aiVersionR), LOGCAT::CAT_LOADING | LOGCAT::CAT_RENDERING, LOGSEV::SEV_INFO);
		Logger::Print("Added default AssetManager", LOGCAT::CAT_CORE, LOGSEV::SEV_TRACE);
	}
	if (!threadManager) {
		threadManager = new ThreadManager();
		Logger::Print("Added default ThreadManager", LOGCAT::CAT_CORE, LOGSEV::SEV_TRACE);
	}
	if (!audioManager) {
		audioManager = new AudioManager();
		Logger::Print("Added default AudioManager", LOGCAT::CAT_CORE, LOGSEV::SEV_TRACE);
	}
	if (!procManager) {
		procManager = new ProcManager();
		Logger::Print("Added default ProcManager", LOGCAT::CAT_CORE, LOGSEV::SEV_TRACE);
	}
	if (!inputManager) {
		inputManager = new InputManager();
		Logger::Print("Added default InputManager", LOGCAT::CAT_CORE, LOGSEV::SEV_TRACE);
	}
	if (!groupManager) {
		groupManager = new GroupManager();
		Logger::Print("Added default Grou[Manager", LOGCAT::CAT_CORE, LOGSEV::SEV_TRACE);
	}

	Logger::Print("Acquiring locks...", LOGCAT::CAT_CORE, LOGSEV::SEV_TRACE);
	//threadManager->renderLock.lock();
	Logger::Print("Acquired locks", LOGCAT::CAT_CORE, LOGSEV::SEV_TRACE);
	threadManager->Setup();

	physicsWorld = new PhysicsWorld();

	// Setup default camera
	defaultCamera = new Camera("default camera", rootEntity, glm::vec3(0, 0, 0));

	Logger::Print("Setup managers", LOGCAT::CAT_CORE, LOGSEV::SEV_INFO);

	isSetup = true;

	return 0;
}

/*
* Teardown engine and clean up resources
*/
bool core::Teardown() {


	// This should be cleaned up by deleting root... but whatever
	delete defaultCamera;
	delete rootEntity;



	delete threadManager;
	delete collisionManager;
	delete animationManager;
	delete renderManager;
	delete assetManager;
	delete physicsWorld;
	delete audioManager;
	delete procManager;
	delete inputManager;
	delete groupManager;
	Logger::Print("Cleaned up managers...", LOGCAT::CAT_CORE, LOGSEV::SEV_INFO);

	Logger::Print("Cleaning up logging...", LOGCAT::CAT_CORE, LOGSEV::SEV_INFO);
	Logger::cleanUp();

	//
	glfwTerminate();
	Logger::Print("Terminated GLFW", LOGCAT::CAT_RENDERING, LOGSEV::SEV_INFO);

	

	
	
	isSetup = false;
	return true;
}



/*
Update time since last frame.
Engine dt property updated for convinient interally use,
But also returned to make it easier for devs
*/
double core::updateDeltaTime() {
	core::lastFrameTime = core::currentFrameTime;
	core::currentFrameTime = glfwGetTime();
	core::dt = core::currentFrameTime - core::lastFrameTime;
	
	//clamp
	if (core::dt > 1.0) {
		core::dt = 1.0;
	}
	return dt;
}

/*
Generate a unique ID for something
TODO reference this
https://stackoverflow.com/questions/24365331/how-can-i-generate-uuid-in-c-without-using-boost-library
*/
std::string core::generateUUID()
{

	static std::random_device dev;
	static std::mt19937 rng(dev());

	std::uniform_int_distribution<int> dist(0, 15);

	const char* v = "0123456789abcdef";
	const bool dash[] = { 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0 };

	std::string res;
	for (int i = 0; i < 16; i++) {
		if (dash[i]) res += "-";
		res += v[dist(rng)];
		res += v[dist(rng)];
	}
	return res;
	
}

bool core::saveRoot(const char* filePath)
{
	std::ofstream oFile(filePath);

	cereal::JSONOutputArchive archive(oFile);

	// Serialise entities
	std::vector<Entity*> entityPointersToSerialise = core::rootEntity->getChildrenRecursive();

	// Serialise UUIDs
	std::vector<std::string> entityUUIDs;
	for (Entity* entity : entityPointersToSerialise) {
		entityUUIDs.push_back(entity->getUUID());
	}
	archive(cereal::make_nvp("entityUUIDs", entityUUIDs));

	// Serialise individually, as opposed to in a vector
	// This is important since we need to deserialise multiple times because types
	// Storing via a vector make name look up really annoying
	for (Entity* p : entityPointersToSerialise) {
		cereal::NameValuePair<Entity&> nvp = cereal::make_nvp(p->getUUID(), *p);
		archive(nvp);
	}

	
	
	// Serialise components
	std::vector<Component*> componentPointersToSerialise;
	for (Entity* p : entityPointersToSerialise) {
		for (Component* c : p->getComponents()) {
			// Check for duplicated
			if (std::find(componentPointersToSerialise.begin(), componentPointersToSerialise.end(), c) == componentPointersToSerialise.end()) {
				componentPointersToSerialise.push_back(c);
			}
		}
	}

	// Serialise UUIDs
	std::vector<std::string> componentUUIDs;
	for (Component* component: componentPointersToSerialise) {
		componentUUIDs.push_back(component->getUUID());
	}
	archive(cereal::make_nvp("componentUUIDs", componentUUIDs));

	
	// Serialise individually, as opposed to in a vector
	// This is important since we need to deserialise multiple times because types
	// Storing via a vector make name look up really annoying
	for (Component* c : componentPointersToSerialise) {
		cereal::NameValuePair<Component&> nvp = cereal::make_nvp(c->getUUID(), *c);
		archive(nvp);
	}



	//oFile.flush();
	
	// Don't explicitly close or it won't serialise properly?
	//oFile.close();
	
	return true;
}



/*
Used for deserialisation
Allocate memory for an Entity of a given type
*/
Entity* core::createEmptyEntity(const std::string& entityType) {
	Entity* entity;
	if (entityType == "Entity") {
		entity = new Entity();
	}
	else if (entityType == "Root") {

		entity = new Root();

	}
	else if (entityType == "Point") {

		entity = new Point();
	}
	else {
		// TODO warning
		entity = new Entity();
	}
	return entity;

}

/*
Used for deserialisation
Allocate memory for an Component of a given type
TODO template?
*/
Component* core::createEmptyComponent(const std::string& componentType) {
	Component* component;
	if (componentType == "Component") {
		component = new Component();
	}
	else if (componentType == "PhysicsComponent") {
		component = new PhysicsComponent();
	}
	else if (componentType == "SoundComponent") {
		component = new SoundComponent();
	}
	else if (componentType == "MicrophoneComponent") {
		component = new MicrophoneComponent();
	}
	else if (componentType == "MouseLookComponent") {
		component = new MouseLookComponent();
	}
	else if (componentType == "MeshRenderer") {
		component = new MeshRenderer();
	}
	else if (componentType == "RectTransformComponent") {
		component = new RectTransformComponent();
	}
	else if (componentType == "UIButtonComponent") {
		component = new UIButtonComponent();
	}
	else if (componentType == "UICheckboxComponent") {
		component = new UICheckboxComponent();
	}
	else if (componentType == "UIComboComponent") {
		component = new UIComboComponent();
	}
	else if (componentType == "UIImageComponent") {
		component = new UIImageComponent();
	}
	else if (componentType == "UIInputTextComponent") {
		component = new UIInputTextComponent();
	}
	else if (componentType == "UILabelComponent") {
		component = new UILabelComponent();
	}
	else if (componentType == "UISliderComponent") {
		component = new UISliderComponent();
	}
	else if (componentType == "UIWindowComponent") {
		component = new UIWindowComponent();
	}
	else if (componentType == "UITreeNodeComponent") {
		component = new UITreeNodeComponent();
	}
	else if (componentType == "UISeparatorComponent") {
		component = new UISeparatorComponent();
	}
	else if (componentType == "UISeparatorComponent") {
		component = new UISeparatorComponent();
	}
	else if (componentType == "UITextColorEditorComponent") {
		component = new UITextColorEditorComponent();
	}
	else if (componentType == "UISpacingComponent") {
		component = new UISpacingComponent();
	}
	else if (componentType == "UISamelineComponent") {
		component = new UISamelineComponent();
	}
	else if (componentType == "UICollapsingComponent") {
		component = new UICollapsingComponent();
	}
	else if (componentType == "UIInputFloat3Component") {
		component = new UIInputFloat3Component();
	}
	else if (componentType == "UIPushColorComponent") {
		component = new UIPushColorComponent();
	}
	else if (componentType == "UIPopColorComponent") {
		component = new UIPopColorComponent();
	}
	else {
		Logger::Print("Could not find matching component type for \"" + componentType + "\"", LOGCAT::CAT_CORE, LOGSEV::SEV_WARNING);
		component = new Component();
	}
	return component;

}

bool core::loadRoot(const char* filePath)
{
	// TODO make this better
	std::ifstream oFile(filePath);
	std::string text;
	std::string line;
	while (std::getline(oFile, line)) {
		text.append(line);
	}
	oFile.close();

	std::stringstream is(text);
	cereal::JSONInputArchive archive(is);

	// Load Entities
	std::vector<std::string> entityUUIDs;
	archive(cereal::make_nvp("entityUUIDs", entityUUIDs));

	std::vector<Entity*> deserialisedEntities;
	for (std::string uuid : entityUUIDs) {
		// Create "dummy" entity to find info
		Entity entity;
		archive(cereal::make_nvp(uuid, entity));

		std::string entityType = entity.getTypeName();

		Entity* typedEntity = nullptr;
		typedEntity = createEmptyEntity(entityType);
		archive(cereal::make_nvp(entity.uuid, *typedEntity));

		deserialisedEntities.push_back(typedEntity);
	}

	// Get root
	// TODO improve this once typing is added
	bool foundNewRoot = false;
	for (Entity* entity : deserialisedEntities) {
		if (!entity->getName().compare("root")) {
			if (core::rootEntity) {
				delete core::rootEntity;
			}

			foundNewRoot = true;
			core::rootEntity = entity;
			break;
		}
	}


	

	// Load Components
	std::vector<std::string> componentUUIDs;
	archive(cereal::make_nvp("componentUUIDs", componentUUIDs));

	std::vector<Component*> deserialisedComponents;
	for (std::string uuid : componentUUIDs) {
		// Create "dummy" component to find info
		Component component;
		archive(cereal::make_nvp(uuid, component));

		std::string componentType = component.getTypeName();

		Component* typedComponent = nullptr;
		typedComponent = createEmptyComponent(componentType);
		archive(cereal::make_nvp(component.uuid, *typedComponent));

		deserialisedComponents.push_back(typedComponent);
	}

	// Add components to entities
	for (Entity* entity : deserialisedEntities) {
		for (std::string& componentID: entity->serialisedData->componentIDs){
			
			// Find component
			bool foundComponent = false;
			for (Component* component : deserialisedComponents) {
				if (component->uuid == componentID) {
					entity->addComponent(component);
					foundComponent = true;
					break;
				}
			}
			
			// TODO raise error if we can't find component ID
			if (!foundComponent) {
				Logger::Print("Unable to find component that matches UUID \"" + componentID + "\"", LOGCAT::CAT_CORE, LOGSEV::SEV_ERROR);
			}

		}
	}


	// TODO if we don't have new root, delete new entities and error
	// Now give each Entity a real parent
	for (Entity* entity : deserialisedEntities) {
		entity->finishSerialisation(deserialisedEntities);
	}

	for (Component* component : deserialisedComponents) {
		component->finishSerialisation(deserialisedComponents);
	}

	return true;
}

/*
Serialise a single Entity
This is used internally by saveRoot
*/
int core::serialiseEntity(cereal::JSONOutputArchive& archive, Entity* entity, std::vector<Entity*>& addedEntities)
{
	if (std::find(addedEntities.begin(), addedEntities.end(), entity) != addedEntities.end()) {
		return 0; // Already archived
	}

	// Serialise Entity itself
	archive(cereal::make_nvp(entity->getName(), *entity));
	addedEntities.push_back(entity);
	
	// Add children
	//std::vector<Entity> children;
	//for (Entity* childEntity : entity->getChildren()) {
		//serialiseEntity(archive, childEntity, addedEntities);
	//	children.push_back(*childEntity);
	//}
	//archive(cereal::make_nvp("children", children));

	return 0;
}



int core::updatePhysics() {
	collisionManager->handleCollisions();
	return 0;
}


bool core::Update() {
	Logger::Print("Tick.In (" + std::to_string(multithreaded) + ")", LOGCAT::CAT_CORE, LOGSEV::SEV_TRACE);

	// Update input state
	inputManager->Update();

	// Reset whether every object is in the tree
	Group* allEntitiesGroup = groupManager->getGroup("Entity");
	auto allEntities = allEntitiesGroup->getEntities();

	// Assume we aren't in tree
	for (Entity* entity : allEntities ) {
		entity->inTree = false;
	}
	// Set that we are
	rootEntity->updateInTree(true, true);

	// If changed, do callback
	allEntities = allEntitiesGroup->getEntities(); // Some may have been deleted
	for (Entity* entity : allEntities) {
		if (entity->oldInTree != entity->inTree) {
			if (entity->callbackInTreeChange) entity->callbackInTreeChange(entity);

			for (Component* component : entity->components) {
				if (component->callbackInTreeChange) component->callbackInTreeChange(component);
			}
		}
	}
	

	// Update root (this will recursively update all children)
	rootEntity->Update();



	if (multithreaded) {
		threadManager->continueLoopThread(threadManager->physicsThreadData);
		
	}
	else {
		bool res = updateFrame(false);
		if (!res) {
			return false;
		}
	}
	
	
	Logger::Print("Tick.Out", LOGCAT::CAT_CORE, LOGSEV::SEV_TRACE);
	return true;
}


bool core::Render() {
//    int x, y;
//    glfwGetWindowSize(window, &x, &y);
//    renderManager->updateScreenSize(glm::ivec2(x, y));
	Logger::Print("Render.In (" + std::to_string(multithreaded) + ")", LOGCAT::CAT_RENDERING, LOGSEV::SEV_TRACE);
	
	// Render root (this will recursively render all children)
	rootEntity->Render();

	if (multithreaded) {
		// If we have the render context, release it (the render thread needs it)
		if (threadManager->hasRenderContext() ) {
			threadManager->releaseRenderContext();
		}

		threadManager->continueLoopThread(threadManager->renderThreadData);

		//threadManager->renderStarted = true;
		// Enable rendering on render thread
		//threadManager->renderLock.unlock();
		//threadManager->renderCondition.notify_one();
		
	}
	else {
		bool res = renderFrame(false);
		if (!res) {
			return false;
		}
	}
	
	

	Logger::Print("Render.Out", LOGCAT::CAT_RENDERING, LOGSEV::SEV_TRACE);
	return true;
}

bool core::updateFrame(bool isThreaded)
{
	

	int res = updatePhysics();
	if (res) {
		return false;
	}
	return true;
}

bool core::renderFrame(bool isThreaded)
{

	elmt::core::getAnimationManager()->UpdateAll();
	// Draw scene
	if (activeCamera) {
        elmt::core::getRenderManager()->render(activeCamera->projectionMatrix, activeCamera->viewMatrix, activeCamera->viewDir, activeCamera->pos);
		// TODO add this back?

        //elmt::core::getRenderManager()->renderModelBoundingBoxes(activeCamera->projectionMatrix, activeCamera->viewMatrix, 8);
	}

	return true;
}

void core::setActiveCamera(Camera* cam) {
	activeCamera = cam;
	activeCamera->screenSize = renderManager->getScreenSize();
}


int core::startFrame() {
	updateDeltaTime();
	int res = threadManager->startFrame();
	if (res) {
		return res;
	}
	
	//threadManager->seizeRenderContext();
	return 0;
}

int core::endFrame()
{
	int res = threadManager->endFrame();
	if (res) {
		return res;
	}

	return 0;
	
}
