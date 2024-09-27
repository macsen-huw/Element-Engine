#include "ModelInstance.hpp"
#include "GroupManager.hpp"
#include "../external/glm-0.9.7.1/glm/gtc/matrix_transform.hpp"
using namespace elmt;

ModelInstance::ModelInstance(const char* name, Entity* parent, glm::vec3 pos, Model* model): Entity(name, parent, pos), model(model)
{
    core::getGroupManager()->addEntityToGroup("ModelInstance", this);
    assert(0);
    instanceID = 0;
    updateInfo();

    typeName = "ModelInstance";    
}

/*
* Render the ModelInstance (basically, render the model at a position)
*/
bool ModelInstance::Render() {
	
	bool res = true; // Set to false if something went wrong
	// TODO render here
	if (!res) return false;


	res = Entity::Render();
	if (!res) return false;
	return true;

}

void ModelInstance::updateInfo() {
//    properties.translationMatrix = glm::mat4x4(1.f);
//    properties.translationMatrix[3][0] = pos.x;
//    properties.translationMatrix[3][1] = pos.y;
//    properties.translationMatrix[3][2] = pos.z;
//
//    // Update rotation matrix based on euler angles
//   /* float cosX = cosf(rotation.x);
//    float sinX = sinf(rotation.x);
//
//    float cosY = cosf(rotation.y);
//    float sinY = sinf(rotation.y);
//
//    float cosZ = cosf(rotation.z);
//    float sinZ = sinf(rotation.z);
//
//    glm::mat4x4 rotMatrix{
//        (cosY * cosZ), (sinX * sinY * cosZ) - (cosX * sinZ), (cosX * sinY * cosZ) + (sinX * sinZ), 0.0,
//        (cosY * sinZ), (sinX * sinY * cosZ) + (cosX * cosZ), (cosX * sinY * sinZ) - (sinX * cosZ), 0.0,
//        -sinY,         (sinX * cosY),                        (cosX * cosY),                        0.0,
//        0.0,           0.0,                                  0.0,                                  1.0
//    };
//
//    */
//    properties.rotationMatrix = rotation;// glm::mat4x4(1.f);
//
//
//    //With updated position and rotation, we now update the instance
//    properties.active = true;
//    model->renderManager->updateInstance(properties, model->modelID, instanceID);

}
