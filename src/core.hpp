#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>


#include <map>
#include <set>
#include <string>
#include <thread>

#include "cereal/cereal.hpp"
#include "cereal/archives/json.hpp"

namespace elmt {

	// Forward declarations
	class CollisionManager;
	class AnimationManager;
	class RenderManager;
	class AssetManager;
	class ThreadManager;
	class AudioManager;
	class ProcManager;
	class InputManager;
	class GroupManager;
    class Entity;
    class core;
    class Camera;
    class PhysicsWorld;
    class Component;

	class core
	{
		// Properties
	public:
		

		// Root object of the object tree
		inline static elmt::Entity* rootEntity = nullptr;

	private:
        int screenWidth, screenHeight;
		// Time since last frame
		inline static double dt = 0.0;

		// Used for dt calculation
		inline static double lastFrameTime = 0.0, currentFrameTime = 0.0;

		static std::map<std::string, std::set<elmt::Entity>> entityGroups;

//		elmt::PhysicsWorld world;

		inline static CollisionManager* collisionManager = nullptr;
		inline static AnimationManager* animationManager = nullptr;
		inline static RenderManager* renderManager = nullptr;
		inline static AssetManager* assetManager = nullptr;
		inline static ThreadManager* threadManager = nullptr;
		inline static AudioManager* audioManager = nullptr;
		inline static ProcManager* procManager = nullptr;
		inline static InputManager* inputManager = nullptr;
		inline static GroupManager* groupManager = nullptr;

		// Has the core been setup, and not cleaned up
		inline static bool isSetup = false;

		// The world used internally for physics processing
		inline static PhysicsWorld* physicsWorld;

		inline static GLFWwindow* window;
		// The camera currently used for rendering
		inline static Camera* activeCamera;
		// The camera that will be used if no custom cameras are made active
		inline static Camera* defaultCamera;

		// Whether or not multithreading is enabled
		inline static bool multithreaded;

		// Methods
		static int serialiseEntity(cereal::JSONOutputArchive& archive, Entity* entity, std::vector<Entity*>& addedEntities);

		
	public:
		static bool Setup(int window_width, int window_height, bool useMultithreading);
		static bool Setup(int window_width, int window_height) { return  Setup(window_width, window_height, true);  };

		/*
		Setup non-static managers for the engine
		nullptr can be passed for any of the arguments, in which case a default manager will be created
		*/
		static int setupManagers(CollisionManager* newCollisionManager,
			AnimationManager* newAnimationManager,
			RenderManager* newRenderManager,
			AssetManager* newAssetManager,
			ThreadManager* newThreadManager,
			AudioManager* newAudioManager,
			ProcManager* newProcManager,
			InputManager* newInputManager,
			GroupManager* newGroupManager);
		/*
		Setup all default managers
		*/
		static int setupManagers() { return setupManagers(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr); }
		static bool Teardown();

		/*
		Update all game engine elements related to logic
		If multithreading is enabled, the function will attempt to use multithreading for performance
		In this case, remember to use endFrame to ensure all threads are finished before going to the next frame
		*/
		static bool Update();
		/*
		Update all game engine elements related to rendering, then draw
		If multithreading is enabled, the function will attempt to use multithreading for performance
		In this case, remember to use endFrame to ensure all threads are finished before going to the next frame
		*/
		static bool Render();

		/*
		Internal function used for multithreaded updating
		If multithreading is not used, then this is just run within Update
		*/
		static bool updateFrame(bool isThreaded);
		/*
		Internal function used for multithreaded rendering
		If multithreading is not used, then this is just run within Render
		*/
		static bool renderFrame(bool isThreaded);


		/*
		Start a new frame. This is currently only required if multithreading is used
		*/
		static int startFrame();

		/*
		End the current tick/frame. This is currently only required if multithreading is used
		*/
		static int endFrame();
		
		static double updateDeltaTime();
		static double getDeltaTime() { return dt; };

		static std::string generateUUID();

		static bool saveRoot(const char* filePath);
		
		static bool loadRoot(const char* filePath);

		static Entity* createEmptyEntity(const std::string& entityType);
		static Component* createEmptyComponent(const std::string& componentType);

		/*
		Clone a component that can be one of multiple types
		This can be used to clone components at runtime
		*/
		static void cloneComponent(Component* componentPointer, Component* clonePointer, Entity* entityToAttach);

		static CollisionManager* getCollisionManager() { return collisionManager; };
		static AnimationManager* getAnimationManager() { return animationManager; };
		static RenderManager* getRenderManager() { return renderManager; };
		static AssetManager* getAssetManager() { return assetManager; };
		static ThreadManager* getThreadManager() { return threadManager; };
		static AudioManager* getAudioManager() { return audioManager; };
		static ProcManager* getProcManager() { return procManager; }
		static InputManager* getInputManager() { return inputManager; }
		static GroupManager* getGroupManager() { return groupManager; }

		static PhysicsWorld* getPhysicsWorld() { return physicsWorld; };
		static GLFWwindow* getWindow() { return window; }
		static Camera* getActiveCamera() { return activeCamera; }
		static void setActiveCamera(Camera* cam);
		static Camera* getDefaultCamera() { return defaultCamera; }
		

		static const bool getIsSetup() { return isSetup; }
		static const bool getMultithreaded() { return multithreaded; }

	private:
		
		static bool setupWindow(int width, int height);

		// Update the physics for this frame
		static int updatePhysics();

		static void windowSizeCallback(GLFWwindow* window, int width, int height);






	};

}
