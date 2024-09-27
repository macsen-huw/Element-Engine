#include "ProcManager.hpp"
#include "core.hpp"

#include <cstdlib>
#include <string>
#include <vector>

#include "../external/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../external/stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "../external/stb_image_resize2.h"

#include "../external/PerlinNoise.hpp"

#include "Logger.hpp"
#include "AssetManager.hpp"
#include <filesystem>



using namespace elmt;


/*
Take a height value and split it up into 8-bit rgb values
*/
void ProcManager::scalarToRGB(float height, float maxHeight, unsigned char* r, unsigned char* g, unsigned char* b) {
    
    const unsigned int maxIntVal = 16777216 - 1; //2^(8+8+8) - 1
    unsigned int heightInt24 = (unsigned int)((height / maxHeight) * maxIntVal);
    *r = (unsigned char)(heightInt24 >> 16);
    *g = (unsigned char)(heightInt24 >> 8);
    *b = (unsigned char)(heightInt24);


}

/*
Take 3 8-bit rgb values and turn them into a float
*/
float ProcManager::RGBToScalar(float maxVal, unsigned char r, unsigned char g, unsigned char b) {

    const unsigned int maxIntVal = 16777216 - 1; //2^(8+8+8) + 1
    unsigned int valInt24 = (unsigned int)0;
    
    valInt24 += ((unsigned int)r) << 16;
    valInt24 += ((unsigned int)g) << 8;
    valInt24 += ((unsigned int)b);

    float val;
    val = (float)valInt24 / (float)maxIntVal;
    val *= maxVal;

    return val;
}

void ProcManager::normalToRGB(glm::vec3 normal, unsigned char* r, unsigned char* g, unsigned char* b)
{
    *r = (colcom)(((normal.x + 1.0) * 0.5) * 255);
    *g = (colcom)(((normal.y + 1.0) * 0.5) * 255);
    *b = (colcom)(((normal.z + 1.0) * 0.5) * 255);

}

glm::vec3 ProcManager::RGBToNormal(unsigned char r, unsigned char g, unsigned char b) {
    float x = (((float)r / 255.0) * 2.0) - 1.0;
    float y = (((float)g / 255.0) * 2.0) - 1.0;
    float z = (((float)b / 255.0) * 2.0) - 1.0;

    glm::vec3 normal = glm::normalize( glm::vec3(x, y, z) );
    return normal;
}

float ProcManager::worley(worleyInfo info, float x, float y, unsigned int seed) {
    float cellW = 1.0 / (float)info.cellsX;
    float cellH = 1.0 / (float)info.cellsY;

    // Get cell point is in
    int sx = (int)(x * (float)info.cellsX);
    int sy = (int)(y * (float)info.cellsY);

    glm::vec2 samplePoint(x, y);

    float dist;
    float maxDist = 9999.0;


    int ci;
    for (int cx = -1; cx < 2; cx++) {
        for (int cy = -1; cy < 2; cy++) {
            ci = (sx * info.cellsY) + sy + (cx * info.cellsY) + cy;
            srand(seed + ci);
            // Advance random state
            rand(); rand(); rand();

            float cellX = ((sx + cx) * cellW);
            cellX += ((float)rand() / (float)RAND_MAX) * cellW;
            float cellY = ((sy + cy) * cellH);
            cellY += ((float)rand() / (float)RAND_MAX) * cellH;

            glm::vec2 cellPoint(cellX, cellY);

            glm::vec2 diff(samplePoint - cellPoint);
            float dist = glm::length(diff);

            if (dist < maxDist) {
                maxDist = dist;
            }

        }
    }

    float noise;
    // Sigmoid
    if (info.useSigmoid) {
        if (info.invertDist) {
            noise = 1.0 / (maxDist + info.dOffset);
        }
        else {
            noise = (maxDist + info.dOffset);
        }
        float ex = (noise + info.dIOffset) * info.dScale;
        const float e = 2.71828;
        noise = powf(e, ex) / (1.0 + powf(e, ex));
        noise += info.hOffset;
        noise *= info.hScale;
    }

    // Clamp
    if (noise > info.dMax) {
        noise = info.dMax;
    }

    if (noise < info.dMin) {
        noise = info.dMin;
    }

    // Not Sigmoid
    if (!info.useSigmoid) {
        noise = (noise - info.dMin) / (info.dMax - info.dMin);
    }

    return noise;
}

/*
Worley noise based procgen function
*/
void ProcManager::worleyHeightFunction(colcom* pixelPointer, float x, float y, unsigned int seed, void* extraData) {
    

   
    // Add extra info
    worleyInfo info;
    if (extraData) {
        info = *((worleyInfo*)extraData);
    }

    float noise =  core::getProcManager()->worley(info, x, y, seed);


    colcom r, g, b;
    core::getProcManager()->scalarToRGB(noise, 1.0, &r, &g, &b);

    pixelPointer[0] = r;
    pixelPointer[1] = g;
    pixelPointer[2] = b;

}


void ProcManager::perlinHeightFunction(colcom* pixelPointer, float x, float y, unsigned int seed, void* extraData)
{
    perlinInfo info;
    if (extraData) {
        info = *((perlinInfo*)extraData);
    }

    siv::PerlinNoise perlin;

    // Update from extra data
    if (extraData) {
        perlin = info.noiseEngine;
    }
    else {
        perlin = siv::PerlinNoise{ seed };
    }

    double noise = perlin.octave2D_01(x * info.xScale, y * info.xScale, info.octaves, info.persistance) * info.scale;

    colcom r, g, b;

    core::getProcManager()->scalarToRGB(noise, 1.0, &r, &g, &b);

    pixelPointer[0] = r;
    pixelPointer[1] = g;
    pixelPointer[2] = b;

}

void elmt::ProcManager::coneHeightFunction(colcom* pixelPointer, float x, float y, unsigned int seed, void* extraData)
{
    coneInfo info;
    if (extraData) {
        info = *((coneInfo*)extraData);
    }

    glm::vec2 p = glm::vec2(x, y);
    float dist = glm::length( p-glm::vec2(info.centerX,info.centerY) );

    float height = info.offset;   
    if (info.mode == CONEMODE::CONE_SHARP) {
        if (dist < info.maxDist) {
            height += 1.0 - (dist / info.maxDist);
            
        }
    }
    else if (info.mode == CONEMODE::CONE_NORMAL) {
        const float e = 2.71828;
        const float root2pi = 2.50663;
        
        float demonimator = info.standard_deviation * root2pi;
        float exponent = (-1.0 / 2.0) * std::pow((dist/info.dScale) / info.standard_deviation, 2.0);

        
        height += (1.0 / demonimator) * std::pow(e, exponent);

    }

    if (info.flip) {
        height = 1.0 - height;
    }
    height *= info.scale;

    colcom r, g, b;

    core::getProcManager()->scalarToRGB(height, 1.0, &r, &g, &b);

    pixelPointer[0] = r;
    pixelPointer[1] = g;
    pixelPointer[2] = b;


}


void ProcManager::materialHeightFunction(colcom* pixelPointer, float x, float y, unsigned int seed, void* extraData)
{
    // Return early if data not specified
    materialHeightInfo info;
    if (extraData) {
        info = *((materialHeightInfo*)extraData);
    }
    else {
        return;
    }

    if (!info.material || info.texture == MATERIALTEXTURE::NONE) {
        return;
    }

    // Choose texture
    colcom* imageData = nullptr;
    unsigned int imageWidth = 0;
    unsigned int imageHeight = 0;
    switch (info.texture) {
    case MATERIALTEXTURE::DIFFUSE:
        imageData = info.material->diffuseTexture.textureData;
        imageWidth = info.material->diffuseTexture.textureWidth;
        imageHeight = info.material->diffuseTexture.textureHeight;
        break;
    case MATERIALTEXTURE::NORMAL:
        imageData = info.material->normalTexture.textureData;
        imageWidth = info.material->normalTexture.textureWidth;
        imageHeight = info.material->normalTexture.textureHeight;
        break;
    case MATERIALTEXTURE::ROUGHNESS:
        imageData = info.material->roughnessTexture.textureData;
        imageWidth = info.material->roughnessTexture.textureWidth;
        imageHeight = info.material->roughnessTexture.textureHeight;
        break;
    case MATERIALTEXTURE::METAL:
        imageData = info.material->metallicnessTexture.textureData;
        imageWidth = info.material->metallicnessTexture.textureWidth;
        imageHeight = info.material->metallicnessTexture.textureHeight;
        break;

    }

    
    unsigned int px = (x * imageWidth);
    unsigned int py = (y * imageHeight);

    unsigned int index = core::getProcManager()->getIndex(px, py, imageWidth, imageHeight, info.imageChannels, false, true); // ((py * imageWidth) + (px))* info.imageChannels;

    float height;
    if (info.sampleChannel == -1) {
        // Average
        height = 0;
        for (int i = 0; i < info.imageChannels; i++) {
            colcom imageSample = imageData[index + i];
            height += (float)imageSample / (float)info.maxValue;
        }
        height /= (float)info.imageChannels;
    }
    else {
        colcom imageSample = imageData[index + info.sampleChannel];
        height = (float)imageSample / (float)info.maxValue;
    }
   

    colcom r, g, b;

    core::getProcManager()->scalarToRGB(height, 1.0, &r, &g, &b);

    pixelPointer[0] = r;
    pixelPointer[1] = g;
    pixelPointer[2] = b;


}

/*
Basic example procgen function
*/
void ProcManager::heightFunctionTest(colcom* pixelPointer, float x, float y, unsigned int seed, void* extraData)
{
    colcom r, g, b;

    r = (colcom)(x * 256.0);
    g = (colcom)(y * 256.0);
    b = seed % 256;

    pixelPointer[0] = r;
    pixelPointer[1] = g;
    pixelPointer[2] = b;

}

