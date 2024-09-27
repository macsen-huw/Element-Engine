#pragma once

#include "miniaudio.h"

#include "AudioManager.hpp"
#include "Component.hpp"
#include "Entity.hpp"



namespace elmt {


    /*
    Component for playing 3D spatial audio
    Attach it to the relevant entity you want to act as an "emitter"
    */
    class SoundComponent :
        public Component
    {

        // Properties
    private:
        // Filename of loaded sound
        std::string fileName;
        // Actual playing sound
        Sound sound;
        // Whether this component is currently playing it's sound
        bool playing;
        // Whether a valid sound has been loaded for this component
        bool hasLoadedSound;

        SoundInfo info;

        // Methods
    public:
        SoundComponent(); // only used for serialisation
        SoundComponent(const char* name, Entity* entity, const char* fileName, SoundInfo info);
        SoundComponent(const char* name, Entity* entity, const char* fileName);

        std::string getFilename() { return fileName; };
        bool isPlaying() { return playing; };
        bool hasSound() { return hasLoadedSound; }

        bool Update();
        int updateTransform();

        int setSound(const char* fileName);
        int Play();
        int Stop();

        SoundInfo getInfo() { return info; }
        void setInfo(SoundInfo newInfo);

        virtual ~SoundComponent();

        virtual void clone(Component*& clonePointer, Entity* entityToAttach);
    };

}

