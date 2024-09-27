#include "element.hpp"

#include "Player.hpp"

#include "globals.hpp"

#include "glm.hpp"
#include "Trigger.hpp"

using namespace globals;

elmt::Model* loadPlayerModel(){
    return g::assMan->importAsset(g::BASE_PATH + "player" + g::fileSep + "Sophie" + g::fileSep, "Sophie.dae");
    //return g::assMan->importAsset(g::BASE_PATH + "player" + g::fileSep, "playerModel_LP2.obj");
}

Player::Player() : GameObject("Player", { 5.0, 15.0, 0.0 }, loadPlayerModel(), false) 
{

    auto monsterAnim = elmt::core::getAssetManager()->importAnimation("assets/characters/Monster/", "Monster.dae");


    //scale = glm::vec3(0.02, 0.02, 0.02);
    




    
    /*delete meshComp;
    auto monsterModel = elmt::core::getAssetManager()->importAsset("assets/characters/Monster/", "Monster.dae");
    elmt::MeshRenderer monsterMeshRenderer("Monster Mesh", &monster, monsterModel);*/

    meshEnt = new elmt::Entity("Player Render Entity", this, pos);
    meshEnt->scale = glm::vec3(0.02, 0.02, 0.02);
    auto meshEntComp = new elmt::MeshRenderer((this->name + " Renderer").c_str(), meshEnt, meshComp->model);
    //meshEnt->rotateWithParent = true;
    delete meshComp;


    monsterAnim1 = new elmt::Animation;
    *monsterAnim1 = elmt::core::getAssetManager()->importAnimation("assets/characters/Monster/", "Monster.dae");

    monsterAnimJump = new elmt::Animation;
    *monsterAnimJump = elmt::core::getAssetManager()->importAnimation("assets/characters/Monster/", "Jump.dae");

    monsterAnimWalk = new elmt::Animation;
    *monsterAnimWalk = elmt::core::getAssetManager()->importAnimation("assets/characters/Monster/", "Run.dae");
    monsterAnimator = new elmt::Animator("Monster Animator", meshEnt, monsterAnimWalk);
    monsterAnimator->SwitchAnimation(monsterAnimWalk, 0.1);


    colModel = elmt::core::getAssetManager()->importAsset(g::BASE_PATH, "player" + g::fileSep + "pm_monster_collision.dae");

    hasPhysics = true;
    physComp = new elmt::PhysicsComponent((name + " Physics").c_str(), this, elmt::PhysicsPrimitives::PRIM_TRIMESH, colModel);



    g::groupMan->addEntityToGroup("Player", this);

    physComp->setKinematic();
    physComp->setMass(1000.0);
    physComp->setOverrideEntityPos(true);
    physComp->customCollision = true;
    


    cam = new elmt::Camera("Camera", this, pos + glm::vec3(-1.0, 1.0, 0.0), true);
    cam->rotateWithParent = false;
    cam->scale = glm::vec3(0.02, 0.02, 0.02);
    mouseLook = new elmt::MouseLookComponent("Mouselook", cam);

    auto camPhysComp = new elmt::PhysicsComponent("Cam Physics", cam, elmt::PRIM_TRIMESH, g::getModel("cube"));
    camPhysComp->setKinematic();
    camPhysComp->setOverrideEntityPos(false);
    

    typeName = "Player";
}

Player::~Player()
{
    delete colModel;
}

