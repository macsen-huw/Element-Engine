#pragma once
#include <cstdint>
#include "../external/glm-0.9.7.1/glm/glm.hpp"


namespace elmt{

    class Skybox{

        public:
            Skybox(char files[12][1024], float scale = 1.f);
            float *irradianceData[6];
            uint8_t *imageData[6];

            int width, height;
            int irradianceWidth, irradianceHeight;
            float scale;

            ~Skybox();
    };
}