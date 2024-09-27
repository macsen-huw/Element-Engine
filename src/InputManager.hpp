#pragma once
#include <iostream>
#include <vector>
#include <map>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "../external/glm-0.9.7.1/glm/glm.hpp"

#include "core.hpp"
#include <imgui.h>

namespace elmt {

	// Forward declaration
	class InputAction;

	/*
		Holds data for a single key/button combo that triggers an action
		*/
	struct KeyCombo {
		std::vector<int> keys = {}; // Keys that need to be pressed
		std::vector<int> mouseButtons = {}; // Mousebuttons that need to be clicked
		bool enabled = true; // Whether this combo is checked when determining if an action was pressed
		bool sequential = false; // Whether the keys of this combo must be pressed in order
	};

	class InputManager
	{
		
	private:
		// List of actions that the manager keeps track of
		std::vector<InputAction*> actions;

		// Which keys are currently pressed
		std::map<int, bool> keyMap = {};
		// Which keys were pressed last frame
		std::map<int, bool> oldKeyMap = {};

		// Which mouse buttons are currently pressed
		std::map<int, bool> mouseButtonMap = {};
		// Which mouse buttons were pressed last frame
		std::map<int, bool> oldMouseButtonMap = {};

		// Keys that were modified in some way between last frame and now
		std::vector< std::pair<int,int> > lastFrameKeys = {};
		// Mouse buttons that were modified in some way between last frame and now
		std::vector< std::pair<int, int>> lastFrameMouseButtons = {};

		double mouseX = 0.0, mouseY = 0.0;
		double oldMouseX = 0.0, oldMouseY = 0.0;
	public:
		InputManager();
		~InputManager();

		// KEYS
		// Check if a specific scancode is pressed
		bool keyPressed(int key, std::map<int, bool>& keyState);
		bool keyPressed(int key) { return keyPressed(key, keyMap); }

		// Check if a specific scancode was just pressed this frame
		bool keyJustPressed(int key);
		// Check if a specific scancode was just released this frame
		bool keyJustReleased(int key);

		// Get the keys that were pressed since last update
		std::vector<int> getJustPressedKeys();
		// Get the keys that were released since last update
		std::vector<int> getJustReleasedKeys();
		const std::map<int, bool>& getKeyMap() { return keyMap; };


		// MOUSE BUTTONS
		// Check if a specific scancode is pressed
		bool mouseButtonPressed(int mouseButton, std::map<int, bool>& mouseButtonState);
		bool mouseButtonPressed(int mouseButton) { return mouseButtonPressed(mouseButton, mouseButtonMap); }

		// Check if a specific scancode was just pressed this frame
		bool mouseButtonJustPressed(int mouseButton);
		// Check if a specific scancode was just released this frame
		bool mouseButtonJustReleased(int mouseButton);

		// Get the mouse buttons that were pressed since last update
		std::vector<int> getJustPressedMouseButtons();
		// Get the mouse buttons that were released since last update
		std::vector<int> getJustReleasedMouseButtons();
		const std::map<int, bool>& getMouseButtonMap() { return mouseButtonMap; };


		// KEY COMBOS
		/*
		Check if a specific key combo is pressed
		If "sequential" is set in the KeyCombo, then this may not be accuracte,
		use InputAction instead as it maintains combo progress
		*/
		bool comboPressed(KeyCombo& combo, std::map<int, bool>& keyState, std::map<int, bool>& mouseButtonState);
		bool comboPressed(KeyCombo& combo) { return comboPressed(combo, keyMap, mouseButtonMap); };

		// Check if a specific key combo was just pressed this frame
		bool comboJustPressed(KeyCombo& combo);
		// Check if a specific key combo was just released this frame
		bool comboJustReleased(KeyCombo& combo);


		// MOUSE
		// Get mouse pos
		double getMouseX() { return mouseX; };
		double getMouseY() { return mouseY; };
		glm::vec2 getMousePos() { return glm::vec2(getMouseX(), getMouseY()); };

		// Get how much the mouse position has changed since last frame
		double getMouseDX() { return mouseX - oldMouseX; };
		double getMouseDY() { return mouseY - oldMouseY; };
		glm::vec2 getMouseDPos() { return glm::vec2(getMouseDX(), getMouseDY()); };
		
		// Set mouse pos
		void setMousePos(double mx, double my);


		// WINDOW
		bool shouldCloseWindow() { return bool(glfwWindowShouldClose(core::getWindow())); };
		

	// Methods
	private:
		friend class core;
		int Update();

		static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
		static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
		static void charCallback(GLFWwindow* window, unsigned int c);
		

		friend class InputAction;

		
	};

}
