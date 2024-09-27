

#include <iostream>

#include "../external/glm-0.9.7.1/glm/gtc/matrix_transform.hpp"
#include "../external/glm-0.9.7.1/glm/gtx/euler_angles.hpp"
#include "DebugRenderObject.hpp"

#include "core.hpp"

#include "element.hpp"

#include "globals.hpp"
#include "Level.hpp"
#include "GameObject.hpp"
#include "Player.hpp"
#include "Platform.hpp"
#include "Collectable.hpp"
#include "FallingRock.hpp"
#include "Trigger.hpp"

using namespace globals;

int main(int argc, char** argv) {
    std::cout << "Demo Go Go" << std::endl;

    int width = 1920;
    int height = 1080;
    elmt::core::Setup(width, height);

    // Setup globals
    g::Setup();



    g::gravity = { 0, -9.8 * 0.1, 0 };
    elmt::core::getPhysicsWorld()->setGravity( g::gravity);

    elmt::InputManager* inpMan = elmt::core::getInputManager();


    elmt::DirectionalLight *directionalLight1 = new elmt::DirectionalLight( "Light1", g::world, glm::normalize(glm::vec3(0, -1, 0)), glm::vec3(1, 1, 1), 0.7 );
    elmt::core::getRenderManager()->addDirectionalLight(directionalLight1);

    //"assets/yoshi_circuit/source/", "yoshi_circuit.obj"
    //auto testCircuit = GameObject("Circuit", g::world, { 0.0,-5.0,0.0 }, "circuit/yoshi_circuit.obj", true);
    //testCircuit.hasGravity = false;


    // Setup UI
    //auto labelEntity = new elmt::UIEntity("Test UI Entity", g::world, { 100,100 }, { 100,100 });
    //new elmt::UILabelComponent("Test Label", labelEntity, "TEST!");


    g::addModel("cube", "cube" + g::fileSep);

    g::player = new Player();
    g::player->getPhysComp()->setKinematic();
    g::player->hasGravity = false;



    g::level = new Level();
    g::level->Setup(g::GENERATE_TERRAIN);

    // skybox
    char skyboxFiles[12][1024] = { "assets/skybox/right.jpg", "assets/skybox/left.jpg", "assets/skybox/top.jpg",
                                   "assets/skybox/bottom.jpg", "assets/skybox/front.jpg", "assets/skybox/back.jpg",
                                   "assets/skybox/right.jpg", "assets/skybox/left.jpg", "assets/skybox/top.jpg",
                                   "assets/skybox/bottom.jpg", "assets/skybox/front.jpg", "assets/skybox/back.jpg"};
    elmt::Skybox skybox(skyboxFiles, 1000.f);
    elmt::core::getRenderManager()->setSkybox(&skybox);

    elmt::UIManager* uiMgr = elmt::core::getRenderManager()->getUIManager();
    elmt::UIEditor* uiEditor = new elmt::UIEditor(uiMgr);
    uiMgr->setUIEditor(uiEditor);

    auto col = new Collectable( g::player->pos + glm::vec3( 10,0,0 ), g::getModel("cube") );


    bool running = true;
    while (running) {

        double frameTime = elmt::core::startFrame();

        if (inpMan->keyJustPressed(glfwGetKeyScancode(GLFW_KEY_1))) {
            elmt::core::getRenderManager()->renderMode = elmt::NO_DEBUG;
        }


        else if (inpMan->keyJustPressed(glfwGetKeyScancode(GLFW_KEY_2))) {
            elmt::core::getRenderManager()->renderMode = elmt::NORMALS;
        }

        else if (inpMan->keyJustPressed(glfwGetKeyScancode(GLFW_KEY_3))) {
            elmt::core::getRenderManager()->renderMode = elmt::AMBIENT_OCCLUSION;
        }

        else if (inpMan->keyJustPressed(glfwGetKeyScancode(GLFW_KEY_4))) {
            elmt::core::getRenderManager()->renderMode = elmt::ENVIRONMENT_LIGHT;
        }

        else if (inpMan->keyJustPressed(glfwGetKeyScancode(GLFW_KEY_5))) {
            elmt::core::getRenderManager()->renderMode = elmt::RGBA_TEXTURE;
        }

        else if (inpMan->keyJustPressed(glfwGetKeyScancode(GLFW_KEY_6))) {
            elmt::core::getRenderManager()->renderMode = elmt::SHADOWS_ONLY;
        }

        else if (inpMan->keyJustPressed(glfwGetKeyScancode(GLFW_KEY_7))) {
            elmt::core::getRenderManager()->renderMode = elmt::MIPMAP_LEVELS;
        }

        else if (inpMan->keyJustPressed(glfwGetKeyScancode(GLFW_KEY_8))) {
            elmt::core::getRenderManager()->renderMode = elmt::BLINN_PHONG_SHADING;
        }

        else if (inpMan->keyJustPressed(glfwGetKeyScancode(GLFW_KEY_9))) {
            elmt::core::getRenderManager()->renderMode = elmt::DIFFUSE_ONLY;
        }

        else if (inpMan->keyJustPressed(glfwGetKeyScancode(GLFW_KEY_0))) {
            elmt::core::getRenderManager()->renderMode = elmt::SPECULAR_ONLY;
        }

        else if (inpMan->keyJustPressed(glfwGetKeyScancode(GLFW_KEY_B))) {
            elmt::core::getRenderManager()->renderMode = elmt::AMBIENT_ONLY;
        }

        else if (inpMan->keyJustPressed(glfwGetKeyScancode(GLFW_KEY_R))) {
            elmt::core::getRenderManager()->renderMode = elmt::ROUGHNESS_ONLY;
        }

        else if (inpMan->keyJustPressed(glfwGetKeyScancode(GLFW_KEY_M))) {
            elmt::core::getRenderManager()->renderMode = elmt::METALLICNESS_ONLY;
        }

        else if (inpMan->keyJustPressed(glfwGetKeyScancode(GLFW_KEY_X))) {
            elmt::core::getRenderManager()->renderMode = elmt::PARALLAX_ONLY;
        }


        if (g::inpMan->shouldCloseWindow()) {
            running = false;
        }
        if (g::inpMan->keyPressed(glfwGetKeyScancode(GLFW_KEY_ESCAPE))) {
            running = false;
        }
        if (inpMan->keyJustPressed(glfwGetKeyScancode(GLFW_KEY_P))) {
            g::player->mouseLook->freezeMouse = !g::player->mouseLook->freezeMouse;
        }

        g::colMan->refreshSceneBVH();

        elmt::core::Update();
        elmt::core::Render();
        elmt::core::endFrame();
    }

    elmt::core::Teardown();
    return EXIT_SUCCESS;
}
