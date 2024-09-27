#include "TextureManager.hpp"
#include "Logger.hpp"
#include <string>

using namespace elmt;

TextureManager::TextureManager(size_t maxBucketSize) {

    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureBuckets);
    glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &maxTextureBucketSize);
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);

    //clamp the maximum array size to some value, we could just use the maximum from opengl, but this could be something huge
    //-1 because we're using 1 texture to render the depth buffer
    maxTextureBucketSize = std::min(maxTextureBucketSize - 1, (GLint) maxBucketSize);

}


void TextureManager::createShadowBucket(size_t width, size_t height, size_t depth) {


    if (shadowBucket != nullptr){
        Logger::Print("Error attempting to create a shadow bucket, but one already exists\n", LOGCAT::CAT_RENDERING, LOGSEV::SEV_ERROR);
    }


    TextureBucket newBucket;

    newBucket.numTextures = 0;
    newBucket.width = width;
    newBucket.height = height;
    newBucket.reserved = 1;


    GLuint textureID;
    glGenTextures(1, &textureID);


    glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);

    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT, width, height, depth, 0,  GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);


    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    newBucket.textureArrayID = textureID;


    textureBuckets.push_back(newBucket);
    shadowBucket = &textureBuckets[textureBuckets.size() - 1];

    for (size_t i = 0; i < depth; i++){
        loadShadowTexture();
    }
}


void TextureManager::loadShadowTexture() {

    if (shadowBucket == nullptr){
        Logger::Print("Error - trying to load a shadow without having created a shadow bucket\n", LOGCAT::CAT_RENDERING, LOGSEV::SEV_ERROR);
        exit(EXIT_FAILURE);
    }

    glBindTexture(GL_TEXTURE_2D_ARRAY, shadowBucket->textureArrayID);

    //the data passed to glTexSubImage3D cannot be null, so we pass an array of 0s
    //this definitely needs investigating in the future, and might not be needed at all
    std::vector<float> dataVec(shadowBucket->width * shadowBucket->height, 0.f);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, shadowBucket->numTextures++, shadowBucket->width, shadowBucket->height, 1,  GL_DEPTH_COMPONENT, GL_FLOAT, dataVec.data());

    //texture parameters the same as learnOpenGL uses, needs investigating in the future if this is the best way
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE,  GL_COMPARE_REF_TO_TEXTURE);

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}


GLuint TextureManager::createDepthBufferTexture(size_t width, size_t height) {
    if (depthBufferTexture != 0){
        glDeleteTextures(1, &depthBufferTexture);
    }

    glGenTextures(1, &depthBufferTexture);
    glBindTexture(GL_TEXTURE_2D, depthBufferTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0,  GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    //texture parameters the same as learnOpenGL uses, needs investigating in the future if this is the best way
//    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    return depthBufferTexture;
}


GLuint TextureManager::loadSkyBoxTexture(Skybox *skybox) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    for (int i = 0; i < 6; i++){
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, skybox->width, skybox->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, skybox->imageData[i]);
    }

//    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.f);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return textureID;
}


GLuint TextureManager::loadSkyBoxIrradianceData(Skybox *skybox) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    for (int i = 0; i < 6; i++){
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, skybox->irradianceWidth, skybox->irradianceHeight, 0, GL_RGBA, GL_FLOAT, skybox->irradianceData[i]);
    }

//    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.f);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return textureID;
}



//void TextureManager::clearShadowBucket() {
//    shadowBucket->numTextures = 0;
//}

GLuint TextureManager::getShadowTextureID() {
    return shadowBucket->textureArrayID;
}



//void TextureManager::deleteTexture(TextureInfo info) {
//    for (size_t i = 0; i < textureBuckets.size(); i++){
//        if (textureBuckets[i].textureArrayID == info.bucketID){
//            textureBuckets[i].freePositions[info.level] = true;
//            textureBuckets[i].numTextures--;
//            if (textureBuckets[i].numTextures == 0 && textureBuckets[i].reserved == 0){
//                textureBuckets.erase(textureBuckets.begin() + i);
//            }
//            return;
//        }
//    }
//}


TextureInfo TextureManager::loadTexture(Texture &texture, bool mipmap) {
    size_t texWidth = 1, texHeight = 1;
    uint8_t *data = nullptr;

    if (texture.textureData != nullptr){
        data = texture.textureData;
        texWidth = texture.textureWidth;
        texHeight = texture.textureHeight;
    }

    size_t bucketIndex = findBucket(texWidth, texHeight, mipmap);
    TextureInfo info{};
    info.bucketID = bucketIndex;
    info.level = textureBuckets[bucketIndex].numTextures++;
    textureBuckets[bucketIndex].textures.push_back(data);

    return info;
}


/*
 * adds a texture to bucket i
 */
