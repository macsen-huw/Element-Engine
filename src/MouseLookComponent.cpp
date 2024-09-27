#include "miniaudio.h"

#include "RenderManager.hpp"
#include "AudioManager.hpp"
#include "InputManager.hpp"
#include "MouseLookComponent.hpp"
#include "Logger.hpp"
#include "core.hpp"
#include "Camera.hpp"
#include "../external/glm-0.9.7.1/glm/glm.hpp"
#include "../external/glm-0.9.7.1/glm/gtx/transform.hpp"

using namespace elmt;


MouseLookComponent::MouseLookComponent() : Component()
{
	typeName = "MouseLookComponent";
}

MouseLookComponent::MouseLookComponent(const char* name, Entity* entity, float speed, float mouseSpeed, bool freezeMouse) : Component(name, entity)
{
    this->speed = speed;
    this->mouseSpeed = mouseSpeed;
    this->freezeMouse = freezeMouse;
	typeName = "MouseLookComponent";
    
}




bool MouseLookComponent::Update()
{
	bool res = Component::Update();
	if (!res) return false;

    // This component can ONLY use a camera
    Camera* camEntity = (Camera*)entity;
    GLFWwindow* window = core::getWindow();

    float deltaTime = core::getDeltaTime();

    InputManager* inpMan = core::getInputManager();


    // Reset mouse position for next frame
    if (!freezeMouse) {

        RenderManager* rm = core::getRenderManager();
        glm::ivec2 screenSize = rm->getScreenSize();

        inpMan->setMousePos(screenSize.x / 2, screenSize.y / 2);

        float dx, dy;
        dx = (screenSize.x / 2) - inpMan->getMouseX();
        dy = (screenSize.y / 2) - inpMan->getMouseY();
        // Compute new orientation
        float h = mouseSpeed * float(dx);
        float v = -mouseSpeed * float(dy);

        glm::vec3 axis = camEntity->getRight();
        glm::mat4x4 normal = glm::rotate(glm::mat4x4(),
            v,
            axis
        );

        glm::mat4x4 tangent = glm::rotate(glm::mat4x4(),
            h,
            glm::vec3(0.0, 1.0, 0.0)
        );



        entity->rotation = tangent * normal * entity->rotation;
        //glfwSetInputMode(core::getWindow(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        
        
        
    }
    else {
        //glfwSetInputMode(core::getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    glm::vec3 camDirection = camEntity->getForward();
    glm::vec3 camRight = glm::normalize(camEntity->getRight());


    if (enableFreeCam) {

        // Move forward
        if (inpMan->keyPressed(glfwGetKeyScancode(GLFW_KEY_UP))) {
            camEntity->pos += camDirection * deltaTime * speed;
        }
        // Move backward
        if (inpMan->keyPressed(glfwGetKeyScancode(GLFW_KEY_DOWN))) {
            camEntity->pos -= camDirection * deltaTime * speed;
        }
        // Strafe right
        if (inpMan->keyPressed(glfwGetKeyScancode(GLFW_KEY_RIGHT))) {
            camEntity->pos += camRight * deltaTime * speed;
        }
        // Strafe left
        if (inpMan->keyPressed(glfwGetKeyScancode(GLFW_KEY_LEFT))) {
            camEntity->pos -= camRight * deltaTime * speed;
        }
    }

	return true;

}

void MouseLookComponent::clone(Component*& clonePointer, Entity* entityToAttach)
{
	clonePointer = new MouseLookComponent(name.c_str(), entityToAttach, speed, mouseSpeed, freezeMouse);
	Logger::Print("Cloned MouseLookComponent " + name + ", UUID " + clonePointer->getName(), LOGCAT::CAT_CORE, LOGSEV::SEV_INFO | LOGSEV::SEV_TRACE);
}