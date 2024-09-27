#include "InputManager.hpp"
#include "InputAction.hpp"
#include "core.hpp"
#include "imgui/backends/imgui_impl_glfw.h";


using namespace elmt;


InputManager::InputManager()
{

    glfwSetKeyCallback(core::getWindow(), keyCallback);
    glfwSetMouseButtonCallback(core::getWindow(), mouseButtonCallback);
}

InputManager::~InputManager()
{
    glfwSetKeyCallback(core::getWindow(), NULL);
    glfwSetMouseButtonCallback(core::getWindow(), NULL);
}

bool InputManager::keyPressed(int key, std::map<int, bool>& keyState)
{ 
    bool res;
    if (keyState.find(key) != keyState.end()) {
        int state = keyState[key];
        if (state != GLFW_RELEASE) {
            res = true;
        }  else {
            res = false;
        }     
    } else {
        keyState[key] = GLFW_RELEASE;
        res = false;
    }
    return res;
}
bool InputManager::keyJustPressed(int key)
{
    bool oldState = keyPressed(key, oldKeyMap);
    bool currentState = keyPressed(key, keyMap);
    if (!oldState && currentState) {
        return true;
    }
    else {
        return false;
    }
}
bool InputManager::keyJustReleased(int key)
{
    bool oldState = keyPressed(key, oldKeyMap);
    bool currentState = keyPressed(key, keyMap);
    if (oldState && !currentState) {
        return true;
    }
    else {
        return false;
    }
}

std::vector<int> InputManager::getJustPressedKeys()
{
    std::vector<int> res;
    for (std::pair<int, int> p : lastFrameKeys) {
        if (p.second != GLFW_RELEASE) {
            res.push_back(p.first);
        }
    }
    return res;
}

std::vector<int> InputManager::getJustReleasedKeys()
{
    std::vector<int> res;
    for (std::pair<int, int> p : lastFrameKeys) {
        if (p.second == GLFW_RELEASE) {
            res.push_back(p.first);
        }
    }
    return res;
}



bool InputManager::mouseButtonPressed(int mouseButton, std::map<int, bool>& mouseButtonState)
{
    bool res;
    if (mouseButtonState.find(mouseButton) != mouseButtonState.end()) {
        int state = mouseButtonState[mouseButton];
        if (state != GLFW_RELEASE) {
            res = true;
        }
        else {
            res = false;
        }
    }
    else {
        mouseButtonState[mouseButton] = GLFW_RELEASE;
        res = false;
    }
    return res;
}
bool InputManager::mouseButtonJustPressed(int mouseButton)
{
    bool oldState = mouseButtonPressed(mouseButton, oldMouseButtonMap);
    bool currentState = mouseButtonPressed(mouseButton, mouseButtonMap);
    if (!oldState && currentState) {
        return true;
    }
    else {
        return false;
    }
}
bool InputManager::mouseButtonJustReleased(int mouseButton)
{
    bool oldState = mouseButtonPressed(mouseButton, oldMouseButtonMap);
    bool currentState = mouseButtonPressed(mouseButton, mouseButtonMap);
    if (oldState && !currentState) {
        return true;
    }
    else {
        return false;
    }
}

std::vector<int> InputManager::getJustPressedMouseButtons()
{
    std::vector<int> res;
    for (std::pair<int, int> p : lastFrameMouseButtons) {
        if (p.second != GLFW_RELEASE) {
            res.push_back(p.first);
        }
    }
    return res;
}

std::vector<int> InputManager::getJustReleasedMouseButtons()
{
    std::vector<int> res;
    for (std::pair<int, int> p : lastFrameMouseButtons) {
        if (p.second == GLFW_RELEASE) {
            res.push_back(p.first);
        }
    }
    return res;
}




bool InputManager::comboPressed(KeyCombo& combo, std::map<int, bool>& keyState, std::map<int, bool>& mouseButtonState)
{

    // TODO mods and mouse
    bool pressed = true;
    if (!combo.enabled || (combo.keys.empty() && combo.mouseButtons.empty()) ) {
        return false;
    }
    for (int key : combo.keys) {
        if (!keyPressed(key, keyState) ) {
            pressed = false;
            break;
        }
    }

    if (pressed) {
        for (int mouseButton : combo.mouseButtons) {
            if (!mouseButtonPressed(mouseButton, mouseButtonState)) {
                pressed = false;
                break;
            }
        }
    }
    return pressed;
}

bool InputManager::comboJustPressed(KeyCombo& combo)
{
    bool oldState = comboPressed(combo, oldKeyMap, oldMouseButtonMap);
    bool currentState = comboPressed(combo, keyMap, mouseButtonMap);
    if (!oldState && currentState) {
        return true;
    }
    return false;
}

bool InputManager::comboJustReleased(KeyCombo& combo)
{
    bool oldState = comboPressed(combo, oldKeyMap, oldMouseButtonMap);
    bool currentState = comboPressed(combo, keyMap, mouseButtonMap);
    if (oldState && !currentState) {
        return true;
    }
    return false;
}


void InputManager::setMousePos(double mx, double my)
{
    glfwSetCursorPos(core::getWindow(), mx, my);
}

int InputManager::Update()
{

    // Update actions
    for (InputAction* action : actions) {
        action->Update();
    }


    oldKeyMap = keyMap;
    oldMouseButtonMap = mouseButtonMap;
    lastFrameKeys.clear();
    lastFrameMouseButtons.clear();
    
    oldMouseX = mouseX;
    oldMouseY = mouseY;

    glfwGetCursorPos(core::getWindow(), &mouseX, &mouseY);

	// Update mouse position
	ImGuiIO& io = ImGui::GetIO();
	io.MousePos = ImVec2((float)mouseX, (float)mouseY);

	// Update mouse button status
	io.MouseDown[0] = glfwGetMouseButton(core::getWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
	io.MouseDown[1] = glfwGetMouseButton(core::getWindow(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
	io.MouseDown[2] = glfwGetMouseButton(core::getWindow(), GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;


    
    glfwPollEvents();

    return 0;
}

void InputManager::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    InputManager* im = core::getInputManager();
    im->keyMap[scancode] = action;

    im->lastFrameKeys.push_back({ scancode,action });
}

void InputManager::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
    InputManager* im = core::getInputManager();
    im->mouseButtonMap[button] = action;

    im->lastFrameMouseButtons.push_back({ button,action });
}

void InputManager::scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	ImGuiIO& io = ImGui::GetIO();
	io.MouseWheelH += (float)xoffset;
	io.MouseWheel += (float)yoffset;
}

void InputManager::charCallback(GLFWwindow* window, unsigned int c)
{
	ImGuiIO& io = ImGui::GetIO();
	if (c > 0 && c < 0x10000)
		io.AddInputCharacter((unsigned short)c);
}
