#include <cstdio>
#include <cstdlib>
#include <chrono>

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif


#include "Point.hpp"
#include "Group.hpp"
#include "ModelInstance.hpp"
#include "Camera.hpp"
#include "AssetManager.hpp"
#include "Model.hpp"

#include "Logger.hpp"
#include "Log.hpp"

#include "PhysicsComponent.hpp"
#include "PhysicsWorld.hpp"
#include "MeshRenderer.hpp"
#include "RenderManager.hpp"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "Skybox.hpp"
#include "MirrorObject.hpp"
#include "BillboardMesh.hpp"
#include "BillBoardInstance.hpp"
#include "CollisionManager.hpp"
#include "Material.hpp"
#include "AnimationManager.hpp"
#include "InputManager.hpp"
#include "InputAction.hpp"

#include "AudioManager.hpp"
#include "SoundComponent.hpp"
#include "MicrophoneComponent.hpp"
#include "DebugRenderObject.hpp"
#include "MouseLookComponent.hpp"

#include "ProcManager.hpp"
#include "Model.hpp"
#include "UIManager.hpp"
#include "UIEntity.hpp"
#include "UIEditor.hpp"
#include <functional>

//static bool boundingVolumeRender = false;
static bool freezeCamera = false;


elmt::SoundComponent* testSoundPointer;
elmt::Sound* testSoundPointer2 = nullptr;

void updateKart(elmt::PhysicsComponent& kartPhysics) {

    //Get the current velocities
    glm::vec3 currentLinearVel = kartPhysics.getLinearVelocity();
    glm::vec3 currentAngularVel = kartPhysics.getAngularVelocity();

    // Left/Right
    float yawChange = 0.0;
    float yawChangeSpeed = 1.0;

    elmt::InputManager* inpMan = elmt::core::getInputManager();

    if (inpMan->keyPressed(glfwGetKeyScancode(GLFW_KEY_A))) {
        yawChange = +2.0;
    }
    if (inpMan->keyPressed(glfwGetKeyScancode(GLFW_KEY_D))) {
        yawChange = -2.0;
    }
    if (inpMan->keyPressed(glfwGetKeyScancode(GLFW_KEY_LEFT_SHIFT)))
        yawChange *= 2.0;
    if (inpMan->keyPressed(glfwGetKeyScancode(GLFW_KEY_LEFT_CONTROL)))
        yawChange *= 0.5;

    glm::vec3 updatedAngularVel = { currentAngularVel.x, currentAngularVel.y + yawChange, currentAngularVel.z };

    kartPhysics.setAngularVelocity(0, yawChange, 0);
    //kartPhysics.applyTorque(0, yawChange * kartPhysics.getMass(), 0);

    // Forward/Backwards
    float kartSpeed = 0.0;
    if (inpMan->keyPressed(glfwGetKeyScancode(GLFW_KEY_W)))
        kartSpeed = 20.0;
    if (inpMan->keyPressed(glfwGetKeyScancode(GLFW_KEY_S)))
        kartSpeed = -20.0;
    if (inpMan->keyPressed(glfwGetKeyScancode(GLFW_KEY_LEFT_SHIFT)))
        kartSpeed *= 2.0;
    if (inpMan->keyPressed(glfwGetKeyScancode(GLFW_KEY_LEFT_CONTROL)))
        kartSpeed *= 0.5;
    kartSpeed *= 10.0;

    glm::vec4 initialVelocity = { 0, 0, kartSpeed, 1.0 };

    //Multiply with rotation matrix to get forward direction
    glm::vec4 updatedVelocity = kartPhysics.getRotationMatrix() * initialVelocity;

    //Make the force relative to the kart's mass
    updatedVelocity = updatedVelocity * kartPhysics.getMass();

    kartPhysics.applyForce(updatedVelocity.x, updatedVelocity.y, updatedVelocity.z);

    //kartPhysics.setLinearVelocity(updatedVelocity.x, currentVel.y, updatedVelocity.z);
}

elmt::DirectionalLight* globalDirectionalLight1 = NULL;

