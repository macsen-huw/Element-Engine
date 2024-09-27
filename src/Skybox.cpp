
#include "Skybox.hpp"

#define STB_IMAGE_STATIC
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif


using namespace elmt;

Skybox::Skybox(char files[12][1024], float scale) {

    this->scale = scale;

    for (int i = 0; i < 6; i++){
        int channels;
        irradianceData[i] = stbi_loadf(files[i], &irradianceWidth, &irradianceHeight, &channels, 4);
        imageData[i] = stbi_load(files[i + 6], &width, &height, &channels, 4);
    }

}

Skybox::~Skybox() {
    for (int i = 0; i < 6; i++) {
        stbi_image_free(irradianceData[i]);
        stbi_image_free(imageData[i]);
    }
}





