#pragma once

#include "Component.hpp"
#include "Entity.hpp"

namespace elmt {

    /*
    Component for manipulating the camera using the mouse
    */
    class MouseLookComponent :
        public Component
    {

        // Properties
    private:
        

    public:
        bool freezeMouse = false;
        bool enableFreeCam = true;
        float speed = 0.5f;
        float mouseSpeed = 0.005f;

        // Methods
    public:
        MouseLookComponent();//only used for serialisation

        virtual bool Update();

        MouseLookComponent(const char* name, Entity* entity) : MouseLookComponent(name, entity, 20.0f, 0.005f, false) {};
        MouseLookComponent(const char* name, Entity* entity, float speed, float mouseSpeed, bool freezeMouse);

        virtual void clone(Component*& clonePointer, Entity* entityToAttach);
    };

}