void ProcManager::waveBRDFFunction(colcom* diffusePointer, colcom* normalPointer, colcom* roughnessPointer, colcom* metalPointer, float x, float y, unsigned int seed, void* extraData)
{
    waveMaterialInfo info;
    if (extraData) {
        info = *((waveMaterialInfo*)extraData);
    }

    float multiplier = (float)info.waveNumber * 2.0 * 3.141592653589793;

    float waveIn = x + info.offset;
    float waveVal = sinf(waveIn * multiplier);
    float blendVal = (waveVal + 1.0) * 0.5;

    // Blend
    float dr, dg, db, da;
    dr = (float)info.tr - (float)info.br;
    dg = (float)info.tg - (float)info.bg;
    db = (float)info.tb - (float)info.bb;
    da = (float)info.ta - (float)info.ba;

    colcom rr, rg, rb, ra;
    rr = info.br + (colcom)(blendVal * dr);
    rg = info.bg + (colcom)(blendVal * dg);
    rb = info.bb + (colcom)(blendVal * db);
    ra = info.ba + (colcom)(blendVal * da);

    diffusePointer[0] = rr;
    diffusePointer[1] = rg;
    diffusePointer[2] = rb;
    diffusePointer[3] = ra;

    // Approximate normal
    glm::vec3 norm;
    if (info.generateNormalMap) {
        float sampleWidth = 0.0001;
        float sampleVal = sinf((waveIn + sampleWidth) * multiplier);
        float dy = (sampleVal - waveVal);

        glm::vec3 tangent{ 0.0, 0.0, 1.0 };
        tangent = glm::normalize(tangent);
        glm::vec3 bitangent{ sampleWidth, dy, 0.0 };
        bitangent = glm::normalize(bitangent);
        norm = glm::cross(tangent, bitangent);
        norm = glm::normalize(norm);
    }
    else {
        norm = glm::vec3{ 0.0, 1.0, 0.0 };
    }

    core::getProcManager()->normalToRGB(norm, normalPointer, normalPointer + 1, normalPointer + 2);


    roughnessPointer[0] = (colcom)(info.roughness * 255);
    metalPointer[0] = (colcom)(info.metalicness * 255);
}

void ProcManager::addChange(bool clamp, colcom change, colcom* diffusePointer, colcom* normalPointer, colcom* roughnessPointer, colcom* metalPointer) {
    if (clamp) {
        diffusePointer[0] = (colcom)std::clamp((int)diffusePointer[0] + change, 0, 255);
        diffusePointer[1] = (colcom)std::clamp((int)diffusePointer[1] + change, 0, 255);
        diffusePointer[2] = (colcom)std::clamp((int)diffusePointer[2] + change, 0, 255);
        diffusePointer[3] = (colcom)std::clamp((int)diffusePointer[3] + change, 0, 255);

        roughnessPointer[0] = (colcom)std::clamp((int)roughnessPointer[0] + change, 0, 255);
        metalPointer[0] = (colcom)std::clamp((int)metalPointer[0] + change, 0, 255);
    }
    else {
        diffusePointer[0] += change;
        diffusePointer[1] += change;
        diffusePointer[2] += change;
        diffusePointer[3] += change;

        roughnessPointer[0] += change;
        metalPointer[0] += change;
    }
}


void ProcManager::solidBRDFFunction(colcom* diffusePointer, colcom* normalPointer, colcom* roughnessPointer, colcom* metalPointer, float x, float y, unsigned int seed, void* extraData)
{
    solidMaterialInfo info;
    if (extraData) {
        info = *((solidMaterialInfo*)extraData);
    }

    diffusePointer[0] = info.r;
    diffusePointer[1] = info.g;
    diffusePointer[2] = info.b;
    diffusePointer[3] = info.a;

    glm::vec3 norm{ 0.0, 1.0, 0.0 };
    core::getProcManager()->normalToRGB(norm, normalPointer, normalPointer + 1, normalPointer + 2);

    roughnessPointer[0] = (colcom)(info.roughness * 255);
    metalPointer[0] = (colcom)(info.metalicness * 255);
}

void ProcManager::gradientBRDFFunction(colcom* diffusePointer, colcom* normalPointer, colcom* roughnessPointer, colcom* metalPointer, float x, float y, unsigned int seed, void* extraData)
{
    gradientMaterialInfo info;
    if (extraData) {
        info = *((gradientMaterialInfo*)extraData);
    }

    float val;
    if (info.useY) {
        val = y;
    }
    else {
        val = x;
    }

    glm::vec4 blendColour;
    float r, g, b, a, roughness, metalicness;

    typedef float (*mixFunction)(float a, float b, float x);
    mixFunction mixFunc;
    
    if (info.useSmoothstep) {
        mixFunc = glm::smoothstep;
    }
    else {
        mixFunc = glm::mix;
    }
    r = mixFunc((float)info.mat1.r, (float)info.mat2.r, val);
    g = mixFunc((float)info.mat1.g, (float)info.mat2.g, val);
    b = mixFunc((float)info.mat1.b, (float)info.mat2.b, val);
    a = mixFunc((float)info.mat1.a, (float)info.mat2.a, val);

    roughness = mixFunc((float)info.mat1.roughness, (float)info.mat2.roughness, val);
    metalicness = mixFunc((float)info.mat1.metalicness, (float)info.mat2.metalicness, val);

    diffusePointer[0] = (colcom)r;
    diffusePointer[1] = (colcom)g;
    diffusePointer[2] = (colcom)b;
    diffusePointer[3] = (colcom)a;

    glm::vec3 norm{ 0.0, 1.0, 0.0 };
    core::getProcManager()->normalToRGB(norm, normalPointer, normalPointer + 1, normalPointer + 2);

    roughnessPointer[0] = (colcom)(roughness * 255);
    metalPointer[0] = (colcom)(metalicness * 255);
    
}

void ProcManager::whiteNoiseBRDFFunction(colcom* diffusePointer, colcom* normalPointer, colcom* roughnessPointer, colcom* metalPointer, float x, float y, unsigned int seed, void* extraData)
{

    whiteNoiseMaterialInfo info;
    if (extraData) {
        info = *((whiteNoiseMaterialInfo*)extraData);
    }

    // Setup with solid colour
    solidBRDFFunction(diffusePointer, normalPointer, roughnessPointer, metalPointer, x, y, seed, &(info.base) );


    // Now add noise
    int change = (rand() % info.variance) - (info.variance / 2);

    core::getProcManager()->addChange(info.clamp, change, diffusePointer, normalPointer, roughnessPointer, metalPointer);

}

void ProcManager::perlinNoiseBRDFFunction(colcom* diffusePointer, colcom* normalPointer, colcom* roughnessPointer, colcom* metalPointer, float x, float y, unsigned int seed, void* extraData)
{

    perlinNoiseMaterialInfo info;
    if (extraData) {
        info = *((perlinNoiseMaterialInfo*)extraData);
    }

    // Setup with solid colour
    solidBRDFFunction(diffusePointer, normalPointer, roughnessPointer, metalPointer, x, y, seed, &(info.base));

    // Update from extra data
    siv::PerlinNoise perlin;
    if (extraData) {
        perlin = info.noiseEngine;
    }
    else {
        perlin = siv::PerlinNoise{ seed };
    }

    // Now add noise
    int change = (perlin.octave2D_01(x, y, info.octaves, info.persistance)-0.5) * info.variance;

    core::getProcManager()->addChange(info.clamp, change, diffusePointer, normalPointer, roughnessPointer, metalPointer);

}


void ProcManager::worleyNoiseBRDFFunction(colcom* diffusePointer, colcom* normalPointer, colcom* roughnessPointer, colcom* metalPointer, float x, float y, unsigned int seed, void* extraData)
{

    worleyNoiseMaterialInfo info;
    if (extraData) {
        info = *((worleyNoiseMaterialInfo*)extraData);
    }

    // Setup with solid colour
    solidBRDFFunction(diffusePointer, normalPointer, roughnessPointer, metalPointer, x, y, seed, &(info.base));

    float noise = core::getProcManager()->worley(info.worley, x, y, seed) - (info.worley.dScale/2.0);

    // Now add noise
    int change = (noise) * 255;

    core::getProcManager()->addChange(info.clamp, change, diffusePointer, normalPointer, roughnessPointer, metalPointer);

}

void ProcManager::brickBRDFFunction(colcom* diffusePointer, colcom* normalPointer, colcom* roughnessPointer, colcom* metalPointer, float x, float y, unsigned int seed, void* extraData)
{

    brickMaterialInfo info;
    if (extraData) {
        info = *((brickMaterialInfo*)extraData);
    }

    // Find out which brick we are on and where in the brick we are
    unsigned int bx, by;
    bx = x / info.brickWidth;
    by = y / info.brickHeight;
    float ox, oy;
    ox = fmodf(x, info.brickWidth);
    oy = fmodf(y, info.brickHeight);

    bool inFiller = false;
    if (ox <= info.fillerWidth || oy <= info.fillerWidth || ox >= info.brickWidth-info.fillerWidth || oy >= info.brickHeight - info.fillerWidth) {
        inFiller = true;
    }


    // Setup with solid colour
    if (inFiller) {
        solidBRDFFunction(diffusePointer, normalPointer, roughnessPointer, metalPointer, x, y, seed, &(info.brickInfo));
    } else {
        solidBRDFFunction(diffusePointer, normalPointer, roughnessPointer, metalPointer, x, y, seed, &(info.fillerInfo));
    }
    
    // Overwrite normal
    if (inFiller && info.fillerRandomNormal) {
        glm::vec3 randNormal;
        randNormal.x = ((float)rand() / (float)RAND_MAX);
        randNormal.y = ((float)rand() / (float)RAND_MAX);
        randNormal.y = ((float)rand() / (float)RAND_MAX);
        randNormal = glm::normalize(randNormal); // Not perfect distribution but who cares
        core::getProcManager()->normalToRGB(randNormal, normalPointer, normalPointer + 1, normalPointer + 2);
    }
}


void ProcManager::patternBRDFFunction(colcom* diffusePointer, colcom* normalPointer, colcom* roughnessPointer, colcom* metalPointer, float x, float y, unsigned int seed, void* extraData)
{

    patternMaterialInfo info;
    if (extraData) {
        info = *((patternMaterialInfo*)extraData);
    }

    // Find out which brick we are on and where in the brick we are
    unsigned int bx, by;
    bx = x / info.patternWidth;
    by = y / info.patternHeight;
    float ox, oy;
    ox = fmodf(x, info.patternWidth);
    oy = fmodf(y, info.patternHeight);
    unsigned int px, py;
    px = (ox / info.patternWidth) * info.pattern.patternWidth;
    py = (oy / info.patternHeight) * info.pattern.patternHeight;

    unsigned int index = core::getProcManager()->getIndex(px, py, info.pattern.patternWidth, info.pattern.patternHeight, 1); // (py * info.pattern.patternWidth) + px;
    char sampleChar = info.pattern.data[index];
    bool useMat2 = true;
    if (sampleChar == '0') {
        useMat2 = false;
    }


    // Setup with solid colour
    
    if (useMat2) {
        solidBRDFFunction(diffusePointer, normalPointer, roughnessPointer, metalPointer, x, y, seed, &(info.mat2Info));
    }
    else {
        solidBRDFFunction(diffusePointer, normalPointer, roughnessPointer, metalPointer, x, y, seed, &(info.mat1Info));
    }

}


