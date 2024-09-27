#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <unordered_set>
#include "RenderManager.hpp"
#include "Skybox.hpp"
#include "RenderStructs.hpp"
#include "Material.hpp"


/*
 * the point of this class is to sort textures into texture arrays
 * we do this because the number of textures that can be bound at any given time is often quite small (eg 32)
 * a common way to overcome this is to use a texture atlas and have multiple textures saved in 1 image
 * but another way to solve this is through texture arrays - we store multiple 2d textures in a 2d array texture
 * and bind the 2d array texture
 * all of the textures in the array need to be the same dimensions
 */


namespace elmt {

    struct TextureInfo;

    struct TextureBucket{
        std::vector<uint8_t*> textures;
        GLuint textureArrayID;
        size_t width;
        size_t height;
        size_t numTextures;
        size_t reserved; //if set to non 0 then this bucket is reserved for non user supplied textures, eg shadow textures
        bool mipmap; //can textures in this bucket be mipmaped? things like height maps can't
    };


    class TextureManager {

        public:
            TextureManager(size_t maxBucketSize);
            ~TextureManager();

            void fillBuckets();

            TextureInfo loadTexture(Texture &texture, bool mipmap = true);
            void loadCubeMapRenderTexture(GLuint *returnTextureID, GLuint *returnRenderBufferID, GLuint frameBufferID, size_t width, size_t height);

            void createShadowBucket(size_t width, size_t height, size_t depth);
            GLuint getShadowTextureID();
            GLuint loadSkyBoxTexture(Skybox *skybox);
            GLuint loadSkyBoxIrradianceData(Skybox *skybox);


            void loadRenderTexture(GLuint *returnTextureID, GLuint *returnRenderBufferID, GLuint frameBufferID, size_t width, size_t height);
            GLuint createDepthBufferTexture(size_t width, size_t height);
            void debugPrintBucketData();
            GLuint getTextureID(size_t bucketID);


        private:
            size_t findBucket(size_t width, size_t height, bool mipmap);
            void createBucket(size_t width, size_t height, size_t depth, bool mipmap);
            void addTextureToBucket(TextureBucket &bucket, size_t layer);
            void loadShadowTexture();

            GLint maxTextureBuckets;
            GLint maxTextureBucketSize;
            GLint maxTextureSize;

            std::vector<TextureBucket> textureBuckets;

            GLuint depthBufferTexture = 0;
            TextureBucket *shadowBucket = nullptr;
    };
}


