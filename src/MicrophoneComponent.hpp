#pragma once

#include "../external/glm-0.9.7.1/glm/glm.hpp"
#include "../external/glm-0.9.7.1/glm/gtc/matrix_transform.hpp"
#include "../external/glm-0.9.7.1/glm/gtx/euler_angles.hpp"


#include "Component.hpp"
#include "Entity.hpp"

namespace elmt {

    // Info used for setting up or updating the MicrophoneComponent
    struct MicrophoneInfo {
        /*
        When sound is between inner and outer cones, will be attenuated between 1.0 and outerGain
        */
        float outerAngle = glm::pi<float>();
        float innerAngle = glm::pi<float>();
        float outerGain = 1.0;
    };

    /*
    Component for listening 3D spatial audio
    Attach it to the relevant entity you want sounds to be played relative to
    In most cases, this will be the camera
    */
    class MicrophoneComponent :
        public Component
    {

        // Properties
    private:
        // Listener index, used by miniaudio. This will always be 0 unless multiple microphone support is implement
        unsigned int listenerIndex = 0;

        MicrophoneInfo info;

    public:


        // Methods
    public:
        MicrophoneComponent();//only used for serialisation

        bool Update();
        int updateTransform();

        // Update the info used for this MicrophoneComponent
        void updateInfo(MicrophoneInfo newInfo);
        
        MicrophoneComponent(const char* name, Entity* entity, MicrophoneInfo info);
        MicrophoneComponent(const char* name, Entity* entity) : MicrophoneComponent(name, entity, MicrophoneInfo()) {}

        virtual void clone(Component*& clonePointer, Entity* entityToAttach);
    };

}