void ProcManager::BRDFFunctionTest(colcom* diffusePointer, colcom* normalPointer, colcom* roughnessPointer, colcom* metalPointer, float x, float y, unsigned int seed, void* extraData)
{

    colcom r, g, b, a;

    r = (colcom)(x * 255.0);
    g = (colcom)(y * 255.0);
    b = seed % 255;
    a = 255;
    if (x > 0.8 && y > 0.8) {
        a = 128;
    }

    diffusePointer[0] = r;
    diffusePointer[1] = g;
    diffusePointer[2] = b;
    diffusePointer[3] = a;

    glm::vec3 norm{ x, 0.5, 1.0-y };
    norm -= glm::vec3(0.5, 0.0, 0.5);
    norm = glm::normalize(norm);
    core::getProcManager()->normalToRGB(norm, normalPointer, normalPointer + 1, normalPointer + 2);


    colcom ro = (y * 255);
    roughnessPointer[0] = ro;

    colcom m = (x * 255);
    metalPointer[0] = m;
    
}

Material ProcManager::generateBRDFMaterial(const char* outName, const char *outFormat, unsigned int width, unsigned int height, BRDFGenFunction genFunction, unsigned int seed, void* extraData, bool loadIfExists)
{
    Material heightMat;
    heightMat.diffuseTexture.textureData = nullptr;
    heightMat.ambient = glm::vec3(1.0, 1.0, 1.0);
    heightMat.diffuse = glm::vec3(1.0, 1.0, 1.0);
    heightMat.emissive = glm::vec3(0.0, 0.0, 0.0);
    heightMat.shininess = 1.0;
    heightMat.specular = glm::vec3(0.0, 0.0, 0.0);
    heightMat.translucent = false;

    

    unsigned int diffuseChannels = 4;
    unsigned int normalChannels = 4;
    unsigned int roughnessChannels = 4;
    unsigned int metalChannels = 4;

    size_t diffuseImageSize = width * height * diffuseChannels;
    size_t normalImageSize = width * height * normalChannels;
    size_t roughnessImageSize = width * height * roughnessChannels;
    size_t metalImageSize = width * height * metalChannels;

    colcom* diffuse = (colcom*)malloc(diffuseImageSize);
    colcom* normal = (colcom*)malloc(normalImageSize);
    colcom* roughness = (colcom*)malloc(roughnessImageSize);
    colcom* metal = (colcom*)malloc(metalImageSize);


    if (diffuse == nullptr || normal == nullptr || roughness == nullptr || metal == nullptr) {
        Logger::Print("ProcManager.generateBRDFMaterial: Failed to allocate memory of size: " + std::to_string(diffuseImageSize + normalImageSize + roughnessImageSize + metalImageSize) +
            " bytes", LOGCAT::CAT_LOADING, LOGSEV::SEV_ERROR);
        return heightMat;
    }


    std::string sOutName = std::string(outName);
    std::string sOutFormat = std::string(outFormat);

    std::string outPathDiffuse = (sOutName + "_Diffuse" + sOutFormat);
    std::string outPathNormal = (sOutName + "_Normal" + sOutFormat);
    std::string outPathRoughness = (sOutName + "_Roughness" + sOutFormat);
    std::string outPathMetal = (sOutName + "_Metal" + sOutFormat);

    bool doGen = true;
    if (outName && loadIfExists) {
        bool dExists = std::filesystem::exists(outPathDiffuse);
        bool nExists = std::filesystem::exists(outPathNormal);
        bool rExists = std::filesystem::exists(outPathRoughness);
        bool mExists = std::filesystem::exists(outPathMetal);
        // All files already exists, so just load
        if (dExists && nExists && rExists && mExists) {
            doGen = false;
        }
    }

    if (doGen) {
        // Fill in default values
        memset(diffuse, (colcom)255, diffuseImageSize);
        memset(normal, (colcom)255, normalImageSize);
        memset(roughness, (colcom)255, roughnessImageSize);
        memset(metal, (colcom)255, metalImageSize);

        srand(seed);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int index = getIndex(x, y, width, height, 1); //((y * width) + (x));

                colcom* pd = diffuse + (index * diffuseChannels);
                colcom* pn = normal + (index * normalChannels);
                colcom* pr = roughness + (index * roughnessChannels);
                colcom* pm = metal + (index * metalChannels);



                float xf = (float)x / float(width);
                float yf = (float)y / float(height);
                genFunction(pd, pn, pr, pm, xf, yf, seed, extraData);

                // Fill in unused channels 

                pr[1] = pr[0];
                pr[2] = pr[0];


                pm[1] = pm[0];
                pm[2] = pm[0];

            }
        }

        heightMat.diffuseTexture.textureData = (uint8_t*)diffuse;
        heightMat.diffuseTexture.textureWidth = width;
        heightMat.diffuseTexture.textureHeight = height;

        heightMat.normalTexture.textureData = (uint8_t*)normal;
        heightMat.normalTexture.textureWidth = width;
        heightMat.normalTexture.textureHeight = height;

        heightMat.roughnessTexture.textureData = (uint8_t*)roughness;
        heightMat.roughnessTexture.textureWidth = width;
        heightMat.roughnessTexture.textureHeight = height;

        heightMat.metallicnessTexture.textureData = (uint8_t*)metal;
        heightMat.metallicnessTexture.textureWidth = width;
        heightMat.metallicnessTexture.textureHeight = height;


        // Output to files if path specified
        if (outName) {
            // TODO improve
            setWriteFlip();

            int res = stbi_write_png(outPathDiffuse.c_str(), width, height, diffuseChannels, diffuse, width * diffuseChannels);
            if (!res) {
                Logger::Print("ProcManager.generateBRDFMaterial: Failed to write to output file (diffuse)", LOGCAT::CAT_LOADING, LOGSEV::SEV_ERROR);
            }
            res = stbi_write_png(outPathNormal.c_str(), width, height, normalChannels, normal, width * normalChannels);
            if (!res) {
                Logger::Print("ProcManager.generateBRDFMaterial: Failed to write to output file (normal)", LOGCAT::CAT_LOADING, LOGSEV::SEV_ERROR);
            }
            res = stbi_write_png(outPathRoughness.c_str(), width, height, roughnessChannels, roughness, width * roughnessChannels);
            if (!res) {
                Logger::Print("ProcManager.generateBRDFMaterial: Failed to write to output file (roughness)", LOGCAT::CAT_LOADING, LOGSEV::SEV_ERROR);
            }
            res = stbi_write_png(outPathMetal.c_str(), width, height, metalChannels, metal, width * metalChannels);
            if (!res) {
                Logger::Print("ProcManager.generateBRDFMaterial: Failed to write to output file (metal)", LOGCAT::CAT_LOADING, LOGSEV::SEV_ERROR);
            }
        }
    }
    else {

        // We already have everything, just load
        CreateMaterialInfo matInfo;
        matInfo.ambient = heightMat.ambient;
        matInfo.diffuse = heightMat.diffuse;
        matInfo.specular = heightMat.specular;
        matInfo.shininess = heightMat.shininess;
        matInfo.translucent = heightMat.translucent;
        
        matInfo.diffuseTexture = outPathDiffuse.c_str();
        matInfo.normalTexture = outPathNormal.c_str();
        matInfo.roughnessTexture = outPathRoughness.c_str();
        matInfo.metallicnessTexture = outPathMetal.c_str();

        heightMat = core::getAssetManager()->createMaterial(matInfo);
    }

    

    return heightMat;
}


/*
Generate a heightmap material using a given generation function and seed
If outPath isn't empty, then the heightmap will be saved to disk

The generation function should be a function with a signature similar to: 
void (unsigned char* pixelPointer, float x, float y, unsigned int seed, void* extraData)

Where x and y go from 0.0->1.0.
pixelPointer is used to set the RGB components of a given pixel from 0->255, like so:

pixelPointer[0] = r;
pixelPointer[1] = g;
pixelPointer[2] = b;

seed is a generation seed used for noise functions
extraData can be used to provide any additional info you want to pass into the function

*/
Material ProcManager::generateHeightmap(const char* outPath, unsigned int width, unsigned int height, heightmapGenFunction genFunction, unsigned int seed, void* extraData, bool loadIfExists)
{

    Material heightMat;
    heightMat.diffuseTexture.textureData = nullptr;
    heightMat.ambient = glm::vec3(1.0, 1.0, 1.0);
    heightMat.diffuse = glm::vec3(1.0, 1.0, 1.0);
    heightMat.emissive = glm::vec3(0.0, 0.0, 0.0);
    heightMat.shininess = 1.0;
    heightMat.specular = glm::vec3(0.0, 0.0, 0.0);
    heightMat.translucent = false;

    

    bool doGen = true;
    if (outPath && loadIfExists) {
        bool hExists = std::filesystem::exists(outPath);
        // File already exists, so just load
        if (hExists) {
            doGen = false;
        }
    }

    if (doGen) {
        unsigned int channels = 3;

        size_t heightmapSize = width * height * channels;

        colcom* heightmap = (colcom*)malloc(heightmapSize);

        if (heightmap == nullptr) {
            Logger::Print("ProcManager.generateHeightmap: Failed to allocate memory of size: " + std::to_string(heightmapSize) + " bytes", LOGCAT::CAT_LOADING, LOGSEV::SEV_ERROR);
            return heightMat;
        }

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int index = getIndex(x, y, width, height, channels, false, true); //((y * width) + (x)) * channels;

                colcom* p = heightmap + index;

                float xf = (float)x / float(width);
                float yf = (float)y / float(height);
                genFunction(p, xf, yf, seed, extraData);

            }
        }

        heightMat.diffuseTexture.textureData = (uint8_t*)heightmap;
        heightMat.diffuseTexture.textureWidth = width;
        heightMat.diffuseTexture.textureHeight = height;

        // Output to file if path specified
        if (outPath) {
            // TODO improve
            setWriteFlip(true);
            

            int res = stbi_write_png(outPath, width, height, channels, heightmap, width * channels);
            if (!res) {
                Logger::Print("ProcManager.generateHeightmap: Failed to write to output file", LOGCAT::CAT_LOADING, LOGSEV::SEV_ERROR);
            }
        }
    }
    else {
        // We already have everything, just load
        CreateMaterialInfo matInfo;
        matInfo.ambient = heightMat.ambient;
        matInfo.diffuse = heightMat.diffuse;
        matInfo.specular = heightMat.specular;
        matInfo.shininess = heightMat.shininess;
        matInfo.translucent = heightMat.translucent;

        matInfo.diffuseTexture = outPath;

        matInfo.channels = 3;

        //matInfo.flipVertically = false;
        //stbi_set_flip_vertically_on_load(false);
        heightMat = core::getAssetManager()->createMaterial(matInfo);
    }

    return heightMat;
}

