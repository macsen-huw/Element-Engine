#pragma once

#include <glm.hpp>

#include "GameObject.hpp"
#include "globals.hpp"
#include "element.hpp"

// Randomly generated platform
class Platform :
    public GameObject
{

private:

    inline static std::map < std::string, elmt::Material> texCache{};

public:

    inline static std::vector<elmt::Model*> models{};
    inline static std::string data_path = g::BASE_PATH + "platforms" + g::fileSep;
    inline static std::string data_gen_path = data_path + "gen" + g::fileSep;

    
    Platform() {};
    Platform(glm::vec3 pos);
    Platform(glm::vec3 pos, elmt::Model* model);

    // Generate a model to be used by platforms
    static elmt::Model* generateModel(std::string maskBaseName, std::string texName, unsigned int seed);
    // Generate a unique name for a platform
    static std::string generateName();
    // Choose a random model for the platofmr
    static elmt::Model* chooseModel();
    // Load a platform texture, or re-use one that;s already loaded
    static elmt::Material& getPlatformTex(std::string texName);

};

