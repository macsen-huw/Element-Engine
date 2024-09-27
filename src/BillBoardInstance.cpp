#include "BillBoardInstance.hpp"

using namespace elmt;

//BillboardInstance::BillboardInstance(const char* name, Entity* parent, glm::vec3 pos, BillboardMesh* billboardMesh)
//	: position(pos), modelTransform(glm::mat4(1.0f)), Entity(name, parent, pos), billboardMesh(billboardMesh)
//{
//	Group::addEntityToGroup("ModelInstance", this);
//	instanceID = billboardMesh->renderManager->addInstance(billboardMesh->billboardID);
//	update();
//}
//
///*
//* Render the ModelInstance (basically, render the model at a position)
//*/
//bool BillboardInstance::Render() {
//
//	bool res = true; // Set to false if something went wrong
//	// TODO render here
//	if (!res) return false;
//
//
//	res = Entity::Render();
//	if (!res) return false;
//	return true;
//}
//
//void BillboardInstance::update()
//{
//	glm::vec3 dir = position - pos; // direction from player
//	float theta = glm::atan(dir.y, dir.x);
//	float distance2D = glm::sqrt(dir.x * dir.x + dir.y * dir.y);
//	float phi = glm::atan(-dir.z, distance2D);
//
//	modelTransform = glm::mat4(1.0f);
//	modelTransform = glm::translate(modelTransform, position)
//		* glm::eulerAngleXYZ(0.0F, 0.0f, theta)
//		* glm::eulerAngleXYZ(0.0F, phi, 0.0f);
//
//	InstanceProperties newProperties{};
//	newProperties.translationMatrix = glm::mat4x4(1.f);
//	newProperties.translationMatrix = modelTransform;
//
//	newProperties.rotationMatrix = glm::mat4x4(1.f);
//	newProperties.active = true;
//	billboardMesh->renderManager->updateInstance(newProperties, billboardMesh->billboardID, instanceID);
//}