Material ProcManager::mixMaterials(
    Material& mat1,
    Material& mat2,
    const char* outName, const char* outFormat, unsigned int outWidth, unsigned int outHeight,
    BlendInfo info
) {

    unsigned int diffuseChannels = 4;
    unsigned int normalChannels = 4;
    unsigned int roughnessChannels = 4;
    unsigned int metalChannels = 4;

    

    size_t diffuseSize = outWidth * outHeight * diffuseChannels;
    size_t normalSize = outWidth * outHeight * normalChannels;
    size_t roughnessSize = outWidth * outHeight * roughnessChannels;
    size_t metalSize = outWidth * outHeight * metalChannels;

    // Create resized version of material 1, if needed
    colcom* m1D = nullptr, *m1N = nullptr, *m1R = nullptr, *m1M = nullptr;
    // DIFFUSE
    if (mat1.diffuseTexture.textureData && mat1.diffuseTexture.textureWidth && mat1.diffuseTexture.textureHeight) {
        if (mat1.diffuseTexture.textureWidth == outWidth && mat1.diffuseTexture.textureHeight == outHeight) {
            m1D = mat1.diffuseTexture.textureData;
        }
        else {
            m1D = (colcom*)malloc(diffuseSize);
            stbir_resize_uint8_srgb(mat1.diffuseTexture.textureData, mat1.diffuseTexture.textureWidth, mat1.diffuseTexture.textureHeight, mat1.diffuseTexture.textureWidth * diffuseChannels,
                m1D, outWidth, outHeight, outWidth * diffuseChannels, STBIR_RGBA);
        }
    }
    // NORMAL
    if (mat1.normalTexture.textureData && mat1.normalTexture.textureWidth && mat1.normalTexture.textureHeight) {
        if (mat1.normalTexture.textureWidth == outWidth && mat1.normalTexture.textureHeight == outHeight) {
            m1N = mat1.normalTexture.textureData;
        }
        else {
            m1N = (colcom*)malloc(normalChannels);
            stbir_resize_uint8_srgb(mat1.normalTexture.textureData, mat1.normalTexture.textureWidth, mat1.normalTexture.textureHeight, mat1.normalTexture.textureWidth * normalChannels,
                m1N, outWidth, outHeight, outWidth * normalChannels, STBIR_RGBA);
        }
    }

    // ROUGHNESS
    if (mat1.roughnessTexture.textureData && mat1.roughnessTexture.textureWidth && mat1.roughnessTexture.textureHeight) {
        if (mat1.roughnessTexture.textureWidth == outWidth && mat1.roughnessTexture.textureHeight == outHeight) {
            m1R = mat1.roughnessTexture.textureData;
        }
        else {
            m1R = (colcom*)malloc(roughnessSize);
            stbir_resize_uint8_srgb(mat1.roughnessTexture.textureData, mat1.roughnessTexture.textureWidth, mat1.roughnessTexture.textureHeight, mat1.roughnessTexture.textureWidth * roughnessChannels,
                m1R, outWidth, outHeight, outWidth * roughnessChannels, STBIR_RGBA);
        }
    }

    // METAL
    if (mat1.metallicnessTexture.textureData && mat1.metallicnessTexture.textureWidth && mat1.metallicnessTexture.textureHeight) {
        if (mat1.metallicnessTexture.textureWidth == outWidth && mat1.metallicnessTexture.textureHeight == outHeight) {
            m1M = mat1.metallicnessTexture.textureData;
        }
        else {
            m1M = (colcom*)malloc(metalSize);
            stbir_resize_uint8_srgb(mat1.metallicnessTexture.textureData, mat1.metallicnessTexture.textureWidth, mat1.metallicnessTexture.textureHeight, mat1.metallicnessTexture.textureWidth * metalChannels,
                m1M, outWidth, outHeight, outWidth * metalChannels, STBIR_RGBA);
        }
    }

    /*
    if (m1D == nullptr || m1N == nullptr || m1R == nullptr || m1M == nullptr ) {
        Logger::Print("ProcManager.mixMaterials: Failed to allocate memory of size: " + std::to_string(diffuseSize + normalSize + roughnessSize + metalSize) +
            " bytes", LOGCAT::CAT_LOADING, LOGSEV::SEV_ERROR);
        return Material();
    }
    */

    // Create resized version of material 2, if needed
    colcom* m2D = nullptr, * m2N = nullptr, * m2R = nullptr, * m2M = nullptr;
    // DIFFUSE
    if (m1D && mat2.diffuseTexture.textureData && mat2.diffuseTexture.textureWidth && mat2.diffuseTexture.textureHeight) {
        if (mat2.diffuseTexture.textureWidth == outWidth && mat2.diffuseTexture.textureHeight == outHeight) {
            m2D = mat2.diffuseTexture.textureData;
        }
        else {
            m2D = (colcom*)malloc(diffuseSize);
            stbir_resize_uint8_srgb(mat2.diffuseTexture.textureData, mat2.diffuseTexture.textureWidth, mat2.diffuseTexture.textureHeight, mat2.diffuseTexture.textureWidth * diffuseChannels,
                m2D, outWidth, outHeight, outWidth * diffuseChannels, STBIR_RGBA);
        }
    }
    // NORMAL
    if (m1N && mat2.normalTexture.textureData && mat2.normalTexture.textureWidth && mat2.normalTexture.textureHeight) {
        if (mat2.normalTexture.textureWidth == outWidth && mat2.normalTexture.textureHeight == outHeight) {
            m2N = mat2.normalTexture.textureData;
        }
        else {
            m2N = (colcom*)malloc(normalChannels);
            stbir_resize_uint8_srgb(mat2.normalTexture.textureData, mat2.normalTexture.textureWidth, mat2.normalTexture.textureHeight, mat2.normalTexture.textureWidth * normalChannels,
                m2N, outWidth, outHeight, outWidth * normalChannels, STBIR_RGBA);
        }
    }

    // ROUGHNESS
    if (m1R && mat2.roughnessTexture.textureData && mat2.roughnessTexture.textureWidth && mat2.roughnessTexture.textureHeight) {
        if (mat2.roughnessTexture.textureWidth == outWidth && mat2.roughnessTexture.textureHeight == outHeight) {
            m2R = mat2.roughnessTexture.textureData;
        }
        else {
            m2R = (colcom*)malloc(roughnessSize);
            stbir_resize_uint8_srgb(mat2.roughnessTexture.textureData, mat2.roughnessTexture.textureWidth, mat2.roughnessTexture.textureHeight, mat2.roughnessTexture.textureWidth * roughnessChannels,
                m2R, outWidth, outHeight, outWidth * roughnessChannels, STBIR_RGBA);
        }
    }

    // METAL
    if (m1M && mat2.metallicnessTexture.textureData && mat2.metallicnessTexture.textureWidth && mat2.metallicnessTexture.textureHeight) {
        if (mat2.metallicnessTexture.textureWidth == outWidth && mat2.metallicnessTexture.textureHeight == outHeight) {
            m2M = mat2.metallicnessTexture.textureData;
        }
        else {
            m2M = (colcom*)malloc(metalSize);
            stbir_resize_uint8_srgb(mat2.metallicnessTexture.textureData, mat2.metallicnessTexture.textureWidth, mat2.metallicnessTexture.textureHeight, mat2.metallicnessTexture.textureWidth * metalChannels,
                m2M, outWidth, outHeight, outWidth * metalChannels, STBIR_RGBA);
        }
    }



    std::vector<colcom*> mat1Textures{ m1D, m1N, m1R, m1N };
    std::vector<colcom*> mat2Textures{ m2D, m2N, m2R, m2N };
    std::vector<unsigned int> texturesChannels{ diffuseChannels, normalChannels, roughnessChannels, metalChannels };

    colcom* mFD = nullptr, * mFN = nullptr, * mFR = nullptr, * mFM = nullptr;
    std::vector<colcom*> matFinalTextures;

    colcom* mat1Tex, *mat2Tex, *matFinalTex;

    


    unsigned int texChannels;
    for (int texI = 0; texI < mat1Textures.size(); texI++) {
        mat1Tex = mat1Textures[texI];
        mat2Tex = mat2Textures[texI];
        // Exit early if the texture is not loaded
        if (!mat1Tex || !mat2Tex) {
            continue;
        }

        texChannels = texturesChannels[texI];

        // Create new texture
        size_t finalSize = outWidth * outHeight * texChannels;
        matFinalTex = (colcom*)malloc(finalSize);
        matFinalTextures.push_back(matFinalTex);

        int xOffsetPixels = -info.xOffset * outWidth;
        int yOffsetPixels = -info.yOffset * outHeight;
        unsigned int maxIndex = outWidth * outHeight * texChannels;
        for (int y = 0; y < outHeight; y++) {
            for (int x = 0; x < outWidth; x++) {
                int index = getIndex(x, y, outWidth, outHeight, texChannels, false, false); //((y * outWidth) + (x)) * texChannels;
                colcom* pf = matFinalTex + index;

                // Load
                int index1 = getIndex(x, y, outWidth, outHeight, texChannels, false, true); //((y * outWidth) + (x)) * texChannels;
                colcom* p1 = mat1Tex + index;
                float r1, g1, b1, a1;
                r1 = ((float)p1[0] * info.mat1Weight) / 255.0;
                g1 = ((float)p1[1] * info.mat1Weight) / 255.0;
                b1 = ((float)p1[2] * info.mat1Weight) / 255.0;
                a1 = ((float)p1[3] * info.mat1Weight) / 255.0;
                
                int x2 = x + xOffsetPixels;
                int y2 = y + yOffsetPixels;
                // We are out of bounds due to offset, skip
                if (x2 < 0 || y2 < 0 || x2 >= outWidth || y2 >= outHeight) {
                    pf[0] = p1[0];
                    pf[1] = p1[1];
                    pf[2] = p1[2];
                    pf[3] = p1[3];
                    continue;
                }
                int index2 = getIndex(x2, y2, outWidth, outHeight, texChannels, false, true); //(((y2) * outWidth) + (x2)) * texChannels;
                

                colcom* p2 = mat2Tex + index2;
                float r2, g2, b2, a2;
                r2 = ((float)p2[0] * info.mat2Weight) / 255.0;
                g2 = ((float)p2[1] * info.mat2Weight) / 255.0;
                b2 = ((float)p2[2] * info.mat2Weight) / 255.0;
                a2 = ((float)p2[3] * info.mat2Weight) / 255.0;
                


                // Blend
                float rf, gf, bf, af;
                switch (info.blendMode) {
                    case BLENDMODE::MODE_ADD:
                        rf = r1 + r2;
                        gf = g1 + g2;
                        bf = b1 + b2;
                        af = a1 + a2;
                        break;
                    
                    case BLENDMODE::MODE_SUB:
                        rf = r1 - r2;
                        gf = g1 - g2;
                        bf = b1 - b2;
                        af = a1 - a2;
                        break;
                    
                    case BLENDMODE::MODE_MUL:
                        rf = r1 * r2;
                        gf = g1 * g2;
                        bf = b1 * b2;
                        af = a1 * a2;
                        break;
                    
                    case BLENDMODE::MODE_MAX:
                        rf = std::fmaxf(r1, r2);
                        gf = std::fmaxf(g1, g2);
                        bf = std::fmaxf(b1, b2);
                        af = std::fmaxf(a1, a2);
                        break;
                    
                    case BLENDMODE::MODE_MIN:
                        rf = std::fminf(r1, r2);
                        gf = std::fminf(g1, g2);
                        bf = std::fminf(b1, b2);
                        af = std::fminf(a1, a2);
                        break;
                    
                    case BLENDMODE::MODE_MASK:
                        MaskInfo& mi = info.maskInfo;
                        if ( (mi.compareMode == COLOURMASKMODE::COLOUR_MASK_ANY && ( (r2 >= mi.minR && r2 <= mi.maxR) || (g2 >= mi.minG && g2 <= mi.maxG) || (b2 >= mi.minB && b2 <= mi.maxB) || (a2 >= mi.minA && a2 <= mi.maxA))) ||
                             (mi.compareMode == COLOURMASKMODE::COLOUR_MASK_ALL && ( (r2 >= mi.minR && r2 <= mi.maxR) && (g2 >= mi.minG && g2 <= mi.maxG) && (b2 >= mi.minB && b2 <= mi.maxB) && (a2 >= mi.minA && a2 <= mi.maxA))) ||
                             (mi.compareMode == COLOURMASKMODE::COLOUR_MASK_R   && (r2 >= mi.minR && r2 <= mi.maxR) ) ||
                             (mi.compareMode == COLOURMASKMODE::COLOUR_MASK_G && (g2 >= mi.minG && g2 <= mi.maxG)) ||
                             (mi.compareMode == COLOURMASKMODE::COLOUR_MASK_B && (b2 >= mi.minB && b2 <= mi.maxB)) ||
                             (mi.compareMode == COLOURMASKMODE::COLOUR_MASK_A && (a2 >= mi.minA && a2 <= mi.maxA))
                           ) {
                            // Pass
                            rf = r1;
                            gf = g1;
                            bf = b1;
                            af = a1;
                        }
                        else {
                            // Fail
                            rf = mi.failR;
                            gf = mi.failG;
                            bf = mi.failB;
                            af = mi.failA;
                        }
                        break;
                }
                

                // Clamp
                if (info.clamp) {
                    rf = std::clamp(rf, 0.0f, 1.0f);
                    gf = std::clamp(gf, 0.0f, 1.0f);
                    bf = std::clamp(bf, 0.0f, 1.0f);
                    af = std::clamp(af, 0.0f, 1.0f);
                }

                
                colcom ri, gi, bi, ai;
                ri = (colcom)(rf * 255);
                gi = (colcom)(gf * 255);
                bi = (colcom)(bf * 255);
                ai = (colcom)(af * 255);
                pf[0] = ri;
                pf[1] = gi;
                pf[2] = bi;
                pf[3] = ai;

            }
        }

        switch (texI){
        case 0:
            m1D = matFinalTex;
            break;
        case 1:
            m1N = matFinalTex;
            break;
        case 2:
            m1R = matFinalTex;
            break;
        case 3:
            m1M = matFinalTex;
            break;
        }

    }


    // Output to files if path specified
    if (outName) {
        // TODO improve
        setWriteFlip();

        std::string sOutName = std::string(outName);
        std::string sOutFormat = std::string(outFormat);

        std::string outPathDiffuse = (sOutName + "_Diffuse" + sOutFormat);
        std::string outPathNormal = (sOutName + "_Normal" + sOutFormat);
        std::string outPathRoughness = (sOutName + "_Roughness" + sOutFormat);
        std::string outPathMetal = (sOutName + "_Metal" + sOutFormat);

        int res;
        if (m1D) {
            res = stbi_write_png(outPathDiffuse.c_str(), outWidth, outHeight, diffuseChannels, m1D, outWidth * diffuseChannels);
            if (!res) {
                Logger::Print("ProcManager.mixMaterials: Failed to write to output file (diffuse)", LOGCAT::CAT_LOADING, LOGSEV::SEV_ERROR);
            }
        }

        if (m1N) {
            res = stbi_write_png(outPathNormal.c_str(), outWidth, outHeight, normalChannels, m1N, outWidth * normalChannels);
            if (!res) {
                Logger::Print("ProcManager.mixMaterials: Failed to write to output file (normal)", LOGCAT::CAT_LOADING, LOGSEV::SEV_ERROR);
            }
        }

        if (m1R) {
            res = stbi_write_png(outPathRoughness.c_str(), outWidth, outHeight, roughnessChannels, m1R, outWidth * roughnessChannels);
            if (!res) {
                Logger::Print("ProcManager.mixMaterials: Failed to write to output file (roughness)", LOGCAT::CAT_LOADING, LOGSEV::SEV_ERROR);
            }
        }

        if (m1M) {
            res = stbi_write_png(outPathMetal.c_str(), outWidth, outHeight, metalChannels, m1M, outWidth * metalChannels);
            if (!res) {
                Logger::Print("ProcManager.mixMaterials: Failed to write to output file (metal)", LOGCAT::CAT_LOADING, LOGSEV::SEV_ERROR);
            }
        }
    }

    // Clean up material 1
    if (mat1.diffuseTexture.textureWidth != outWidth || mat1.diffuseTexture.textureHeight != outHeight) {
        free(m1D);
    }
    if (mat1.normalTexture.textureWidth != outWidth || mat1.normalTexture.textureHeight != outHeight) {
        free(m1N);
    }
    if (mat1.roughnessTexture.textureWidth != outWidth || mat1.roughnessTexture.textureHeight != outHeight) {
        free(m1R);
    }
    if (mat1.metallicnessTexture.textureWidth != outWidth || mat1.metallicnessTexture.textureHeight != outHeight) {
        free(m1M);
    }

    // Clean up material 2
    if (mat2.diffuseTexture.textureWidth != outWidth || mat2.diffuseTexture.textureHeight != outHeight) {
        free(m2D);
    }
    if (mat2.normalTexture.textureWidth != outWidth || mat2.normalTexture.textureHeight != outHeight) {
        free(m2N);
    }
    if (mat2.roughnessTexture.textureWidth != outWidth || mat2.roughnessTexture.textureHeight != outHeight) {
        free(m2R);
    }
    if (mat2.metallicnessTexture.textureWidth != outWidth || mat2.metallicnessTexture.textureHeight != outHeight) {
        free(m2M);
    }

    // Create material
    Material outMat;


    // Mix colours and scalars
    glm::vec3 mat1Ambient{ mat1.ambient * info.mat1Weight };
    glm::vec3 mat1Diffuse{ mat1.diffuse * info.mat1Weight };
    glm::vec3 mat1Specular{ mat1.specular * info.mat1Weight };
    float mat1Shininess = mat1.shininess;

    glm::vec3 mat2Ambient{ mat2.ambient * info.mat2Weight };
    glm::vec3 mat2Diffuse{ mat2.diffuse * info.mat2Weight };
    glm::vec3 mat2Specular{ mat2.specular * info.mat2Weight };
    float mat2Shininess = mat2.shininess;
    switch (info.blendMode)
    {
    case (BLENDMODE::MODE_ADD):
        outMat.ambient = mat1Ambient + mat2Ambient;
        outMat.diffuse = mat1Diffuse + mat2Diffuse;
        outMat.specular = mat1Specular + mat2Specular;
        outMat.shininess = mat1Shininess + mat2Shininess;
        break;
    case (BLENDMODE::MODE_SUB):
        outMat.ambient = mat1Ambient - mat2Ambient;
        outMat.diffuse = mat1Diffuse - mat2Diffuse;
        outMat.specular = mat1Specular - mat2Specular;
        outMat.shininess = mat1Shininess - mat2Shininess;
        break;
    case (BLENDMODE::MODE_MUL):
        outMat.ambient = mat1Ambient * mat2Ambient;
        outMat.diffuse = mat1Diffuse * mat2Diffuse;
        outMat.specular = mat1Specular * mat2Specular;
        outMat.shininess = mat1Shininess * mat2Shininess;
        break;
    case (BLENDMODE::MODE_MAX):
        outMat.ambient = glm::max(mat1Ambient,mat2Ambient);
        outMat.diffuse = glm::max(mat1Diffuse,mat2Diffuse);
        outMat.specular = glm::max(mat1Specular,mat2Specular);
        outMat.shininess = glm::max(mat1Shininess,mat2Shininess);
        break;
    case (BLENDMODE::MODE_MIN):
        outMat.ambient = glm::min(mat1Ambient, mat2Ambient);
        outMat.diffuse = glm::min(mat1Diffuse, mat2Diffuse);
        outMat.specular = glm::min(mat1Specular, mat2Specular);
        outMat.shininess = glm::min(mat1Shininess, mat2Shininess);
        break;
    }  

    outMat.diffuseTexture.textureData = m1D;
    outMat.diffuseTexture.textureWidth = outWidth;
    outMat.diffuseTexture.textureHeight = outHeight;
    
    outMat.normalTexture.textureData = m1N;
    outMat.normalTexture.textureWidth = outWidth;
    outMat.normalTexture.textureHeight = outHeight;

    outMat.roughnessTexture.textureData = m1R;
    outMat.roughnessTexture.textureWidth = outWidth;
    outMat.roughnessTexture.textureHeight = outHeight;

    outMat.metallicnessTexture.textureData = m1M;
    outMat.metallicnessTexture.textureWidth = outWidth;
    outMat.metallicnessTexture.textureHeight = outHeight;
    return outMat;

}