void TextureManager::addTextureToBucket(TextureBucket &bucket, size_t layer) {

    uint8_t dummy[4] = {0, 255, 0, 255};
    uint8_t *texture;
    if (bucket.textures[layer] == nullptr){
        assert(bucket.width == 1);
        assert(bucket.height == 1);
    }
    assert(bucket.width && bucket.height);

    texture = (bucket.textures[layer] == nullptr) ? dummy : bucket.textures[layer];

    glBindTexture(GL_TEXTURE_2D_ARRAY, bucket.textureArrayID);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, layer, bucket.width, bucket.height, 1, GL_RGBA, GL_UNSIGNED_BYTE, texture);
    

    if (bucket.mipmap) glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    if (bucket.mipmap) glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.f);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}


/*
 * creates a new texture bucket and adds to the end of the textureBuckets vector
 */

void TextureManager::createBucket(size_t width, size_t height, size_t depth, bool mipmap) {

    if (width > maxTextureSize || height > maxTextureSize || depth > maxTextureSize){
        Logger::Print("Error - trying to load a texture bigger than the maximum texture size supported by the hardware\n"
                      "Maximum size is" + std::to_string(maxTextureSize) + "x" + std::to_string(maxTextureSize) +
                      "but the texture is " + std::to_string(width) + "x" + std::to_string(height) + "\n",
                      LOGCAT::CAT_RENDERING, LOGSEV::SEV_ERROR);
        return;
    }


    TextureBucket newBucket{};

    newBucket.width = width;
    newBucket.height = height;
    newBucket.mipmap = mipmap;

    textureBuckets.push_back(newBucket);
}

/*
 * searches through the textureBuckets array for an array of the right size with room for another texture
 * if it can't find one it will try and make a new bucket
 */

size_t TextureManager::findBucket(size_t width, size_t height, bool mipmap) {

    //linear search isn't a problem because the number of buckets is small
    for (size_t i = 0; i < textureBuckets.size(); i++){
        TextureBucket bucket = textureBuckets[i];
        if (bucket.width == width && bucket.height == height && bucket.numTextures < maxTextureBucketSize &&
            bucket.reserved == 0 && bucket.mipmap == mipmap){
            return i;
        }
    }

    createBucket(width, height, maxTextureBucketSize, mipmap);
    return textureBuckets.size() - 1;
}

/*
 * function creates a texture which we can render to
 * not used at the moment, but might be useful in the future
 */
void TextureManager::loadRenderTexture(GLuint *returnTextureID, GLuint *returnRenderBufferID, GLuint frameBufferID, size_t width, size_t height) {

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);



    GLuint renderBufferID;
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID);
    glGenRenderbuffers(1, &renderBufferID);
    glBindRenderbuffer(GL_RENDERBUFFER, renderBufferID);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderBufferID);

    *returnTextureID = textureID;
    *returnRenderBufferID = renderBufferID;

}



/*
 * prints texture bucket data
 */
void TextureManager::debugPrintBucketData() {
    for (size_t i = 0; i < textureBuckets.size(); i++){
        TextureBucket bucket = textureBuckets[i];
        printf("Bucket %lu\n"
               "Width = %lu, height = %lu\n"
               "Number of textures = %lu\n"
               "Texture ID = %d\n\n",
               i, bucket.width, bucket.height, bucket.numTextures, bucket.textureArrayID);
    }
}


void TextureManager::fillBuckets() {
    for (auto &bucket : textureBuckets){
        if (bucket.reserved) continue; //bucket is being used for something else (eg shadow maps)
        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);
        size_t levels = bucket.mipmap ? std::floor(log2(std::max(bucket.width, std::max(bucket.height, bucket.numTextures)))) + 1 : 1;
        glTexStorage3D(GL_TEXTURE_2D_ARRAY, levels, GL_RGBA8, bucket.width, bucket.height, bucket.numTextures);
        bucket.textureArrayID = textureID;
        assert(bucket.numTextures == bucket.textures.size());


        for (size_t i = 0; i < bucket.textures.size(); i++){
            addTextureToBucket(bucket, i);
        }
    }
}


void TextureManager::loadCubeMapRenderTexture(GLuint *returnTextureID, GLuint *returnRenderBufferID, GLuint frameBufferID, size_t width, size_t height) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    //TODO:
    //the alpha channel here should be unnecessary
    for (int i = 0; i < 6; i++){
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);



    GLuint renderBufferID;
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID);
    glGenRenderbuffers(1, &renderBufferID);
    glBindRenderbuffer(GL_RENDERBUFFER, renderBufferID);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderBufferID);

    *returnTextureID = textureID;
    *returnRenderBufferID = renderBufferID;
}



TextureManager::~TextureManager() {

}

GLuint TextureManager::getTextureID(size_t bucketID) {
    assert(bucketID < textureBuckets.size());
    return textureBuckets[bucketID].textureArrayID;
}
