#include "MeshRenderer.hpp"
#include "Group.hpp"
#include "Logger.hpp"

using namespace elmt;

MeshRenderer::MeshRenderer() : Component()
{
    callbackInTreeChange = treeChangeCallback;
    typeName = "MeshRenderer";
}

MeshRenderer::MeshRenderer(const char* name, Entity* entity, Model* model) : Component(name, entity), model(model) {
    callbackInTreeChange = treeChangeCallback;
    
    if (entity && model && model->isValid()) {
        instanceID = model->renderManager->addInstance(entity->getName(), model->modelID);
        instanceIDs.push_back(instanceID);
        models.push_back(model);
        updateInfo();
    }
    else {
        Logger::Print("[Error/MeshRenderer]: Invalid Model in: " + std::string(name) + "\n", LOGCAT::CAT_RENDERING, LOGSEV::SEV_ERROR);
    }
    typeName = "MeshRenderer";
}


MeshRenderer::MeshRenderer(const char *name, Entity *entity, std::vector<Model*> &models, std::vector<float> &lodPartitions) : Component(name, entity){

    callbackInTreeChange = treeChangeCallback;
    typeName = "MeshRenderer";

    std::vector<size_t> modelIDs;

    for (auto mod : models){
        if (!mod || !mod->isValid()){
            Logger::Print("[Error/MeshRenderer]: Invalid Model in: " + std::string(name) + "\n", LOGCAT::CAT_RENDERING, LOGSEV::SEV_ERROR);
            continue; //model invalid don't try and use it
        }
        modelIDs.push_back(mod->modelID);
        this->models.push_back(mod);
    }

    model = models[0];
    instanceIDs = model->renderManager->addInstance(entity->getName(), modelIDs, lodPartitions);
}


void MeshRenderer::updateInfo() {
    InstanceProperties newProperties{};
    newProperties.translationMatrix = glm::mat4x4(1.f);
    newProperties.translationMatrix[3][0] = this->getEntity()->pos.x;
    newProperties.translationMatrix[3][1] = this->getEntity()->pos.y;
    newProperties.translationMatrix[3][2] = this->getEntity()->pos.z;

    glm::mat4x4 scaleMatrix = glm::mat4x4(1.f);
    scaleMatrix[0][0] = this->getEntity()->scale.x;
    scaleMatrix[1][1] = this->getEntity()->scale.y;
    scaleMatrix[2][2] = this->getEntity()->scale.z;

    newProperties.rotationMatrix = this->getEntity()->rotation * scaleMatrix;

    bool isVisible;
    if (this->getEntity()->getInTree()) {
        if (visible) {
            isVisible = true;
        }
        else {
            isVisible = false;
        }
    }
    else {
        isVisible = false;
    }
    newProperties.active = isVisible;

    for (size_t i = 0; i < instanceIDs.size(); i++) {
        model->renderManager->updateInstance(newProperties, models[i]->modelID, instanceIDs[i]);
    }
}

bool MeshRenderer::Update()
{
    updateInfo();
    
    return true;
}

void MeshRenderer::clone(Component*& clonePointer, Entity* entityToAttach)
{
    clonePointer = new MeshRenderer(this->name.c_str(), entityToAttach, model);
    Logger::Print("Cloned MeshRenderer " + name + ", UUID " + clonePointer->getUUID(), LOGCAT::CAT_CORE, LOGSEV::SEV_INFO | LOGSEV::SEV_TRACE);
}

bool MeshRenderer::treeChangeCallback(Component* c) {
    MeshRenderer* mr = (MeshRenderer*)c;
    mr->updateInfo();

    return true;
}

MeshRenderer::~MeshRenderer() {
    for (size_t i = 0; i < instanceIDs.size(); i++) {
        model->renderManager->deleteInstance(models[i]->modelID, instanceIDs[i]);
    }
    Component::~Component();
}