Material ProcManager::mixHeightmapMaterials(
    Material& mat1, float mat1MaxHeight,
    Material& mat2, float mat2MaxHeight,
    const char* outPath, unsigned int outWidth, unsigned int outHeight, float outMaxHeight,
    BlendInfo& info)
{

    
    unsigned int m1Channels = info.mat1Channels;
    unsigned int m2Channels = info.mat2Channels;
    unsigned int outChannels = 3;

    size_t m1RSize = outWidth * outHeight * info.mat1Channels;
    size_t m2RSize = outWidth * outHeight * info.mat2Channels;

    size_t heightmapSize = outWidth * outHeight * outChannels;

    // Create resized version of heightmap 1, if needed
    colcom* h1R;
    if (mat1.diffuseTexture.textureWidth == outWidth && mat1.diffuseTexture.textureHeight == outHeight) {
        h1R = mat1.diffuseTexture.textureData;
    }
    else {
        h1R = (colcom*)malloc(heightmapSize);
        if (h1R == nullptr) {
            Logger::Print("ProcManager.mixMaterials: Failed to allocate memory of size: " + std::to_string(heightmapSize) + " bytes", LOGCAT::CAT_LOADING, LOGSEV::SEV_ERROR);
            return Material();
        }
        stbir_resize_uint8_srgb(mat1.diffuseTexture.textureData, mat1.diffuseTexture.textureWidth, mat1.diffuseTexture.textureHeight, mat1.diffuseTexture.textureWidth * m1Channels,
            h1R, outWidth, outHeight, outWidth * outChannels,
            STBIR_RGB);
    }


    // Create resized version of heightmap 2, if needed
    colcom* h2R;
    if (mat2.diffuseTexture.textureWidth == outWidth && mat2.diffuseTexture.textureHeight == outHeight) {
        h2R = mat2.diffuseTexture.textureData;
    }
    else {
        h2R = (colcom*)malloc(heightmapSize);
        if (h2R == nullptr) {
            Logger::Print("ProcManager.mixMaterials: Failed to allocate memory of size: " + std::to_string(heightmapSize) + " bytes", LOGCAT::CAT_LOADING, LOGSEV::SEV_ERROR);
            return Material();
        }
        stbir_resize_uint8_srgb(mat2.diffuseTexture.textureData, mat2.diffuseTexture.textureWidth, mat2.diffuseTexture.textureHeight, mat2.diffuseTexture.textureWidth * m2Channels,
            h2R, outWidth, outHeight, outWidth * outChannels,
            STBIR_RGB);
    }


    colcom* hFinal;
    hFinal = (colcom*)malloc(heightmapSize);
    if (hFinal == nullptr) {
        Logger::Print("ProcManager.mixMaterials: Failed to allocate memory of size: " + std::to_string(heightmapSize) + " bytes", LOGCAT::CAT_LOADING, LOGSEV::SEV_ERROR);
        return Material();
    }

    // Now go through and add the two heightmaps
    int xOffsetPixels = info.xOffset * outWidth;
    int yOffsetPixels = info.yOffset * outHeight;

 
    for (int y = 0; y < outHeight; y++) {
        for (int x = 0; x < outWidth; x++) {
            int y1 = y;
            if (info.mat1Flip) {
                y1 = outHeight - 1 - y1;
            }
            int index = getIndex(x, y1, outWidth, outHeight, info.mat1Channels, false, true);// ((y1 * outWidth) + (x))* info.mat1Channels;
            colcom* pf = hFinal + index;

            int index1 = getIndex(x, y1, outWidth, outHeight, info.mat1Channels, false, true);
            colcom* p1 = h1R + index1;
            colcom r1, g1, b1;
            r1 = p1[0];
            g1 = p1[1];
            b1 = p1[2];
            float height1 = RGBToScalar(mat1MaxHeight, r1, g1, b1);
            height1 *= info.mat1Weight;

            int x2 = x + xOffsetPixels;
            int y2 = y + yOffsetPixels;
            // We are out of bounds due to offset, skip
            if (x2 < 0 || y2 < 0 || x2 >= outWidth || y2 >= outHeight) {
                pf[0] = p1[0];
                pf[1] = p1[1];
                pf[2] = p1[2];
                continue;
            }

            if (info.mat2Flip) {
                y2 = outHeight - 1 - y2;
            }

            int index2 = getIndex(x2, y2, outWidth, outHeight, info.mat2Channels, false, true); //(((y2)*outWidth) + (x2)) * info.mat2Channels;
            

            colcom* p2 = h2R + index2;
            colcom r2, g2, b2;
            r2 = p2[0];
            g2 = p2[1];
            b2 = p2[2];
            float height2 = RGBToScalar(mat2MaxHeight, r2, g2, b2);
            height2 *= info.mat2Weight;

            float finalHeight;
            switch (info.blendMode) {
            case BLENDMODE::MODE_ADD:
                finalHeight = height1 +height2;
                break;
            case BLENDMODE::MODE_SUB:
                finalHeight = height1 - height2;
                break;
            case BLENDMODE::MODE_MUL:
                finalHeight = height1 * height2;
                break;
            case BLENDMODE::MODE_MAX:
                finalHeight = std::fmaxf(height1,height2);
                break;
            case BLENDMODE::MODE_MIN:
                finalHeight = std::fminf(height1, height2);
                break;
            case BLENDMODE::MODE_MASK:
                if (height2 >= info.maskInfo.minVal && height2 <= info.maskInfo.maxVal) {
                    // Pass
                    finalHeight = height1;
                }
                else {
                    finalHeight = info.maskInfo.failVal;
                }
            }

            // Clamp
            if (info.clamp) {
                finalHeight = std::clamp(finalHeight, 0.0f, outMaxHeight);
            }


            colcom fr, fg, fb;
            scalarToRGB(finalHeight, outMaxHeight, &fr, &fg, &fb);

            
            pf[0] = fr;
            pf[1] = fg;
            pf[2] = fb;

        }
    }

    // TODO improve
    setWriteFlip(true);

    int res = stbi_write_png(outPath, outWidth, outHeight, outChannels, hFinal, outWidth * outChannels);
    if (!res) {
        Logger::Print("ProcManager.mixMaterials: Failed to write to output file", LOGCAT::CAT_LOADING, LOGSEV::SEV_ERROR);
    }

    // Clean up
    if (mat1.diffuseTexture.textureWidth != outWidth || mat1.diffuseTexture.textureHeight != outHeight) {
        free(h1R);
    }
    if (mat2.diffuseTexture.textureWidth != outWidth || mat2.diffuseTexture.textureHeight != outHeight) {
        free(h2R);
    }

    Material outMat;
    outMat.diffuseTexture.textureData = hFinal;
    outMat.diffuseTexture.textureWidth = outWidth;
    outMat.diffuseTexture.textureHeight = outHeight;

    return outMat;
}