bool Player::Update() {
    static bool flag = false;

    bool res = GameObject::Update();
    if (!res) return false;

    float turnSpeed = 2.0;
    float dt = elmt::core::getDeltaTime();

    auto& a = g::turnLeft;

    if (g::turnLeft.isPressed()) {
        rotateHorizontal(-1.0 * turnSpeed * dt);
        physComp->setRotationMatrix(rotation);
    }

    if (g::turnRight.isPressed()) {
        rotateHorizontal(1.0 * turnSpeed * dt);
        physComp->setRotationMatrix(rotation);
    }

    float moveSpeed = 5.f;
    glm::vec3 moveVec = getForward() * (moveSpeed);
    if (g::moveForward.isPressed()) {
        //rotateVertical(1.0 * turnSpeed * dt);
        //physCompGameObject->setRotationMatrix(rotation);
        moveVec *= 1.0;
    } else if (g::moveBackward.isPressed()) {
        
        //rotateVertical(-1.0 * turnSpeed * dt);
        //physCompGameObject->setRotationMatrix(rotation);
        moveVec *= -1.0;
    }
    else {
        //moveVec.x = velocity.x * 0.9;
        //moveVec.z = velocity.z * 0.99;
       
    }




    float vertChange = -1.0;
    /*if (g::moveDown.isPressed()){
        vertChange = -moveSpeed;
    }
    else if (g::moveUp.isPressed()){
        vertChange = moveSpeed;
    }*/

    


    velocity.x = moveVec.x;
    velocity.z = moveVec.z;
   
    glm::vec2 hor = glm::vec2(velocity.x, velocity.z);
    if (glm::length(hor) > 10.0) {
        hor = glm::normalize(hor) * 10.0f;
        velocity.x = hor.x;
        velocity.z = hor.y;
    }


    velocity.y = glm::clamp(velocity.y + vertChange, -10.f, 10.f);

    if (g::inpMan->keyJustPressed(glfwGetKeyScancode(GLFW_KEY_SPACE))) {
        velocity.y = 50;
        monsterAnimator->SwitchAnimation(monsterAnimJump, 0.0);

    }


    /*
    if (pos.y + (velocity.y * elmt::core::getDeltaTime()) > floorHeight) {
        
    }
    else {
        float diff = floorHeight - pos.y;
        velocity.y /= elmt::core::getDeltaTime();

    }
    if (velocity.y > 100.0) {
        velocity.y = 0.0;
    }*/
    


    assert(!isnan(velocity.x));
    assert(!isnan(velocity.y));
    assert(!isnan(velocity.z));

    if (glm::length(moveVec)) {
//        printf("Velocity = %f, %f, %f\n", velocity.x, velocity.y, velocity.z);
        physComp->setLinearVelocity(velocity);
        
//        physCompGameObject->setAngularVelocity(glm::vec3(0, 0, 0));
//        physCompGameObject->setAngularDamping(100);

    }

    

    //physComp->getPosition();
    //this->pos += (velocity * (float)elmt::core::getDeltaTime() );
    if (this->pos.y <= floorHeight) {
        //pos.y = floorHeight;
        auto resetPos = physComp->getPosition();
       /* resetPos.y = floorHeight;
        physComp->setPosition(resetPos);
        if (velocity.y < 0) {
            velocity.y = 0.0;
        }*/
        
        if (monsterAnimator->currentAnimation == monsterAnimJump) {
            monsterAnimator->SwitchAnimation(monsterAnimWalk, 0.0);
        }
    }
    

    //std::cout << glm::length(moveVec) << " " << physComp->getLinearVelocity().x << ", " << physComp->getLinearVelocity().y << ", " << physComp->getLinearVelocity().z << std::endl;
    //physComp->applyForce(moveVec.x, moveVec.y, moveVec.z);

    if (g::state == STARTED) {
        timeLeft -= elmt::core::getDeltaTime();
        if (timeLeft <= 0.0) {
            killPlaneCallback(this, nullptr);
        }
    }

    meshEnt->rotation = rotation;

    return true;

}

void Player::changeScore(unsigned int amount)
{
    score += amount;
    std::cout << "Changed Score (" << amount << ") : " << score << std::endl;
}

void Player::Reset()
{
    GameObject::Reset();
    score = 0;
    timeLeft = 0;
    g::state = NOTSTARTED;
}

void Player::killPlaneCallback(elmt::Entity* e, Trigger* t)
{

    g::audMan->playSound( (g::SOUND_PATH + "die.wav").c_str() );
    g::state = GAMEOVER;
    g::player->Reset();
}


void Player::winCallback(elmt::Entity* e, Trigger* t)
{
    g::audMan->playSound((g::SOUND_PATH + "win.wav").c_str());
    g::player->Reset();
    g::state = WIN;

}

void Player::startCallback(elmt::Entity* e, Trigger* t) {
    if (g::state == NOTSTARTED) {
        g::audMan->playSound((g::SOUND_PATH + "timerstart.wav").c_str());
        g::state = STARTED;
    }
};