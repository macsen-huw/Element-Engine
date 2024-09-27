#pragma once

#include "GameObject.hpp"
#include "Trigger.hpp"

class Player :
        public GameObject
{
public:
    Player();
    ~Player();

    unsigned int score = 0;
    unsigned int timeLeft = 0;

    virtual bool Update();

    void changeScore(unsigned int amount);

    virtual void Reset();

    // For debugging, camera hitbox
    elmt::Camera* cam;
    elmt::MouseLookComponent* mouseLook;

    static void killPlaneCallback(elmt::Entity* e, Trigger* t);
    static void winCallback(elmt::Entity* e, Trigger* t);
    static void startCallback(elmt::Entity* e, Trigger* t);

    elmt::Animator *monsterAnimator;
    elmt::Animation *monsterAnim1;
    elmt::Animation* monsterAnimWalk;
    elmt::Animation* monsterAnimJump;

    elmt::Entity* meshEnt;

    elmt::Model* colModel;
    float floorHeight = 0.0;
};

