#include "DebugRenderObject.hpp"
#include "core.hpp"
#include "RenderManager.hpp"
#include "AssetManager.hpp"
#include "Logger.hpp"
#include "LogType.hpp"


using namespace elmt;

DebugRenderObject::DebugRenderObject(glm::vec3 origin, glm::vec3 dimensions, glm::vec3 colour, ObjectTypes type) {


    if (type == CUBE) model = core::getAssetManager()->importAsset("assets/cube/", "cube.obj", false);
    else if (type == SPHERE) model = core::getAssetManager()->importAsset("assets/sphere/", "sphere.obj", false);
    else{
        Logger::Print("Error - attempting to create an invalid debug object\n", LOGCAT::CAT_RENDERING, LOGSEV::SEV_ERROR);
        exit(EXIT_FAILURE);
    }

    model->materials[model->meshes[0].materialIndex].ambient = colour;
    model->materials[model->meshes[0].materialIndex].diffuse = colour;

    glm::mat4x4 scaleMatrix(1.f);
    scaleMatrix[0][0] = dimensions.x / 2.f;
    scaleMatrix[1][1] = dimensions.y / 2.f;
    scaleMatrix[2][2] = dimensions.z / 2.f;

    glm::mat4x4 translationMatrix(1.f);
    translationMatrix[3] = glm::vec4(origin, 1);

    glm::mat4x4 transformMatrix = translationMatrix * scaleMatrix;
    InstanceProperties properties;
    properties.translationMatrix = transformMatrix;


    instanceID = core::getRenderManager()->addInstance("Test box", model->modelID);
    core::getRenderManager()->updateInstance(properties, model->modelID, instanceID);
}

DebugRenderObject::DebugRenderObject(glm::vec3 dimensions, glm::vec3 colour, ObjectTypes type) {

    // From https://stackoverflow.com/questions/12971499/how-to-get-the-file-separator-symbol-in-standard-c-c-or
    const std::string kPathSeparator =
    #ifdef _WIN32
            "\\";
    #else
            "/";
    #endif


    if (type == CUBE) model = core::getAssetManager()->importAsset("assets" + kPathSeparator + "cube" + kPathSeparator, "cube.obj", false);
    else if (type == SPHERE) model = core::getAssetManager()->importAsset("assets" + kPathSeparator + "sphere" + kPathSeparator, "sphere.obj", false);
    else{
        Logger::Print("Error - attempting to create an invalid debug object\n", LOGCAT::CAT_RENDERING, LOGSEV::SEV_ERROR);
        exit(EXIT_FAILURE);
    }

    model->materials[model->meshes[0].materialIndex].ambient = colour;
    model->materials[model->meshes[0].materialIndex].diffuse = colour;

}
