#pragma once
#include "InputManager.hpp"
#include "core.hpp"

namespace elmt {


	class InputAction
	{
	// Attributes
	private:
		// List of combos that can activate this action
		std::vector<KeyCombo> combos = {};

		std::vector<unsigned int> comboSequentialState = {};
		std::vector<unsigned int> oldComboSequentialState = {};


	// Methods
	private:
		
		bool isSequentialComboPressed(size_t index, const std::vector<unsigned int>& state);

		// Is this action currently pressed
		bool isPressed(std::map<int, bool>& keyState, std::map<int, bool>& mouseButtonState, std::vector<unsigned int> sequentialState);

		

	public:
		/*
		Update this action.Only used currently for sequential combos.
		Called automatically by InputManager
		Note that is(Just)Pressed / Release will work even without updating
		*/
		int Update();

		// Single Combo
		InputAction(KeyCombo combo);
		// Multiple Combos
		InputAction(std::vector<KeyCombo> combos);
		// Key scancodes for a single Combo
		InputAction(std::vector<int> keys);
		// Key scancodes for multiple combos
		InputAction(std::vector<std::vector<int>> comboKeys);

		InputAction() : InputAction(std::vector<int>{}) {};

		~InputAction();

		// Is this action currently pressed
		bool isPressed() { return isPressed(core::getInputManager()->keyMap, core::getInputManager()->mouseButtonMap, comboSequentialState); };

		// Did this action become pressed during the last InputManager update
		bool isJustPressed();

		// Did this action stop being pressed during the last InputManager update
		bool isJustReleased();

		const std::vector<KeyCombo>& getCombos() { return combos; }

		// Change the combos this InputAction listens for. Will reset the progress of any sequential combos
		void updateCombos(std::vector<KeyCombo> newCombos);
		void updateCombos(std::vector<int> newComboKeys);
		void updateCombos(std::vector<std::vector<int>> comboKeys);

	};

}

