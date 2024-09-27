#pragma once

#include <string>

#include "element.hpp"

// Forward declaration
class Player;
class Level;

enum GameState {
	NOTSTARTED,
	STARTED,
	WIN,
	GAMEOVER
};

namespace globals {

	// Use this for storing constants and global data
	class g {
	public:

		static inline const std::string fileSep =
		#ifdef _WIN32
		"\\";
		#else
		"/";
		#endif


		static inline bool GENERATE_TERRAIN = true;

		static inline std::string BASE_PATH = "demo" + fileSep + "data" + fileSep;
		static inline std::string SOUND_PATH = BASE_PATH + "sound" + fileSep;

		static inline glm::vec3 gravity;
		static inline elmt::Entity* world;
		
		static inline elmt::Entity* platformsBase;
		static inline elmt::Entity* collectablesBase;
		static inline elmt::Entity* triggersBase;

		static inline elmt::InputManager* inpMan;
		static inline elmt::RenderManager* rendMan;
		static inline elmt::AssetManager* assMan;
		static inline elmt::ProcManager* procMan;
		static inline elmt::GroupManager* groupMan;
		static inline elmt::CollisionManager* colMan;
		static inline elmt::AudioManager* audMan;
		static inline elmt::UIManager* uiMan;


		static inline Player* player;

		static inline elmt::InputAction turnLeft;
		static inline elmt::InputAction turnRight;
		
		static inline elmt::InputAction moveForward;
		static inline elmt::InputAction moveBackward;

        static inline elmt::InputAction moveUp;
        static inline elmt::InputAction moveDown;


        static inline Level* level;

		static inline std::map<std::string, elmt::Model*> modelList;

		static void Setup();
		static void Cleanup();

		inline static unsigned int idNext = 0;

		static inline GameState state = GameState::NOTSTARTED;

		static std::string getObjectID();

		// Add model to list of models in the level (.obj is appended to the end)
		static void addModel(std::string modelName, std::string modelPath);

		// Get a model from the list of models in the level
		static elmt::Model* getModel(std::string modelName);
		
		
	};
}
