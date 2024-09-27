#include "InputAction.hpp"
#include "Logger.hpp"
#include "core.hpp"

using namespace elmt;

int InputAction::Update()
{
	InputManager* im = core::getInputManager();

	oldComboSequentialState = comboSequentialState;

	unsigned comboI = 0;
	for (KeyCombo& combo : combos) {
		if (!combo.enabled || combo.keys.empty() ) {
			continue;
		}

		unsigned int comboProgress = comboSequentialState[comboI];

		bool comboValid = true;
		int keyToCheck;
		// Check whether previous keys are still held
		for (unsigned int i = 0; i < comboProgress; i++) {
			keyToCheck = combo.keys[i];
			if (!im->keyPressed(keyToCheck)) {
				comboValid = false;
				break;
			}
		}

		// Combo no longer valid, reset progress
		if (!comboValid) {
			comboSequentialState[comboI]--;
			comboI++;
			continue;
		}

		// Now check new key
		if (comboProgress < combo.keys.size()) {
			keyToCheck = combo.keys[comboProgress];
			// Combo has progressed
			if (im->keyJustPressed(keyToCheck)) {
				comboProgress++;
				comboSequentialState[comboI] = comboProgress;
			}
		}
			
		comboI++;
	}
	return 0;
}


InputAction::InputAction(std::vector<KeyCombo> combos)
{
	updateCombos(combos);
	if (core::getIsSetup()) { core::getInputManager()->actions.push_back(this); }
}

InputAction::InputAction(KeyCombo combo) : InputAction(std::vector<KeyCombo>{ combo }) {}


InputAction::InputAction(std::vector<int> keys)
{
	updateCombos(keys);
	if (core::getIsSetup()) { core::getInputManager()->actions.push_back(this); }
}


InputAction::InputAction(std::vector<std::vector<int>> comboKeys)
{
	updateCombos(comboKeys);
	if (core::getIsSetup()) { core::getInputManager()->actions.push_back(this); }
}



InputAction::~InputAction(){
	InputManager* im = core::getInputManager();
	if (core::getIsSetup()) {
		auto it = std::find(im->actions.begin(), im->actions.end(), this);
		if (it != im->actions.end()) {
			im->actions.erase(it);
		}
		else {
			Logger::Print("Attempted to remove InputAction from ActionManager, but it was not found", LOGCAT::CAT_CORE, LOGSEV::SEV_WARNING);
		}
	}

}

bool InputAction::isSequentialComboPressed(size_t index, const std::vector<unsigned int>& state) {
	KeyCombo& c = combos[index];
	unsigned int progress = state[index];
	if (progress == c.keys.size()) {
		return true;
	}
	else {
		return false;
	}
}

bool InputAction::isPressed(std::map<int, bool>& keyState, std::map<int, bool>& mouseButtonState, std::vector<unsigned int> sequentialState)
{
	InputManager* ti = core::getInputManager();
	bool foundCombo = false;
	size_t comboI = 0;
	for (KeyCombo& combo : combos) {
		if (combo.sequential) {
			if (isSequentialComboPressed(comboI, sequentialState)) {
				foundCombo = true;
				break;
			}
		}
		else {
			if (ti->comboPressed(combo, keyState, mouseButtonState)) {
				foundCombo = true;
				break;
			}
		}
		comboI++;
		
	}
	return foundCombo;
}

bool InputAction::isJustPressed()
{
	InputManager* im = core::getInputManager();

	bool oldState = isPressed(im->oldKeyMap, im->oldMouseButtonMap, oldComboSequentialState);
	bool currentState = isPressed(im->keyMap, im->mouseButtonMap, comboSequentialState);

	if (!oldState && currentState) {
		return true;
	}
	return false;
}

bool InputAction::isJustReleased()
{
	InputManager* im = core::getInputManager();

	bool oldState = isPressed(im->oldKeyMap, im->oldMouseButtonMap, oldComboSequentialState);
	bool currentState = isPressed(im->keyMap, im->mouseButtonMap, comboSequentialState);

	if (oldState && !currentState) {
		return true;
	}
	return false;
}

void InputAction::updateCombos(std::vector<KeyCombo> newCombos)
{
	combos = newCombos;
	comboSequentialState.resize(combos.size());
	std::fill(comboSequentialState.begin(), comboSequentialState.end(), 0);
	oldComboSequentialState = comboSequentialState;
}

void InputAction::updateCombos(std::vector<int> newKeys)
{

	KeyCombo combo;
	combo.keys = newKeys;
	return updateCombos({combo});

}

void InputAction::updateCombos(std::vector< std::vector<int>> newComboKeys)
{
	std::vector<KeyCombo> combos = {};
	for (std::vector<int>& keys : newComboKeys) {
		KeyCombo combo;
		combo.keys = keys;
		combos.push_back(combo);
	}

	return updateCombos(combos);
}