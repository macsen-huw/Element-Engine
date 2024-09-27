#pragma once
#include <cstdint>
#include "glm.hpp"
#include <vector>
#include <GL/glew.h>

/*
 * this file contains structs used in rendering that are needed by multiple files
 */

namespace elmt {

    struct TextureInfo{
        size_t level = 0;
        GLuint bucketID = 0; //the array texture that the 2d texture is in
    };
}
