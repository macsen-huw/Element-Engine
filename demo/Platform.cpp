#include "Platform.hpp"

#include "../external/glm-0.9.7.1/glm/gtc/matrix_transform.hpp"
#include "../external/glm-0.9.7.1/glm/gtx/euler_angles.hpp"



Platform::Platform(glm::vec3 pos, elmt::Model* model) : GameObject(generateName().c_str(), g::platformsBase, pos, model, false)
{

}

Platform::Platform(glm::vec3 pos) : Platform(pos, chooseModel())
{

}

elmt::Model* Platform::generateModel(std::string maskBaseName, std::string texName, unsigned int seed)
{

    const unsigned int genSize = 64;
    const unsigned int meshRes = 64;

    const float platformWidth = 10, platformHeight = 7, platformLength = 10;

    auto rockM = getPlatformTex(texName);

    elmt::Material topH, bottomH, topMaH;

    

    if (g::GENERATE_TERRAIN) {
        auto topM = g::assMan->createMaterialDiffuse((data_path + maskBaseName + "Mult.png").c_str());
        auto topMa = g::assMan->createMaterialDiffuse((data_path + maskBaseName + "Mask.png").c_str());

        // Generate top heightmap
        elmt::perlinInfo genInfoTop_HF;

        auto topHF = g::procMan->generateHeightmap((data_gen_path + maskBaseName + "topHF.png").c_str(), genSize, genSize, elmt::ProcManager::perlinHeightFunction, seed + 0, &genInfoTop_HF);
        // Load mult
        elmt::materialHeightInfo genInfoTop_MH;
        genInfoTop_MH.material = &topM;
        auto topMH = g::procMan->generateHeightmap((data_gen_path + maskBaseName + "topMH.png").c_str(), genSize, genSize, elmt::ProcManager::materialHeightFunction, 0, &genInfoTop_MH);
        // Multiply
        elmt::BlendInfo blendInfoTop_H;
        blendInfoTop_H.mat1Weight = 1.0;
        blendInfoTop_H.mat2Weight = 1.0;
        blendInfoTop_H.blendMode = elmt::BLENDMODE::MODE_MUL;
        blendInfoTop_H.mat1Channels = 3;
        blendInfoTop_H.mat2Channels = 3;
        topH = g::procMan->mixHeightmapMaterials(topHF, 1.0, topMH, 1.0, (data_gen_path + maskBaseName + "topH.png").c_str(), genSize, genSize, 1.0, blendInfoTop_H);

        // Generate bottom heightmap
        elmt::perlinInfo genInfoBottom_HF = genInfoTop_HF;
        genInfoBottom_HF.xScale = 10.0;
        genInfoBottom_HF.yScale = 10.0;
        auto bottomHF = g::procMan->generateHeightmap((data_gen_path + maskBaseName + "bottomHF.png").c_str(), genSize, genSize, elmt::ProcManager::perlinHeightFunction, seed + 1, &genInfoBottom_HF);
        bottomH = g::procMan->mixHeightmapMaterials(bottomHF, 1.0, topMH, 1.0, (data_gen_path + maskBaseName + "bottomH.png").c_str(), genSize, genSize, 1.0, blendInfoTop_H);

        // Create Mesh Mask
        elmt::materialHeightInfo genInfoTop_MaH;
        genInfoTop_MaH.material = &topMa;
        topMaH = g::procMan->generateHeightmap((data_gen_path + maskBaseName + "topMaH.png").c_str(), genSize, genSize, elmt::ProcManager::materialHeightFunction, 0, &genInfoTop_MaH);


    } else {
        topH = g::assMan->createMaterialDiffuse((data_gen_path + maskBaseName + "topH.png").c_str());
        bottomH = g::assMan->createMaterialDiffuse((data_gen_path + maskBaseName + "bottomH.png").c_str());
        bottomH = g::assMan->createMaterialDiffuse((data_gen_path + maskBaseName + "topMaH.png").c_str());
    }
    


    // Create Mesh
    elmt::ProcManager::MeshCreateInfo topCI{
        glm::vec3(0.0, 0.0, 0.0),
        platformWidth, platformLength, // dimensions
        meshRes, meshRes, // cells
        0.0, platformHeight*0.25, // height range
        0, // material index
        elmt::ProcManager::SampleFilter::FILT_SAMPLE_LINEAR, // filtering
        3, // channels
        0.2, 0.2 // tex scale
        //glm::mat3(glm::eulerAngleXYZ(glm::pi<float>() * -0.25, 0.0, 0.0)) // rotation
    };
    topCI.maskMaterial = &topMaH;
    topCI.maskChannels = 3;
    auto topMesh1 = g::procMan->createMesh(topH, topCI);

    // Generate bottom of platform

    elmt::ProcManager::MeshCreateInfo bottomCI = topCI;
    bottomCI.maxHeight =  -platformHeight*0.75;
    bottomCI.windingOrder = elmt::ProcManager::WindingType::WIND_REVERSE;
    bottomCI.maskInfo.minVal = 0.25;
    bottomCI.maskInfo.maxVal = 1.0;

    auto bottomMesh1 = g::procMan->createMesh(bottomH, bottomCI);

    std::vector<elmt::Mesh> meshes = { bottomMesh1, topMesh1 };
    std::vector<elmt::Material> mats = { rockM };
    elmt::Model* model = new elmt::Model(meshes, mats, false);

    return model;
}

std::string Platform::generateName()
{
    return "Random Platform " + g::getObjectID();
}

elmt::Model* Platform::chooseModel()
{
    if (models.size() ) {
        auto i = rand() % models.size();
        return models[i];
    }
    else {
        elmt::Logger::Print("No models for platform to choose from", elmt::LOGCAT::CAT_LOADING, elmt::LOGSEV::SEV_ERROR);
        return nullptr;
    }
    
    
}

elmt::Material& Platform::getPlatformTex(std::string texName)
{
    if (texCache.find(texName) == texCache.end()) {
        auto newMat = g::assMan->createMaterialDiffuse((data_path + texName).c_str());
        texCache.insert({ texName, newMat });
    }


    return texCache[texName];
}
