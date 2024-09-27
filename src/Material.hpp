#pragma once
#include <string>
#include "RenderStructs.hpp"
#include "../external/glm-0.9.7.1/glm/glm.hpp"

namespace elmt {
    struct Texture {
        std::string textureType;
        std::string texturePath;

        uint8_t* textureData = nullptr;
        size_t textureWidth = 0;
        size_t textureHeight = 0;

        TextureInfo textureInfo{};
    };

    class Material {
        public:

            std::string materialName;
            glm::vec3 diffuse = glm::vec3(1, 1, 1);
            glm::vec3 specular = glm::vec3(0, 0, 0);
            glm::vec3 emissive = glm::vec3(0, 0, 0);
            glm::vec3 ambient = glm::vec3(1, 1, 1);

            Texture diffuseTexture;
            Texture normalTexture;
            Texture roughnessTexture;
            Texture metallicnessTexture;
            Texture parallaxTexture;

            /*
             * a quick note on ambient occlusion textures:
             * an ambient occlusion texture only models occlusion caused by the model itself - it doesn't know about nearby objects
             * so when sampling the ao map, we need to blend it with the ambient occlusion from ssao
             * however, there is 1 exception - when we have a flat plane (eg, parallax map) where the ssao will be 0, but
             * we want to think the model does cause self occlusion - we can just add the 2
             */
            Texture ambientOcclusionTexture;

            float shininess = 1;

            bool translucent = false; //set to true if its texture contains any translucent (not including fully transparent!) texels
            bool opaque = true; //set to true if every pixel in its texture is 100% opaque (ie no transparent texels)
    };
}

