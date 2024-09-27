#include "core.hpp"
#include "Camera.hpp"
#include "GroupManager.hpp"

#include "../external/glm-0.9.7.1/glm/glm.hpp"
#include "../external/glm-0.9.7.1/glm/gtc/matrix_transform.hpp"
#include "../external/glm-0.9.7.1/glm/gtx/euler_angles.hpp"
#include "../external/glm-0.9.7.1/glm/gtx/orthonormalize.hpp"


using namespace elmt;

Camera::Camera(const char* name, Entity* parent, glm::vec3 pos, bool active) : Entity(name, parent, pos)
{
	core::getGroupManager()->addEntityToGroup("Camera", this);
    time = glfwGetTime();
    fov = 45.0f;

    typeName = "Camera";

    this->setActive(active);
}

//basic implementation to update the camera based on user input
//reuses provided code from rasterisation cw
bool Camera::Update() {
    bool res = Entity::Update();
    if (!res) return false;

    glm::vec3 up;
    glm::vec3 direction;

    // Remove roll
    if (!rollEnabled){
        rotation[1][0] = 0.0;
        rotation[1][1] = 1.0;
        rotation[1][2] = 0.0;

        up = getUp();
        direction = getForward();

        glm::vec3 orthoRight = glm::cross(up, direction);

        rotation[0][0] = orthoRight.x;
        rotation[0][1] = orthoRight.y;
        rotation[0][2] = orthoRight.z;
    }
    else {
        up = getUp();
        direction = getForward();
    }
    

    float ratio = (float)screenSize.x / (float)screenSize.y;
    projectionMatrix = glm::perspective(glm::radians(fov), ratio, 0.1f, 200.0f);


    

    // Camera matrix
    viewMatrix = glm::lookAt(
        pos,           // Camera is here
        pos + direction, // and looks here : at the same position, plus "direction"
        up                  // Head is up (set to 0,-1,0 to look upside-down)
    );

    viewDir = direction;

    return true;
}

Camera::Camera(const char* name, Entity* parent, glm::vec3 pos) : Camera(name, parent, pos, true) {};

void Camera::setActive(bool val) {
    active = val;

    Camera* oldCam = core::getActiveCamera();
    if (oldCam == this) {
        
        if (!active) { // Set default camera
            core::getDefaultCamera()->setActive(true);
        }
    } else {
        if (active) { // Override camera
            core::setActiveCamera(this);
            if (oldCam) {
                oldCam->setActive(false);
            }
            
        } 
    }
    
}