int main(int argc, char** argv) {

    // For now, treat main as if it were a game that is using the engine
    // Since ofc we don't "run" the engine, we use it as a base to make a game

    int width = 1080;
    int height = 720;
    elmt::core::Setup(width, height);

    glm::vec3 gravity = { 0, -9.8, 0 };

    elmt::core::getPhysicsWorld()->setGravity(gravity);


    // Setup game-specific entities
    elmt::Entity rootEntity{ "test entity 1", elmt::core::rootEntity };


    elmt::DirectionalLight directionalLight1{ "test light 2", &rootEntity, glm::normalize(glm::vec3(0, -1, 0)), glm::vec3(1, 1, 1), 0.7 };
    globalDirectionalLight1 = &directionalLight1;
    elmt::core::getRenderManager()->addDirectionalLight(&directionalLight1);
    //    glfwSetMouseButtonCallback(window, elmt::mouse_button_callback);


    // circuit
    glm::vec3 circuitPosition = { 0,0,0 };
    //	PhysicsBodyProps circuitProps = { world.getWorldID(), world.getSpaceID(), BOX };

    // yoshi
    glm::vec3 yoshiPosition = { 0, 60, 0 };

    //	PhysicsBodyProps yoshiProps = { world.getWorldID(), world.getSpaceID(), BOX };

//    elmt::Model* yoshiModel = elmt::core::getAssetManager()->importAsset("assets/yoshi/", "yoshi_model.obj");
//    elmt::Entity yoshi{ "Yoshi", &rootEntity, yoshiPosition };
//    elmt::MeshRenderer meshRendererYoshi("Yoshi Mesh", &yoshi, yoshiModel);
//    elmt::PhysicsComponent yoshiBody("Yoshi Rigidbody", &yoshi, elmt::PRIM_BOX, yoshiModel, true);
//    yoshiBody.setKinematic(); //yoshi is stronger than gravity



    //    char sphereName[] = "Debug sphere";
//    elmt::DebugRenderObject debugBox(glm::vec3{20, 50, 10}, glm::vec3{1, 1, 1}, glm::vec3{1, 0, 0}, elmt::CUBE);

    //    PhysicsBodyProps debugSphereProps = { world.getWorldID(), world.getSpaceID(), BOX };

    //    auto *sphereObject = new elmt::DebugRenderObject(glm::vec3{2, 2, 2}, glm::vec3{0, 1, 0}, elmt::SPHERE);
    //    elmt::Entity sphere{ "Sphere", &rootEntity, glm::vec3{10, 50, 10}};
    //    elmt::MeshRenderer meshRendererSphere{"Sphere mesh", &sphere, sphereObject->model};
    //    elmt::PhysicsComponent sphereBody("Sphere Rigidbody", &sphere, BOX, sphereObject->model, true);
    //    sphereBody.setKinematic();

    // circuit

    elmt::Model* circuitModel = elmt::core::getAssetManager()->importAsset("assets/yoshi_circuit/source/", "yoshi_circuit.obj");
    elmt::Entity circuit{ "Circuit", &rootEntity, circuitPosition };
    elmt::MeshRenderer meshRendererCircuit("Circuit Mesh", &circuit, circuitModel);
    elmt::PhysicsComponent circuitBody("Circuit Rigidbody", &circuit, elmt::PRIM_TRIMESH, circuitModel, true);
    circuitBody.setKinematic();
    //Set geoms
    //The road is made up of meshes 41, 46, 47, 48, 49, 50
//    int roadMeshes[] = { 41, 46, 47, 48, 49, 50 };
//
//    //Enable the geoms in the road
//    for (auto& mesh : roadMeshes)
//    {
//        //			circuitModel->meshes[i].materialIndex = 1;
//        circuitBody.enableGeom(circuitBody.getGeom()[mesh]);
//    }
//
//    circuitBody.setKinematic(); //Infinite mass, not affected by any forces
//    circuitBody.setContactMode(dContactSlip2 | dContactMu2);
//    //Set collision properties for the circuit
//    circuitBody.setSlip(1.0);
//    circuitBody.setFriction(dInfinity);

    // yoshi
// glm::vec3 yoshiPosition = { -30, 7.9, 40 };

// elmt::Model *yoshiModel = elmt::core::getAssetManager()->importAsset("assets/yoshi/", "yoshi_model.obj");
// elmt::Entity yoshi{ "Yoshi", &rootEntity, yoshiPosition };
// elmt::MeshRenderer meshRendererYoshi("Yoshi Mesh", &yoshi, yoshiModel);
// elmt::PhysicsComponent yoshiBody("Yoshi Rigidbody", &yoshi, BOX, yoshiModel);

//Set yoshiGeoms
//Note that mass doesn't matter in this instance since we're setting to kinematic, which disregards mass
//for (int i = 0; i < yoshiModel->meshes.size(); i++)
//{
    //yoshiBody.addGeom(yoshiProps, yoshiModel->meshes.at(i), 0);

//}

    // kart
    //Initial position and rotation to be at the starting line
    glm::vec3 kartPosition = { 6.32874, 8, 75.33403 };
    glm::mat4x4 kartRotation = { 0,  0, 1, 0,
                                  0,  1, 0, 0,
                                  -1,  0, 0, 0,
                                  0,  0, 0, 1
    };

    //    elmt::Model *brickModel = elmt::core::getAssetManager()->importAsset("assets/pebbles/", "wall.obj");
    //    elmt::Entity modelBrick{ "Brick", &rootEntity, glm::vec3 {0, 40, 0} };
    //    elmt::MeshRenderer meshRendererBrick("Brick Wall Mesh", &modelBrick, brickModel);
    //    elmt::PhysicsComponent pebblesBody("Pebbles Rigidbody", &modelBrick, elmt::PRIM_BOX, brickModel, true);
    //    pebblesBody.setKinematic(); //yoshi is stronger than gravity
    //
    //    elmt::Model* mirrorModel = elmt::core::getAssetManager()->importAsset("assets/mirror/", "mirror.obj", true);
    //    elmt::Entity modelMirror{ "Mirror", &rootEntity, glm::vec3 {7.5, 40.5, 8} };
    //    elmt::MeshRenderer meshRendererMirror("Mirror Mesh", &modelMirror, mirrorModel);
    //
    //    elmt::Model *hydrantModel = elmt::core::getAssetManager()->importAsset("assets/Hydrant/", "FireHydrantMesh.obj");
    //    elmt::Entity modelHydrant{ "Hydrant" , &rootEntity, glm::vec3 {0, 40, 10} };
    //    elmt::MeshRenderer meshRendererHydrant("Hydrant Mesh", &modelHydrant, hydrantModel);
    //    elmt::PhysicsComponent hydrantBody("Hydrant Rigidbody", &modelHydrant, elmt::PRIM_BOX, hydrantModel, true);
    //    hydrantBody.setKinematic(); //yoshi is stronger than gravity
    //
    //    elmt::PointLight pointLight1{ "test light point", &rootEntity, glm::vec3(0, 42, 11), glm::vec3(1, 0, 0), 2 };
    //    elmt::core::getRenderManager()->addPointLight(&pointLight1);
    //
    //    elmt::PointLight pointLight2{ "test light point", &rootEntity, glm::vec3(0, 42, 8), glm::vec3(0, 1, 0), 2 };
    //    elmt::core::getRenderManager()->addPointLight(&pointLight2);
    //
    //    elmt::PointLight pointLight3{ "test light point", &rootEntity, glm::vec3(2, 42, 9.5), glm::vec3(0, 0, 1), 2 };
    //    elmt::core::getRenderManager()->addPointLight(&pointLight3);
    //

    elmt::Model* kartModel = elmt::core::getAssetManager()->importAsset("assets/mario_kart/", "kart.obj");
    elmt::Entity modelYoshiKart{ "Kart", &rootEntity, kartPosition };
    elmt::MeshRenderer meshRendererKart("Kart Mesh", &modelYoshiKart, kartModel);
    elmt::PhysicsComponent kartBody("Kart Rigidbody", &modelYoshiKart, elmt::PRIM_TRIMESH, kartModel);

    kartBody.setPosition(glm::vec3(0, 100, 0));
    //    kartBody.setRotationMatrix(kartRotation);
    //    kartBody.setDynamic();
    //    kartBody.setMass(1);
    //    kartBody.setLinearDamping(0.25);
    //    kartBody.setAngularDamping(0.5);

        //Enable all geoms for the kart
    //    kartBody.enableCollisions();

        //Set collision parameters
    //    kartBody.setContactMode(dContactBounce | dContactSlip1 | dContactSoftCFM);
    //    kartBody.setFriction(dInfinity);
    //    kartBody.setBounciness(0.01);
    //    kartBody.setBouncinessVelocity(0.01);
    //    kartBody.setSlip(1);
    //    kartBody.setSoftCFM(0.1);

    elmt::Camera camera1{ "main camera", &rootEntity, glm::vec3(0, 0, 0), true };
    elmt::MouseLookComponent mouselook{ "main camera mouselook", &camera1 };

    // elmt::Model* brickModel = elmt::core::getAssetManager()->importAsset("assets/pebbles/", "wall.obj");
    // elmt::Entity modelBrick{ "Brick", &rootEntity, glm::vec3 {0, 40, 0} };
    // elmt::MeshRenderer meshRendererBrick("Brick Wall Mesh", &modelBrick, brickModel);

    // elmt::Model* brickModel = elmt::core::getAssetManager()->importAsset("assets/pebbles/", "wall.obj");
    // elmt::Entity modelBrick{ "Brick", &rootEntity, glm::vec3 {0, 40, 0} };
    // elmt::MeshRenderer meshRendererBrick("Brick Wall Mesh", &modelBrick, brickModel);

    // elmt::Model* hydrantModel = elmt::core::getAssetManager()->importAsset("assets/Hydrant/", "FireHydrantMesh.obj");
    // elmt::Entity modelHydrant{ "Brick", &rootEntity, glm::vec3 {0, 0, 0} };
    // elmt::MeshRenderer meshRendererHydrant("Hydrant Mesh", &modelHydrant, hydrantModel);

    // character
//	elmt::Model *characterModel = elmt::core::getAssetManager()->importAsset("assets/character/", "Breakdance.fbx");
//	elmt::Entity eve{ "Character", &rootEntity, glm::vec3(60, 0, 30) };
//    elmt::MeshRenderer meshRendererEve("Character Mesh", &eve, characterModel);

    // elmt::Model* hydrantModel = elmt::core::getAssetManager()->importAsset("assets/Hydrant/", "FireHydrantMesh.obj");
    // elmt::Entity modelHydrant{ "Brick", &rootEntity, glm::vec3 {0, 0, 0} };
    // elmt::MeshRenderer meshRendererHydrant("Hydrant Mesh", &modelHydrant, hydrantModel);

    // animations
    elmt::Animation idleAnimation = elmt::core::getAssetManager()->importAnimation("assets/animations/", "Standing Idle.dae");
    //    elmt::Animation ninjaAnimation = elmt::core::getAssetManager()->importAnimation("assets/animations/", "Ninja Idle.dae");
    //    elmt::Animation walkingAnimation = elmt::core::getAssetManager()->importAnimation("assets/animations/", "Walking.dae");
    //
        // sophie
    elmt::Entity sophie{ "Sophie", &rootEntity, glm::vec3(10, 41, 0) };
    sophie.scale = glm::vec3(0.02, 0.02, 0.02);

    elmt::Model* sophieModel = elmt::core::getAssetManager()->importAsset("assets/sophie/", "sophie.dae");
    elmt::MeshRenderer meshRendererSophie("Sophie Mesh", &sophie, sophieModel);

    elmt::Animator sophieAnimator("Sophie Animator", &sophie, &idleAnimation);

    // pilot
    /*elmt::Entity pilot{ "Pilot", &rootEntity, glm::vec3(5, 41, 0) };
    pilot.scale = glm::vec3(0.2, 0.2, 0.2);

    auto [pilotMesh, pilotSkeleton] = elmt::core::getAssetManager()->importMesh("assets/characters/", "Pilot_LP_Animated.fbx");

    elmt::CreateMaterialInfo pilotMaterialInfo = {};
    pilotMaterialInfo.diffuseTexture = "assets/characters/textures/Material.002_Base_Color.png";
    pilotMaterialInfo.normalTexture = "assets/characters/textures/Material.002_Normal_OpenGL.png";
    pilotMaterialInfo.metallicnessTexture = "assets/characters/textures/Material.002_Metallic.png";
    pilotMaterialInfo.roughnessTexture = "assets/characters/textures/Material.002_Roughness.png";
    std::vector<elmt::Material> pilotMaterial = { elmt::core::getAssetManager()->createMaterial(pilotMaterialInfo) };

    elmt::Model* pilotModel = new elmt::Model(pilotMesh, pilotSkeleton, pilotMaterial, elmt::core::getRenderManager());
    elmt::MeshRenderer meshRendererPilot("Pilot Mesh", &pilot, pilotModel);

    elmt::Animator pilotAnimator("Pilot Animator", &pilot, &idleAnimation);*/

    // element character
//    elmt::Entity elementGuy{ "Element Guy", &rootEntity, glm::vec3(5, 41, 0) };
//    //elementGuy.setRotationEuler(glm::vec3(0, 0, 0));
//    elementGuy.scale = glm::vec3(0.5);
//    elmt::Model* elementGuyModel = elmt::core::getAssetManager()->importAsset("assets/characters/", "element.dae");
//    //elmt::Model* elementGuyModelFbx = elmt::core::getAssetManager()->importAsset("assets/characters/", "element.fbx");
//    elmt::MeshRenderer elementGuyMeshRenderer("Element Guy Mesh", &elementGuy, elementGuyModel);
//
//    elmt::Animation elementAnimation = elmt::core::getAssetManager()->importAnimation("assets/characters/", "element.dae");
//    elmt::Animator elementGuyAnimator("Element Guy Animator", &elementGuy, &elementAnimation);

    // skybox
    char skyboxFiles[12][1024] = { "assets/skybox/right.jpg", "assets/skybox/left.jpg", "assets/skybox/top.jpg",
                                  "assets/skybox/bottom.jpg", "assets/skybox/front.jpg", "assets/skybox/back.jpg",
                                  "assets/skybox/right.jpg", "assets/skybox/left.jpg", "assets/skybox/top.jpg",
                                  "assets/skybox/bottom.jpg", "assets/skybox/front.jpg", "assets/skybox/back.jpg" };
    elmt::Skybox skybox(skyboxFiles, 1000.f);
    elmt::core::getRenderManager()->setSkybox(&skybox);

    // Setup mic
    elmt::MicrophoneComponent microphone{ "microphone", &camera1 };

    // Play test music
    std::string musicPath = "assets/sound/Somewhere In Silence (Bonus Track).mp3";
    elmt::Entity audioSource{ "Audio Source", &rootEntity, {0, 0, 30} };
    elmt::SoundInfo soundCreateInfo;
    soundCreateInfo.attenuation = elmt::ATT_INVERSE;
    soundCreateInfo.rolloff = 0.25;
    soundCreateInfo.dopplerFactor = 20.0;
    soundCreateInfo.isLooping = true;
    elmt::SoundComponent musicPlayerSpatial{ "musicPlayer", &audioSource, musicPath.c_str(), soundCreateInfo };

    // Play test music
    testSoundPointer = &musicPlayerSpatial;

    // BDRF gen
    //elmt::Material brdfMat;
    //test
    //brdfMat = elmt::core::getProcManager()->generateBRDFMaterial("testbdrf", ".png", 128, 128, elmt::ProcManager::BRDFFunctionTest, 42, nullptr);
    //wave
    //elmt::waveMaterialInfo brdfInfo = elmt::waveMaterialInfo{}; brdfInfo.bb = 255; brdfInfo.tb = 0; brdfInfo.br = 0; brdfInfo.tr = 255;
    //white noise
    //elmt::whiteNoiseMaterialInfo brdfInfo = elmt::whiteNoiseMaterialInfo{};  brdfInfo.base.r = 128; brdfInfo.base.g = 32; brdfInfo.base.b = 196; brdfInfo.variance = 64;
    //perlin noise
    //worley noise
    //elmt::worleyNoiseMaterialInfo brdfInfo; brdfInfo.base.r = 16; brdfInfo.base.g = 32; brdfInfo.base.b = 128;
    //brdfMat = elmt::ProcManager::generateBRDFMaterial("testbdrf_gradient", ".png", 128, 128, elmt::ProcManager::gradientBRDFFunction, 42, nullptr);

    // Heightmaps
    /*
    elmt::Material heightMat;
    //load
    int hw, hh, hc;
    auto heightMapTexture = stbi_load("heightmap_perlin.png", &hw, &hh, &hc, 3);
    heightMat.textureWidth = hw;
    heightMat.textureHeight = hh;
    heightMat.diffuseTexture = heightMapTexture;

    //worley
    //elmt::worleyInfo hmInfo = elmt::worleyInfo{ 10,10,true, 0.0,-2.7,0.2, -0.7,1.8, 0.1,25.0, true };
    //heightMat = elmt::ProcManager::generateHeightmap("heightmap.png", 64, 64, elmt::ProcManager::worleyHeightFunction, 42, &hmInfo);

=======
    // BDRF gen
    //elmt::Material brdfMat;
    //test
    //brdfMat = elmt::core::getProcManager()->generateBRDFMaterial("testbdrf", ".png", 128, 128, elmt::ProcManager::BRDFFunctionTest, 42, nullptr);
    //wave
    //elmt::waveMaterialInfo brdfInfo = elmt::waveMaterialInfo{}; brdfInfo.bb = 255; brdfInfo.tb = 0; brdfInfo.br = 0; brdfInfo.tr = 255;
    //white noise
    //elmt::whiteNoiseMaterialInfo brdfInfo = elmt::whiteNoiseMaterialInfo{};  brdfInfo.base.r = 128; brdfInfo.base.g = 32; brdfInfo.base.b = 196; brdfInfo.variance = 64;
    //perlin noise
    //worley noise
    //elmt::worleyNoiseMaterialInfo brdfInfo; brdfInfo.base.r = 16; brdfInfo.base.g = 32; brdfInfo.base.b = 128;
    //brdfMat = elmt::ProcManager::generateBRDFMaterial("testbdrf_gradient", ".png", 128, 128, elmt::ProcManager::gradientBRDFFunction, 42, nullptr);

    // Heightmaps
    /*
    elmt::Material heightMat;
    //load
    int hw, hh, hc;
    auto heightMapTexture = stbi_load("heightmap_perlin.png", &hw, &hh, &hc, 3);
    heightMat.textureWidth = hw;
    heightMat.textureHeight = hh;
    heightMat.diffuseTexture = heightMapTexture;

    //worley
    //elmt::worleyInfo hmInfo = elmt::worleyInfo{ 10,10,true, 0.0,-2.7,0.2, -0.7,1.8, 0.1,25.0, true };
    //heightMat = elmt::ProcManager::generateHeightmap("heightmap.png", 64, 64, elmt::ProcManager::worleyHeightFunction, 42, &hmInfo);

>>>>>>> main
    //perlin
    //heightMat = elmt::ProcManager::generateHeightmap("heightmap_perlin.png", 128, 128, elmt::ProcManager::perlinHeightFunction, 42, nullptr);
    elmt::ProcManager::MeshCreateInfo info{
        glm::vec3(0.0, -10.0, 0.0),
        80.0, 80.0, // dimensions
        100, 100, // cells
        0.0, 30.0, // height range
        0, // material index
        elmt::ProcManager::MeshCreateFilter::FILT_BLEND_LINEAR // filtering
    };
    //draw terrain
    elmt::Mesh terrainMesh = elmt::ProcManager::createMesh(heightMat, info);
    auto terrainMeshes = std::vector<elmt::Mesh>{ terrainMesh };
    auto terrainMaterials = std::vector<elmt::Material>{bdrfMat };// { heightMat }; circuitModel->materials[1]
    //TODO remove rendermanager param
    elmt::Model terrainModel{ terrainMeshes, terrainMaterials, elmt::core::getRenderManager()};


    elmt::Entity terrain{ "Terrain", &rootEntity, {0, 0, 0} };
    elmt::MeshRenderer meshRendererTerrain("Terrain Mesh", &terrain, &terrainModel);
    */

    // Blend
    /*
    elmt::BlendInfo mixInfo{ elmt::BLENDMODE::MODE_MAX, 1.0, 1.0, true };
    elmt::Material mixMat = elmt::ProcManager::mixMaterials(circuitModel->materials[2], circuitModel->materials[3], "mixMaterial", ".png", 128, 128, mixInfo);
    */


    // Random spawn
    elmt::ProcManager::RandomSpawnInfo spawnBounds{
            kartPosition + glm::vec3(0, 5, 0),
            4.0, 1.0, 4.0,
            elmt::ProcManager::RandomSpawnType::SPWN_BOX
    };
    //	std::vector<elmt::Entity*> spawnList{ &modelYoshiKart };
    //	std::vector<elmt::Entity*> spawnedEntities{};// = elmt::ProcManager::spawnEntities(spawnList, 3, 4, 1.0, spawnBounds);
    //	for (auto spawnedEnt : spawnedEntities) {
    //		auto bodyComp = (elmt::PhysicsComponent*)spawnedEnt->getComponentOfType("PhysicsComponent");
    //		bodyComp->setDynamic();
    //	}

    // UI
    elmt::UIManager* uiMgr = elmt::core::getRenderManager()->getUIManager();
    elmt::UIEditor* uiEditor = new elmt::UIEditor(uiMgr);
    uiEditor->setDirectionalLight(&directionalLight1);
    uiEditor->setTestSoundPointer(testSoundPointer);
    uiMgr->setUIEditor(uiEditor);




    // Input Manager
    elmt::InputManager* inpMan = elmt::core::getInputManager();
    auto scM = glfwGetKeyScancode(GLFW_KEY_M);
    auto scN = glfwGetKeyScancode(GLFW_KEY_N);
    auto scV = glfwGetKeyScancode(GLFW_KEY_V);
    auto scB = glfwGetKeyScancode(GLFW_KEY_B);
    auto mbL = GLFW_MOUSE_BUTTON_LEFT;
    auto mbR = GLFW_MOUSE_BUTTON_RIGHT;

    elmt::KeyCombo c1{ { scN, scM },{mbL, mbR} };
    auto testInputAction = elmt::InputAction({ c1 });

    bool running = true;

    int channel = 0;

    float totalTime = 0.f;
    size_t numFrames = 0;
    float renderTime = 0.f;

    std::chrono::milliseconds totalStartTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    );
    bool updateBoneId = false;


    while (running) {
        double frameTime = elmt::core::startFrame();
        mouselook.freezeMouse = freezeCamera;

        totalTime += frameTime;
        ++numFrames;


        //std::cout << inpMan->mouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT) << " " << inpMan->mouseButtonJustPressed(GLFW_MOUSE_BUTTON_RIGHT) << " " << inpMan->mouseButtonJustReleased(GLFW_MOUSE_BUTTON_RIGHT) << std::endl;
        //std::cout << testInputAction.isPressed() << " " << testInputAction.isJustPressed() << " " << testInputAction.isJustReleased() << std::endl;
        //std::cout << inpMan->getJustPressedKeys().size() << " " << inpMan->getJustPressedMouseButtons().size() << std::endl;

        if (inpMan->mouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
            elmt::core::getRenderManager()->mouseX = inpMan->getMouseX();
            elmt::core::getRenderManager()->mouseY = inpMan->getMouseY();
            elmt::core::getRenderManager()->debugClick = true;
        }

        if (inpMan->shouldCloseWindow()) {
            running = false;
        }

        if (inpMan->keyPressed(glfwGetKeyScancode(GLFW_KEY_H))) {
            elmt::core::getAudioManager()->setGlobalVolume(elmt::core::getAudioManager()->getGlobalVolume() + 0.4);
        }
        if (inpMan->keyPressed(glfwGetKeyScancode(GLFW_KEY_G))) {
            elmt::core::getAudioManager()->setGlobalVolume(elmt::core::getAudioManager()->getGlobalVolume() - 0.4);
        }


        if (inpMan->keyJustPressed(glfwGetKeyScancode(GLFW_KEY_C))) {
            if (testSoundPointer->isPlaying()) {
                testSoundPointer->Stop();
            }
            else {
                testSoundPointer->Play();
            }

        }

        //elmt::core::getCollisionManager()->getIntersections(&modelYoshiKart, elmt::IntersectionTestInfo());


        if (inpMan->keyJustPressed(glfwGetKeyScancode(GLFW_KEY_V))) {
            // Play test sound
            std::string soundPath = "assets/sound/Sound.wav";
            elmt::SoundInfo info;
            info.pan = -1.0;
            if (testSoundPointer2) {
                elmt::core::getAudioManager()->stopSound(testSoundPointer2);
                testSoundPointer2 = nullptr;
            }
            else {
                testSoundPointer2 = elmt::core::getAudioManager()->playSound(soundPath.c_str(), info);
            }

        }
        if (inpMan->keyJustPressed(glfwGetKeyScancode(GLFW_KEY_O))) {
            elmt::core::getRenderManager()->parallaxEnabled = !elmt::core::getRenderManager()->parallaxEnabled;
            printf("Parallax mapping set to %s\n", elmt::core::getRenderManager()->parallaxEnabled ? "enabled" : "disabled");
        }
        if (inpMan->keyJustPressed(glfwGetKeyScancode(GLFW_KEY_P))) {
            freezeCamera = !freezeCamera;
        }

        if (inpMan->keyPressed(glfwGetKeyScancode(GLFW_KEY_ESCAPE))) {
            running = false;
        }

        //check if change render mode
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

        //        else if (inpMan->keyJustPressed(glfwGetKeyScancode(GLFW_KEY_I))) {
        //            sophieAnimator.SwitchAnimation(&walkingAnimation, 0.5f);
        //        }
        //
        //        else if (inpMan->keyJustPressed(glfwGetKeyScancode(GLFW_KEY_K))) {
        //            sophieAnimator.SwitchAnimation(&idleAnimation, 0.5f);
        //        }
        //
        //        else if (inpMan->keyJustPressed(glfwGetKeyScancode(GLFW_KEY_O))) {
        //            sophieAnimator.SwitchAnimation(&ninjaAnimation, 0.5f);
        //        }
        //
        //        for (int i = 0; i < yoshiModel->materials.size(); i++) {
        //            yoshiModel->materials[i].diffuse[channel] += 0.01;
        //            if (yoshiModel->materials[i].diffuse[channel] > 1) {
        //                yoshiModel->materials[i].diffuse[channel] = 0;
        //                channel = (channel + 1) % 3;
        //            }
        //        }

                //Update entity position
        updateKart(kartBody);

        //Get the kart's linear velocity outside the loop - reduces total number of function evaluations
        //glm::vec3 kartLinVel = kartBody.getLinearVelocity();

        //Add gravity to the linear velocity
        //glm::vec3 kartMove = kartLinVel + gravityPerStep;
        //kartBody.applyForce(kartMove.x, kartMove.y, kartMove.z);

        //Update the world (assuming 60fps)
        // elmt::core::getCollisionManager()->handleCollisions(&world);
        // world.takeStep(step);
        // world.clearJoints();

        //Update entity position
//        updateKart(kartBody);

        //uiMgr->updateWindowSize();
        //uiEditor.Update();

        elmt::core::Update();
        elmt::core::Render();
        elmt::core::endFrame();

    }


    std::chrono::milliseconds totalEndTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    );

    totalTime = totalEndTime.count() - totalStartTime.count();


    printf("Average fps was %f\n", (numFrames * 1000.f) / (float)totalTime);


    // Cleanup
    elmt::core::Teardown();


    return EXIT_SUCCESS;
}