/*
Mix two heightmaps together using specified weights
TODO add offsets, blend types
*/
colcom* ProcManager::mixHeightmaps(colcom* h1, unsigned int h1Width, unsigned int h1Height, float h1Weight, float h1MaxHeight,
    colcom* h2, unsigned int h2Width, unsigned int h2Height, float h2Weight, float h2MaxHeight,
    const char* outPath, unsigned int outWidth, unsigned int outHeight, float outMaxHeight)
{
    unsigned int channels = 3;


    size_t heightmapSize = outWidth * outHeight * channels;

    // Create resized version of heightmap 1, if needed
    colcom* h1R;
    if (h1Width == outWidth && h1Height == outHeight) {
        h1R = h1;
    }
    else {
        h1R = (colcom*)malloc(heightmapSize);
        if (h1R == nullptr) {
            Logger::Print("ProcManager.mixHeightmaps: Failed to allocate memory of size: " + std::to_string(heightmapSize) + " bytes", LOGCAT::CAT_LOADING, LOGSEV::SEV_ERROR);
            return nullptr;
        }
        stbir_resize_uint8_srgb(h1, h1Width, h1Height, h1Width * channels,
            h1R, outWidth, outHeight, outWidth * channels,
            STBIR_RGB);
    }


    // Create resized version of heightmap 2, if needed
    colcom* h2R;
    if (h2Width == outWidth && h2Height == outHeight) {
        h2R = h2;
    } else {
        h2R = (colcom*)malloc(heightmapSize);
        if (h2R == nullptr) {
            Logger::Print("ProcManager.mixHeightmaps: Failed to allocate memory of size: " + std::to_string(heightmapSize) + " bytes", LOGCAT::CAT_LOADING, LOGSEV::SEV_ERROR);
            return nullptr;
        }
        stbir_resize_uint8_srgb(h2, h2Width, h2Height, h2Width * channels,
            h2R, outWidth, outHeight, outWidth * channels,
            STBIR_RGB);
    }


    colcom* hFinal;
    hFinal = (colcom*)malloc(heightmapSize);
    if (hFinal == nullptr) {
        Logger::Print("ProcManager.mixHeightmaps: Failed to allocate memory of size: " + std::to_string(heightmapSize) + " bytes", LOGCAT::CAT_LOADING, LOGSEV::SEV_ERROR);
        return nullptr;
    }

    // Now go through and add the two heightmaps
    for (int y = 0; y < outHeight; y++) {
        for (int x = 0; x < outWidth; x++) {
            int index = getIndex(x, y, outWidth, outHeight, channels); //((y * outWidth) + (x)) * channels;

            colcom* p1 = h1R + index;
            colcom r1, g1, b1;
            r1 = p1[0];
            g1 = p1[1];
            b1 = p1[2];
            float height1 = RGBToScalar(h1MaxHeight, r1, g1, b1);

            colcom* p2 = h2R + index;
            colcom r2, g2, b2;
            r2 = p2[0];
            g2 = p2[1];
            b2 = p2[2];
            float height2 = RGBToScalar(h2MaxHeight, r2, g2, b2);

            float finalHeight = (height1 * h1Weight) + (height2 * h2Weight);

            if (finalHeight > outMaxHeight) {
                finalHeight = outMaxHeight;
            }

            colcom fr, fg, fb;
            scalarToRGB(finalHeight, outMaxHeight, &fr, &fg, &fb);

            colcom* pf = hFinal + index;
            pf[0] = fr;
            pf[1] = fg;
            pf[2] = fb;

        }
    }
    int res = stbi_write_png(outPath, outWidth, outHeight, channels, hFinal, outWidth * channels);
    if (!res) {
        Logger::Print("ProcManager.mixHeightmaps: Failed to write to output file", LOGCAT::CAT_LOADING, LOGSEV::SEV_ERROR);
    }

    // Clean up
    if (h1Width != outWidth || h1Height != outHeight) {
        free(h1R);
    }
    if (h2Width != outWidth || h2Height != outHeight) {
        free(h2R);
    }


    return hFinal;
}

    // Used by createMesh. Check if vertex i was generated or dropped
 bool generatedVertex(std::vector<unsigned int>& droppedVertices, unsigned int i) {
     if (i) {
         if (droppedVertices[i] - droppedVertices[i - 1]) {
             return false;
         }
         else {
             return true;
         }
     }
     else {
         if (droppedVertices[0]) {
             return false;
         }
         else {
             return true;
         }
     }
}

Mesh ProcManager::createMesh(const Material& mat, const MeshCreateInfo& createInfo)
{
    float sampleWidth = (float)mat.diffuseTexture.textureWidth / (float)createInfo.nPointsX;
    float sampleHeight = (float)mat.diffuseTexture.textureHeight / (float)createInfo.nPointsY;

    float pointWidth = createInfo.width / ((float)createInfo.nPointsX -1.0);
    float pointLength = createInfo.length / ((float)createInfo.nPointsY - 1.0);

    float pointTexWidth = 1.0 / (float)createInfo.nPointsX;
    float pointTexHeight = 1.0 / (float)createInfo.nPointsY;

    pointTexWidth /= createInfo.texScaleX;
    pointTexHeight /= createInfo.texScaleY;


    unsigned int channels = createInfo.channels;

    float heightRange = createInfo.maxHeight - createInfo.minHeight;

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    
    // For normals
    unsigned int normalChannels = 4;
    Material* normalMat = createInfo.normalMaterial;
    Texture& normalTex = normalMat->normalTexture;

    // For masking
    Material* maskMat = createInfo.maskMaterial;
    Texture& maskTex = maskMat->diffuseTexture;
    unsigned int maskChannels = createInfo.maskChannels;
    int mx, my; // sample pix for mask

    int x, y; //sample pix
    float px, pz; //pos
    float tx, ty; //tex

    int nx, ny; //sample pix for normal

    // How many vertices were not generated by vertex i
    unsigned int droppedVertexCount = 0;
    std::vector<unsigned int> droppedVertices{};

    // Create vertices
    unsigned int i = 0;//index

    // Sample info used when sampling the heightmap
    SampleInfo sampleInfo;
    sampleInfo.channels = channels;
    sampleInfo.filterMode = createInfo.filterType;
    sampleInfo.heightRange = heightRange;
    sampleInfo.sampleType = SampleType::SAMPLE_HEIGHT;
    sampleInfo.texWidth = mat.diffuseTexture.textureWidth;
    sampleInfo.texHeight = mat.diffuseTexture.textureHeight;

    // Sample info used when sampling the mask
    SampleInfo maskSampleInfo = sampleInfo;
    if (maskMat) {
        maskSampleInfo.texWidth = maskMat->diffuseTexture.textureWidth;
        maskSampleInfo.texHeight = maskMat->diffuseTexture.textureHeight;
        maskSampleInfo.heightRange = 1.0;
        maskSampleInfo.channels = maskChannels;
    }

    // Sample info used when sampling the normal texture
    SampleInfo normalSampleInfo = sampleInfo;
    if (normalMat) {
        normalSampleInfo.texWidth = normalMat->normalTexture.textureWidth;
        normalSampleInfo.texHeight = normalMat->normalTexture.textureHeight;
        normalSampleInfo.channels = normalChannels;
        normalSampleInfo.filterMode = SampleFilter::FILT_SAMPLE_NEAREST;
        normalSampleInfo.sampleType = SampleType::SAMPLE_NORMAL;
    }
    
    colcom r1, g1, b1, a1;
    float nsx = 0.0, nsy = 0.0;
    for (int sy = 0; sy < createInfo.nPointsY; sy++) {
        for (int sx = 0; sx < createInfo.nPointsX; sx++) {
            nsx = ((float)sx / (float)createInfo.nPointsX);
            nsy = ((float)sy / (float)createInfo.nPointsY);// +((sy + 1.0) / (float)createInfo.nPointsY);

            // Add one grid point of length over the course of the loop
            nsx -= (float)sx * (0.5 / (float)createInfo.nPointsX) / (float)createInfo.nPointsX;
            nsy -= (float)sy * (0.5 / (float)createInfo.nPointsY) / (float)createInfo.nPointsY;

            float pointHeight = sampleAt(mat.diffuseTexture.textureData, nsx, nsy, &r1, &g1, &b1, &a1, sampleInfo);

            // Check whether to do masking
            bool dropVertex = false;
            if (maskMat) {
                float maskHeight = sampleAt(maskTex.textureData, nsx, nsy, &r1, &g1, &b1, &a1, maskSampleInfo);

                if (maskHeight < createInfo.maskInfo.minVal || maskHeight > createInfo.maskInfo.maxVal) {
                    dropVertex = true;
                }
                else {
                    dropVertex = false;
                }
            }

            // Drop vertex if masking failed
            if (dropVertex) {
                droppedVertexCount++;
                Vertex vertex;
                vertex.Normal = { -0.0,-0.0,-0.0 }; // Impossible normal
                // Push back early
                vertices.push_back(vertex);
                droppedVertices.push_back(droppedVertexCount);
                continue;
            }



            droppedVertices.push_back(droppedVertexCount);

            // TODO linear filtering for normal
            // Sample Normal
            glm::vec3 normalVal;
            if (normalMat) {
                //TODO test
                
                sampleAt(normalTex.textureData, nsx / createInfo.normTexScaleX, nsy / createInfo.normTexScaleY, &r1, &g1, &b1, &a1, normalSampleInfo);
                normalVal = RGBToNormal(r1, g1, b1);

                if (pointHeight < 0.0) {
                    //normalVal *= -1.0;
                }
                //std::cout << "(" << nx << "," << ny << ") " << normalVal.x << ", " << normalVal.y << normalVal.z << " (" << glm::length(normalVal) << ")" << std::endl;
            }
            

            
            // Create point
            pointHeight += createInfo.minHeight;

            px = (float)(sx)*pointWidth;
            pz = (float)(sy) * pointLength;

            glm::vec3 point;
            point.x = createInfo.offset.x + px;
            point.z = createInfo.offset.z + pz;
            point.y = createInfo.offset.y;
            
            // Vertex-specific offset
            glm::vec3 vertexOffset{ 0.0, 1.0, 0.0 };
            if (normalMat) {
                vertexOffset = normalVal;
            }

            vertexOffset *= pointHeight;
            point += vertexOffset;
            

            // Create vertex
            float tOffsetX = 1.5;// (1.0 / (float)createInfo.nPointsX) * 30.0;
            float tOffsetY = 1.5;// (1.0 / (float)createInfo.nPointsY) * 30.0;
            tx = (float)(sx + 2.0 - tOffsetX)* pointTexWidth;
            ty = (float)((1.0 - sy) - tOffsetY)* pointTexHeight;

           /* tx += (float)sx* ((1.0 / pointTexWidth) / pointTexWidth);
            ty += (float)sy * ((1.0 / pointTexHeight) / pointTexHeight);*/

            Vertex vertex;


            vertex.Position = point;
            vertex.TexCoords = glm::vec2(tx, ty);

            glm::vec3 e1 = 
            vertex.Normal = glm::vec3(0.0, 1.0, 0.0);

            vertices.push_back(vertex);
            
            i++;
        }
    }

    // Rotate  all vertices if required
    if (createInfo.rotation != glm::mat3(1.0)) {
        glm::vec3 midPoint = glm::vec3(createInfo.width*0.5, createInfo.minHeight, createInfo.length*0.5) + createInfo.offset;
        for (elmt::Vertex& vertex : vertices) {
            glm::vec3 diffFromCenter = vertex.Position - midPoint;
            glm::vec3 rotatedPoint = createInfo.rotation * diffFromCenter;
            rotatedPoint += midPoint;
            
            vertex.Position = rotatedPoint;
        }
    }

    // Generate normals
    int il, ir, it, ib;
    int norms;
    glm::vec3 e1, e2, norm, normSum;
    for (int sy = 0; sy < createInfo.nPointsY; sy++) {
        for (int sx = 0; sx < createInfo.nPointsX; sx++) {

            norms = 0;
            normSum = glm::vec3(0.0, 0.0, 0.0);

            i = (sy * createInfo.nPointsX) + sx;
            if (!generatedVertex(droppedVertices, i)) {
                continue;
            }
            il = i - 1;
            ir = i + 1;
            it = i - createInfo.nPointsX;
            ib = i + createInfo.nPointsX;
            
            Vertex& vert = vertices[i];

            if (sx > 0 && sy > 0 && generatedVertex(droppedVertices, il) && generatedVertex(droppedVertices, it)) {
                //top, left
                e1 = vertices[il].Position - vert.Position;
                e2 = vertices[it].Position - vert.Position;
                norm = glm::normalize(glm::cross(e2, e1));
                normSum += norm;
                norms++;
            }

            if (sy > 1 && sx < createInfo.nPointsX - 1 && generatedVertex(droppedVertices, ir) && generatedVertex(droppedVertices, it)) {
                //top, right
                e1 = vertices[ir].Position - vert.Position;
                e2 = vertices[it].Position - vert.Position;
                norm = glm::normalize(glm::cross(e1, e2));
                normSum += norm;
                norms++;
            }
            
            if (sy < createInfo.nPointsY -1 && sx < createInfo.nPointsX - 1 && generatedVertex(droppedVertices, ib) && generatedVertex(droppedVertices, ir)) {
                //right, bottom
                e1 = vertices[ib].Position - vert.Position;
                e2 = vertices[ir].Position - vert.Position;
                norm = glm::normalize(glm::cross(e1, e2));
                normSum += norm;
                norms++;
            }
            
            if (sy < createInfo.nPointsY - 1 && sx > 0 && generatedVertex(droppedVertices, il) && generatedVertex(droppedVertices, ib)) {
                //bottom, left
                e1 = vertices[il].Position - vert.Position;
                e2 = vertices[ib].Position - vert.Position;
                norm = glm::normalize(glm::cross(e1, e2));
                normSum += norm;
                norms++;
            }
            
            auto len = glm::length(normSum);
            if (norms ) {
                normSum /= (float)norms;
                vert.Normal = normSum;
            }
            else {
                // No adjacent vertices, use default
                vert.Normal = { 0.0, 1.0, 0.0 };
            }
            
        }
    }

    // Create indices
    for (int iy = 0; iy < createInfo.nPointsY-1; iy++) {
        for (int ix = 0; ix < createInfo.nPointsX-1; ix++) {
            // Create two triangles
            i = (iy * createInfo.nPointsX) + ix;
            
            unsigned int tli = i; // Top left
            unsigned int bli = i + createInfo.nPointsX; // Bottom left
            unsigned int bri = i + createInfo.nPointsX + 1; // Bottom right
            unsigned int tri = i + 1; // Top Right
            
            bool topleft = false, bottomleft = false, topright = false, bottomright = false;
            if (generatedVertex(droppedVertices, tli) && generatedVertex(droppedVertices, tri) && generatedVertex(droppedVertices, bli)) {
                topleft = true;
            }

            if (generatedVertex(droppedVertices, tri) && generatedVertex(droppedVertices, bli) && generatedVertex(droppedVertices, bri)) {
                bottomright = true;
            }

            if (generatedVertex(droppedVertices, tli) && generatedVertex(droppedVertices, tri) && generatedVertex(droppedVertices, bri)) {
                topright = true;
            }

            if (generatedVertex(droppedVertices, tli) && generatedVertex(droppedVertices, bli) && generatedVertex(droppedVertices, bri)) {
                bottomleft = true;
            }

            // TODO why does this only seem to work on one side
            if (topleft || topright) {
                if (topleft) {
                    if (createInfo.windingOrder == WindingType::WIND_REVERSE || createInfo.windingOrder == WindingType::WIND_BOTH) {
                        indices.push_back(tri); //tr
                        indices.push_back(bli); //bl
                        indices.push_back(tli); //tl
                    }
                    if (createInfo.windingOrder == WindingType::WIND_NORMAL || createInfo.windingOrder == WindingType::WIND_BOTH) {
                        indices.push_back(tli); //tl
                        indices.push_back(bli); //bl
                        indices.push_back(tri); //tr
                    }
                }

                if (bottomright) {
                    if (createInfo.windingOrder == WindingType::WIND_REVERSE || createInfo.windingOrder == WindingType::WIND_BOTH) {
                        indices.push_back(tri); //tr
                        indices.push_back(bri); //br
                        indices.push_back(bli); //bl
                    }
                    if (createInfo.windingOrder == WindingType::WIND_NORMAL || createInfo.windingOrder == WindingType::WIND_BOTH) {
                        indices.push_back(bli); //bl
                        indices.push_back(bri); //br
                        indices.push_back(tri); //tr
                    }
                }
            }
            else {
                if (topright) {
                    if (createInfo.windingOrder == WindingType::WIND_REVERSE || createInfo.windingOrder == WindingType::WIND_BOTH) {
                        indices.push_back(tri); //tr
                        indices.push_back(bri); //br
                        indices.push_back(tli); //tl
                    }
                    if (createInfo.windingOrder == WindingType::WIND_NORMAL || createInfo.windingOrder == WindingType::WIND_BOTH) {
                        indices.push_back(tli); //tl
                        indices.push_back(bri); //br
                        indices.push_back(tri); //tr
                    }
                }

                if (bottomleft) {
                    if (createInfo.windingOrder == WindingType::WIND_REVERSE || createInfo.windingOrder == WindingType::WIND_BOTH) {
                        indices.push_back(tli); //tl
                        indices.push_back(bri); //br
                        indices.push_back(bli); //bl
                    }
                    if (createInfo.windingOrder == WindingType::WIND_NORMAL || createInfo.windingOrder == WindingType::WIND_BOTH) {
                        indices.push_back(bli); //bl
                        indices.push_back(bri); //br
                        indices.push_back(tli); //tl
                    }
                }
            }
        }
    }

    // Shift indices, vertices, 
    unsigned int si;
    i = 0;
    for (Vertex& v : vertices) {
        si = i - droppedVertices[i];
        if (generatedVertex(droppedVertices, i) && si != i) {
            vertices[si] = v;
        }
        i++;
    }
    vertices.resize(vertices.size() - droppedVertices[vertices.size()-1]);
    for (i = 0; i < indices.size(); i++) {
        indices[i] -= droppedVertices[indices[i] ];
    }

    Mesh mesh{ vertices,indices,createInfo.materialIndex };
    
    // This should be fine since the compiler shouldn't copy
    return mesh;
}

Model elmt::ProcManager::createModel(const Material& matHeight, const Material& matTexture, const MeshCreateInfo& createInfo)
{

    Mesh mesh = createMesh(matHeight, createInfo);

    std::vector<Mesh> meshes = std::vector<Mesh>{ mesh };


    std::vector<elmt::Material> materials = std::vector<Material>{ matTexture };

    return Model(meshes, materials);
}

// Used by spawnEntities
glm::uvec3 ProcManager::indexToCell(unsigned int index, CellSpawnInfo& info) {
    glm::uvec3 xyz;
    xyz.x = index % info.cellsX;
    xyz.y = (index / info.cellsX) % info.cellsY;
    xyz.z = index / (info.cellsX * info.cellsY);
    return xyz;
}

unsigned int ProcManager::cellToIndex(unsigned int cx, unsigned int cy, unsigned int cz, CellSpawnInfo& info) {
    unsigned int chosen = (cz * info.cellsY * info.cellsX) + (cy * info.cellsX) + cx;
    return chosen;
}

void ProcManager::setWriteFlip(bool flip)
{
    stbi_flip_vertically_on_write(flip);
    //stbi__flip_vertically_on_write;
    //stbi__set_flip_vertically_on_write(flip);
}

unsigned int ProcManager::getIndex(unsigned int px, unsigned int py, unsigned int width, unsigned int height, unsigned int channels, bool flipX, bool flipY)
{
    if (flipX) {
        px = width - px - 1;
    }

    if (flipY) {
        py = height - py - 1;
    }

    unsigned int ret = (( (py) * width) + (px) ) * channels;
    return ret;
}




float ProcManager::sampleAt(colcom* texData, float x, float y, colcom* r, colcom* g, colcom* b, colcom* a, const SampleInfo& info)
{



    unsigned int px, py;
    unsigned int texWidth = info.texWidth;
    unsigned int texHeight = info.texHeight;

    float sampleWidth = 1.0 / (float)texWidth;
    float sampleHeight = 1.0 / (float)texHeight;

    // Sample
    px = (unsigned int)(x * texWidth) % info.texWidth;
    py = (unsigned int)( (1.0-y) * texHeight) % info.texHeight;

    int index = getIndex(px, py, texWidth, texHeight, info.channels); //((py * texWidth) + (px))* info.channels;

    float returnVal = 0.0;

    if (info.filterMode != SampleFilter::FILT_SAMPLE_NEAREST) {
        // Sample point (within texel)
        float tx = fmodf(x * texWidth, 1.0);
        float ty = fmodf(y * texHeight, 1.0);

        // Center of texel
        float x1 = (float)px + (sampleWidth / 2.0);
        float y1 = (float)py + (sampleHeight / 2.0);

        // Sample other texels
        float dsx, dsy;
        if (tx < x1) {
            dsx = -1.0;
        }
        else {
            dsx = 1.0;
        }

        if (ty < y1) {
            dsy = -1.0;
        }
        else {
            dsy = 1.0;
        }

        // Indices into adjacement texels
        int i1, i2, i3, i4;
        i4 = -1;
        i1 = index;
        // Horizontal
        if ((x > 0 && dsx < 0) || (x < texWidth - 1 && dsx>0)) {
            i2 = index + ((int)dsx * info.channels);
        }
        else {
            i2 = i1;
            i4 = i1;
        }
        // Vertical
        if ((y > 0 && dsy < 0) || (y < texWidth - 1 && dsy>0)) {
            i3 = index + (dsy * (int)texWidth * info.channels);
        }
        else {
            i3 = i1;
            i4 = i1;
        }
        // Horizontal + Vertical
        if (i4 == -1) {
            i4 = i3 + ((int)dsx * info.channels);
        }

        // Sample
        colcom *p1, *p2, *p3, *p4;
        p1 = texData + i1;
        p2 = texData + i2;
        p3 = texData + i3;
        p4 = texData + i4;

        // Dist to sample point
        dsx *= sampleWidth;
        dsy *= sampleHeight;
        float du, dv;
        du = fabs(tx - 0.5);
        dv = fabs(ty - 0.5);


        //HEIGHT
        if (info.sampleType == SampleType::SAMPLE_HEIGHT) {
            float t1 = RGBToScalar(info.heightRange, p1[0], p1[1], p1[2]);
            float t2 = RGBToScalar(info.heightRange, p2[0], p2[1], p2[2]);
            float t3 = RGBToScalar(info.heightRange, p3[0], p3[1], p3[2]);
            float t4 = RGBToScalar(info.heightRange, p4[0], p4[1], p4[2]);

            if (info.filterMode == SampleFilter::FILT_SAMPLE_LINEAR) {
                // Blend linearly
                returnVal = ((du) * (dv)*t4) +
                    ((du) * (1.0 - dv) * t2) +
                    ((1.0 - du) * (dv)*t3) +
                    ((1.0 - du) * (1.0 - dv) * t1);
            }
            else if (info.filterMode == SampleFilter::FILT_SAMPLE_POOL_MAX) {
                returnVal = std::max({ t1,t2,t3,t4 });
            }
            else if (info.filterMode == SampleFilter::FILT_SAMPLE_POOL_MIN) {
                returnVal = std::min({ t1,t2,t3,t4 });
            }
        }
        else {
            for (int channel = 0; channel < info.channels; channel++) {
                float t1 = (float)p1[channel];
                float t2 = (float)p2[channel];
                float t3 = (float)p3[channel];
                float t4 = (float)p4[channel];
                float channelBlend = ((du) * (dv)*t4) +
                    ((du) * (1.0 - dv) * t2) +
                    ((1.0 - du) * (dv)*t3) +
                    ((1.0 - du) * (1.0 - dv) * t1);
                if (channelBlend > 255.0) {
                    Logger::Print("Linear filter resulted in component " + std::to_string(channel) + ", pos (" + std::to_string(px) + "," + std::to_string(py) + ") with val > 255 (" + std::to_string(channelBlend) + ")",
                        LOGCAT::CAT_LOADING | LOGCAT::CAT_CORE | LOGCAT::CAT_RENDERING, LOGSEV::SEV_WARNING);
                }

                colcom channelRes = (colcom)channelBlend;
                switch (channel) {
                case 0:
                    *r = channelRes;
                    break;
                case 1:
                    *g = channelRes;
                    break;
                case 2:
                    *b = channelRes;
                    break;
                case 3:
                    *a = channelRes;
                    break;
                }
            }
        }
        

    }
    else {
        colcom* p1 = texData + index;
        for (int channel = 0; channel < info.channels; channel++) {
            switch (channel) {
            case 0:
                *r = p1[channel];
                break;
            case 1:
                *g = p1[channel];
                break;
            case 2:
                *b = p1[channel];
                break;
            case 3:
                *a = p1[channel];
                break;
            }
            
        }

        
        // HEIGHT
        if (info.sampleType == SampleType::SAMPLE_HEIGHT) {
            returnVal = RGBToScalar(info.heightRange, *r, *g, *b);
        }
        
    }

    return returnVal;
}