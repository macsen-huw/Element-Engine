#include "AssetManager.hpp"

#include <cassert>
#include <string>
#include "Logger.hpp"
#include "LogType.hpp"

#define STB_IMAGE_STATIC
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Log.hpp"
#endif

using namespace elmt;

#define ASSIMP_LOAD_FLAGS (aiProcess_Triangulate | aiProcess_GenNormals |  aiProcess_JoinIdenticalVertices )

// generic function to load model file
// returns { mesh data, material data, skeleton data, animation data }
Model* AssetManager::importAsset(const std::string &filePath, const std::string &fileName, bool mirror) {


	Assimp::Importer importer;
	const aiScene* aiScene = importer.ReadFile(filePath + fileName, ASSIMP_LOAD_FLAGS);

	Model* model = nullptr;
	if (aiScene) {

		std::tuple< std::vector<Mesh>, SkeletonRig> meshData;
		if (aiScene->HasMeshes()) meshData = convertMesh(aiScene);

		std::vector<Material> materialData;
		if (aiScene->HasMaterials()) materialData = convertMaterial(aiScene, filePath.c_str());
        
        model = new Model(std::get<0>(meshData), std::get<1>(meshData), materialData, mirror);
		return model;
	}

	else {
        Logger::Print("[Error/AssetManager::import]: Unable to load file:" + filePath + fileName, LOGCAT::CAT_LOADING, LOGSEV::SEV_ERROR);
		return model;
	}
}

Animation AssetManager::importAnimation(const std::string& filePath, const std::string& fileName) {
	Assimp::Importer importer;
	const aiScene* aiScene = importer.ReadFile(filePath + fileName, ASSIMP_LOAD_FLAGS);

	Animation animation;

	if (aiScene) {
		animation.skeletonRig = loadSkeletonRig(aiScene);

		if (aiScene->HasAnimations()) {
			animation.animDuration = aiScene->mAnimations[0]->mDuration;
			animation.ticsPerSecond = aiScene->mAnimations[0]->mTicksPerSecond;

			readHeirarchyData(animation.assimpNodeData, aiScene->mRootNode);

			for (int i = 0; i < aiScene->mAnimations[0]->mNumChannels; i++) {
				auto channel = aiScene->mAnimations[0]->mChannels[i];

				animation.bones.emplace_back(AnimBone(
					channel->mNodeName.data, 
					animation.skeletonRig.boneMap[channel->mNodeName.data].boneId,
					channel
				));
			}

			return animation;
		}
		else {
			Logger::Print("[Warning/AssetManager::import]: No animations found in: " + filePath + fileName, LOGCAT::CAT_LOADING, LOGSEV::SEV_WARNING);
		}
	}
	else {
        Logger::Print("[Error/AssetManager::import]: unable to load file: " + filePath + fileName, LOGCAT::CAT_LOADING, LOGSEV::SEV_ERROR);
	}
}

// mesh data methods
std::tuple<std::vector<Mesh>, SkeletonRig> AssetManager::importMesh(const std::string& filePath, const std::string& fileName) {
	Assimp::Importer importer;
	const aiScene* aiScene = importer.ReadFile(filePath + fileName, ASSIMP_LOAD_FLAGS);

	if (aiScene) {
		std::tuple<std::vector<Mesh>, SkeletonRig> meshData;
		if (aiScene->HasMeshes())
			meshData = convertMesh(aiScene);

		return meshData;
	}
	else {
        Logger::Print("[Error/AssetManager::import]: Unable to load file:" + filePath + fileName, LOGCAT::CAT_LOADING, LOGSEV::SEV_ERROR);
	}
}

Material AssetManager::createMaterial(CreateMaterialInfo createMaterialInfo) {
	Material material;
	material.diffuse = createMaterialInfo.diffuse;
	material.ambient = createMaterialInfo.ambient;
	material.emissive = createMaterialInfo.emissive;
	material.specular = createMaterialInfo.specular;
	material.shininess = createMaterialInfo.shininess;
	material.translucent = createMaterialInfo.translucent;

	if (createMaterialInfo.diffuseTexture) {
		auto [width, height, diffuseTexture] = loadTexture(createMaterialInfo.diffuseTexture, createMaterialInfo);

		material.diffuseTexture.texturePath = createMaterialInfo.diffuseTexture;
		material.diffuseTexture.textureData = diffuseTexture;
		material.diffuseTexture.textureWidth = width;
		material.diffuseTexture.textureHeight = height;
	}
	if (createMaterialInfo.normalTexture) {
		auto [width, height, normalTexture] = loadTexture(createMaterialInfo.normalTexture, createMaterialInfo);

		material.normalTexture.texturePath = createMaterialInfo.normalTexture;
		material.normalTexture.textureData = normalTexture;
		material.normalTexture.textureWidth = width;
		material.normalTexture.textureHeight = height;
	}
	if (createMaterialInfo.roughnessTexture) {
		auto [width, height, roughnessTexture] = loadTexture(createMaterialInfo.roughnessTexture, createMaterialInfo);

		material.roughnessTexture.texturePath = createMaterialInfo.roughnessTexture;
		material.roughnessTexture.textureData = roughnessTexture;
		material.roughnessTexture.textureWidth = width;
		material.roughnessTexture.textureHeight = height;
	}
	if (createMaterialInfo.metallicnessTexture) {
		auto [width, height, metallicnessTexture] = loadTexture(createMaterialInfo.metallicnessTexture, createMaterialInfo);

		material.metallicnessTexture.texturePath = createMaterialInfo.metallicnessTexture;
		material.metallicnessTexture.textureData = metallicnessTexture;
		material.metallicnessTexture.textureWidth = width;
		material.metallicnessTexture.textureHeight = height;
	}

	return material;
}


Material AssetManager::createMaterialDiffuse(const char* texturePath, unsigned int channels, bool flipVertically) {
	CreateMaterialInfo info;
	info.specular = glm::vec3(1.0, 1.0, 1.0);
	info.diffuseTexture = texturePath;
	info.channels = channels;
	info.flipVertically = flipVertically;

	return createMaterial(info);
}

std::tuple<std::vector<Mesh>, SkeletonRig> AssetManager::convertMesh(const aiScene *scene) {
	std::vector<Mesh> meshData;
	SkeletonRig skeletonRig;
	processMeshNode(scene->mRootNode, scene, &meshData, skeletonRig);

	return { meshData, skeletonRig };
}

void AssetManager::processMeshNode(aiNode* node, const aiScene* scene, std::vector<Mesh>* meshData, SkeletonRig& skeletonRig) {
	// process all the node's meshes (if any)
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshData->emplace_back(processMesh(mesh, scene, skeletonRig));
	}
	// then do the same for each of its children
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processMeshNode(node->mChildren[i], scene, meshData, skeletonRig);
	}
}

Mesh AssetManager::processMesh(aiMesh* mesh, const aiScene* scene, SkeletonRig& skeletonRig) {
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	std::vector<int> boneIds;
	std::vector<float> weights;

	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		
		// process vertex positions, normals and texture coordinates
		glm::vec3 vector;

		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.Position = vector;

		vector.x = mesh->mNormals[i].x;
		vector.y = mesh->mNormals[i].y;
		vector.z = mesh->mNormals[i].z;
		vertex.Normal = glm::normalize(vector);

		// add texture coordinate data
		if (mesh->mTextureCoords[0])
		{
			glm::vec2 vec;
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.TexCoords = vec;
		}
		else
			vertex.TexCoords = glm::vec2(0.0f, 0.0f);

		vertices.push_back(vertex);
	}

	// process indices
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	if (mesh->HasBones()) {
		// add skeleton data
		readSkeletonRigData(skeletonRig, mesh);
		
		// add vertex bone data
		for (int boneIndex = 0; boneIndex < mesh->mNumBones; boneIndex++) {

			std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
			int boneId = skeletonRig.boneMap[boneName].boneId;			

			assert(boneId != -1);
			auto weights = mesh->mBones[boneIndex]->mWeights;

			for (int weightIndex = 0; weightIndex < mesh->mBones[boneIndex]->mNumWeights; weightIndex++)
			{
				int vertexId = weights[weightIndex].mVertexId;
				float weight = weights[weightIndex].mWeight;
				assert(vertexId <= vertices.size());

				for (int i = 0; i < MAX_BONE_VERTEX_INFLUENCE; i++)
				{
					if (vertices[vertexId].BoneIds[i] == -1)
					{
						vertices[vertexId].Weights[i] = weight;
						vertices[vertexId].BoneIds[i] = boneId;
						break;
					}
				}
			}
		}
	}

	return Mesh(vertices, indices, mesh->mMaterialIndex);
}


// skeleton data methods
SkeletonRig AssetManager::loadSkeletonRig(const aiScene* scene) {
	SkeletonRig skeletonRig;

	processSkeletonNode(scene->mRootNode, scene, skeletonRig);

	return skeletonRig;
}

void AssetManager::processSkeletonNode(aiNode* node, const aiScene* scene, SkeletonRig& skeletonRig) {
	// process all the node's meshes (if any)
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		readSkeletonRigData(skeletonRig, mesh);
	}
	// then do the same for each of its children
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processSkeletonNode(node->mChildren[i], scene, skeletonRig);
	}
}

void AssetManager::readSkeletonRigData(SkeletonRig& skeletonRig, aiMesh* mesh) {
	for (int boneIndex = 0; boneIndex < mesh->mNumBones; boneIndex++) {

		std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();

		// add new bone
		if (skeletonRig.boneMap.find(boneName) == skeletonRig.boneMap.end()) {
			Bone newBone;
			newBone.boneId = skeletonRig.numBones;
			newBone.offsetMatrix = ConvertMatrixToGLMFormat(mesh->mBones[boneIndex]->mOffsetMatrix);

			skeletonRig.boneMap[boneName] = newBone;

			skeletonRig.numBones++;
		}

	}
}

// material data methods
std::vector<Material> AssetManager::convertMaterial(const aiScene* scene, const char* filePath) {
    	std::vector<Material> materialData;

	for (int i = 0; i < scene->mNumMaterials; i++) {
		Material material{};

        aiString materialName;
        scene->mMaterials[i]->Get(AI_MATKEY_NAME, materialName);
        material.materialName = std::string(materialName.data);

		aiString texturePath;
		if (AI_SUCCESS == scene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath)) {

            material.diffuseTexture.texturePath = texturePath.data;

			const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(texturePath.C_Str());

			if (embeddedTexture) {
				auto [width, height, diffuseTexture] = loadEmbeddedTexture(embeddedTexture);
				material.diffuseTexture.textureData = diffuseTexture;
				material.diffuseTexture.textureWidth = width;
				material.diffuseTexture.textureHeight = height;
			}
			else {
				auto [width, height, diffuseTexture] = loadTexture((std::string(filePath) + std::string(texturePath.data)).c_str());
				material.diffuseTexture.textureData = diffuseTexture;
				material.diffuseTexture.textureWidth = width;
				material.diffuseTexture.textureHeight = height;
			}
            if (material.diffuseTexture.textureWidth == 0 || material.diffuseTexture.textureHeight == 0){
                Logger::Print("Error, texture loaded from file: " + std::string(texturePath.data) + "has " +
                              std::string(material.diffuseTexture.textureWidth ? "height" : "width") + " equal to 0", LOGCAT::CAT_LOADING,
                              LOGSEV::SEV_ERROR);
                material.diffuseTexture.textureData = nullptr;
                material.diffuseTexture.textureWidth = 0;
                material.diffuseTexture.textureHeight = 0;
            }
        }


        aiString normalTexturePath;
        if (AI_SUCCESS == scene->mMaterials[i]->GetTexture(aiTextureType_NORMALS, 0, &normalTexturePath)) {

            material.normalTexture.texturePath = normalTexturePath.data;

            const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(normalTexturePath.C_Str());

            if (embeddedTexture) {
                auto [width, height, normalTexture] = loadEmbeddedTexture(embeddedTexture);
                material.normalTexture.textureData = normalTexture;
                material.normalTexture.textureWidth = width;
                material.normalTexture.textureHeight = height;
            }
            else {
                auto [width, height, normalTexture] = loadTexture((std::string(filePath) + std::string(normalTexturePath.data)).c_str());
                material.normalTexture.textureData = normalTexture;
                material.normalTexture.textureWidth = width;
                material.normalTexture.textureHeight = height;
            }

            if (material.normalTexture.textureWidth == 0 || material.normalTexture.textureHeight == 0){
                Logger::Print("Error, texture loaded from file: " + std::string(normalTexturePath.data) + "has " +
                              std::string(material.normalTexture.textureWidth ? "height" : "width") + " equal to 0\n", LOGCAT::CAT_LOADING,
                              LOGSEV::SEV_ERROR);

                material.normalTexture.textureData = nullptr;
                material.normalTexture.textureWidth = 0;
                material.normalTexture.textureHeight = 0;
            }

        }

        //obj doesnt have an ao texture, so treat the ambient texture as an ambient occlusion texture
        aiString ambientMapTexturePath;
        if (AI_SUCCESS == scene->mMaterials[i]->GetTexture(aiTextureType_AMBIENT, 0, &ambientMapTexturePath)) {

            material.ambientOcclusionTexture.texturePath = ambientMapTexturePath.data;

            const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(ambientMapTexturePath.C_Str());

            if (embeddedTexture) {
                auto [width, height, ambientOcclusionTexture] = loadEmbeddedTexture(embeddedTexture);
                material.ambientOcclusionTexture.textureData = ambientOcclusionTexture;
                material.ambientOcclusionTexture.textureWidth = width;
                material.ambientOcclusionTexture.textureHeight = height;
            }
            else {
                auto [width, height, ambientOcclusionTexture] = loadTexture((std::string(filePath) + std::string(ambientMapTexturePath.data)).c_str());
                material.ambientOcclusionTexture.textureData = ambientOcclusionTexture;
                material.ambientOcclusionTexture.textureWidth = width;
                material.ambientOcclusionTexture.textureHeight = height;
            }

            if (material.ambientOcclusionTexture.textureWidth == 0 || material.ambientOcclusionTexture.textureHeight == 0){
                Logger::Print("Error, texture loaded from file: " + std::string(ambientMapTexturePath.data) + "has " +
                              std::string(material.ambientOcclusionTexture.textureWidth ? "height" : "width") + " equal to 0", LOGCAT::CAT_LOADING,
                              LOGSEV::SEV_ERROR);

                material.ambientOcclusionTexture.textureData = nullptr;
                material.ambientOcclusionTexture.textureWidth = 0;
                material.ambientOcclusionTexture.textureWidth = 0;
            }
        }



        //because obj doesnt support parallax maps (old ass file format) we pretend that a displacement map is a parallax map
        //we need a more robust way of dealing with this in the future (maybe just assume all obj files dont have a parallax map?)
        //but testing with obj is more convenient that fbx
        aiString heightMapTexturePath;
        if (AI_SUCCESS == scene->mMaterials[i]->GetTexture(aiTextureType_DISPLACEMENT, 0, &heightMapTexturePath)) {
            const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(heightMapTexturePath.C_Str());

            material.parallaxTexture.texturePath = heightMapTexturePath.data;

            if (embeddedTexture) {
                auto [width, height, parallaxTexture] = loadEmbeddedTexture(embeddedTexture);
                material.parallaxTexture.textureData = parallaxTexture;
                material.parallaxTexture.textureWidth = width;
                material.parallaxTexture.textureHeight = height;
            }
            else {
                auto [width, height, parallaxTexture] = loadTexture((std::string(filePath) + std::string(heightMapTexturePath.data)).c_str());
                material.parallaxTexture.textureData = parallaxTexture;
                material.parallaxTexture.textureWidth = width;
                material.parallaxTexture.textureHeight = height;
            }
            if (material.parallaxTexture.textureWidth == 0 || material.parallaxTexture.textureHeight == 0){
                Logger::Print("Error, texture loaded from file: " + std::string(heightMapTexturePath.data) + "has " +
                              std::string(material.parallaxTexture.textureWidth ? "height" : "width") + " equal to 0", LOGCAT::CAT_LOADING,
                              LOGSEV::SEV_ERROR);

                material.parallaxTexture.textureData = nullptr;
                material.parallaxTexture.textureWidth = 0;
                material.parallaxTexture.textureHeight = 0;
            }

        }



        aiString roughnessTexturePath;
        if (AI_SUCCESS == scene->mMaterials[i]->GetTexture(aiTextureType_SHININESS, 0, &roughnessTexturePath)) {
            const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(normalTexturePath.C_Str());

            material.roughnessTexture.texturePath = roughnessTexturePath.data;

            if (embeddedTexture) {
                auto [width, height, roughnessTexture] = loadEmbeddedTexture(embeddedTexture);
                material.roughnessTexture.textureData = roughnessTexture;
                material.roughnessTexture.textureWidth = width;
                material.roughnessTexture.textureHeight = height;
            }
            else {
                auto [width, height, roughnessTexture] = loadTexture((std::string(filePath) + std::string(roughnessTexturePath.data)).c_str());
                material.roughnessTexture.textureData = roughnessTexture;
                material.roughnessTexture.textureWidth = width;
                material.roughnessTexture.textureHeight = height;
            }
            if (material.roughnessTexture.textureWidth == 0 || material.roughnessTexture.textureHeight == 0){
                Logger::Print("Error, texture loaded from file: " + std::string(roughnessTexturePath.data) + "has " +
                              std::string(material.roughnessTexture.textureWidth ? "height" : "width") + " equal to 0", LOGCAT::CAT_LOADING,
                              LOGSEV::SEV_ERROR);

                material.roughnessTexture.textureData = nullptr;
                material.roughnessTexture.textureWidth = 0;
                material.roughnessTexture.textureHeight = 0;
            }
        }


        aiString metallicnessTexturePath;
        if (AI_SUCCESS == scene->mMaterials[i]->GetTexture(AI_MATKEY_METALLIC_TEXTURE, &metallicnessTexturePath)) {
            const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(normalTexturePath.C_Str());

            material.metallicnessTexture.texturePath = metallicnessTexturePath.data;

            if (embeddedTexture) {
                auto [width, height, metallicnessTexture] = loadEmbeddedTexture(embeddedTexture);
                material.metallicnessTexture.textureData = metallicnessTexture;
                material.metallicnessTexture.textureWidth = width;
                material.metallicnessTexture.textureHeight = height;
            }
            else {
                auto [width, height, metallicnessTexture] = loadTexture((std::string(filePath) + std::string(metallicnessTexturePath.data)).c_str());
                material.metallicnessTexture.textureData = metallicnessTexture;
                material.metallicnessTexture.textureWidth = width;
                material.metallicnessTexture.textureHeight = height;
            }

            if (material.metallicnessTexture.textureWidth == 0 || material.metallicnessTexture.textureHeight == 0){
                Logger::Print("Error, texture loaded from file: " + std::string(metallicnessTexturePath.data) + "has " +
                              std::string(material.metallicnessTexture.textureWidth ? "height" : "width") + " equal to 0", LOGCAT::CAT_LOADING,
                              LOGSEV::SEV_ERROR);

                material.metallicnessTexture.textureData = nullptr;
                material.metallicnessTexture.textureWidth = 0;
                material.metallicnessTexture.textureHeight = 0;
            }
        }




        aiColor4D diffuse;
		if (AI_SUCCESS == aiGetMaterialColor(scene->mMaterials[i], AI_MATKEY_COLOR_DIFFUSE, &diffuse))
			material.diffuse = glm::vec3(diffuse.r, diffuse.g, diffuse.b);
        else material.diffuse = glm::vec3(0.5, 0.5, 0.5);

		aiColor4D specular;
		if (AI_SUCCESS == aiGetMaterialColor(scene->mMaterials[i], AI_MATKEY_COLOR_SPECULAR, &specular))
			material.specular = glm::vec3(specular.r, specular.g, specular.b);
        else material.specular = glm::vec3(0.1, 0.1, 0.1);

		aiColor4D ambient;
		if (AI_SUCCESS == aiGetMaterialColor(scene->mMaterials[i], AI_MATKEY_COLOR_AMBIENT, &ambient))
			material.ambient = glm::vec3(ambient.r, ambient.g, ambient.b);
        else material.ambient = glm::vec3(0.1, 0.1, 0.1);


        aiColor4D emissive;
		if (AI_SUCCESS == aiGetMaterialColor(scene->mMaterials[i], AI_MATKEY_COLOR_EMISSIVE, &emissive))
			material.emissive = glm::vec3(emissive.r, emissive.g, emissive.b);
        else material.emissive = glm::vec3(0, 0, 0);


        float shininess = 0;
		if (AI_SUCCESS == aiGetMaterialFloat(scene->mMaterials[i], AI_MATKEY_SHININESS, &shininess))
			material.shininess = shininess;
        else material.shininess = 1;


        materialData.emplace_back(material);
	}

	return materialData;
}


std::tuple<int, int, uint8_t*> AssetManager::loadTexture(const char* texturePath, const CreateMaterialInfo& info ) {
	int width, height, n;
	stbi_set_flip_vertically_on_load(info.flipVertically);
	uint8_t* textureData = stbi_load(texturePath, &width, &height, &n, info.channels);

	//if texture successfully loaded, add it to the material
	if (textureData != nullptr) {
		return { width, height, textureData };
	}
	else {
		Logger::Print("Error - unable to load texture: " + std::string(texturePath), LOGCAT::CAT_LOADING, LOGSEV::SEV_ERROR);
		return { 0, 0, nullptr };
	}
}

std::tuple<int, int, uint8_t*> AssetManager::loadTexture(const char* texturePath) {
	CreateMaterialInfo defaultInfo;
	return loadTexture(texturePath, defaultInfo);
}

std::tuple<int, int, uint8_t*> AssetManager::loadEmbeddedTexture(const aiTexture* aiTexture) {
	int width, height, n;

	stbi_set_flip_vertically_on_load(true);

	uint8_t* textureData = stbi_load_from_memory((const stbi_uc*)aiTexture->pcData, aiTexture->mWidth, &width, &height, &n, 4);

	//if texture successfully loaded, add it to the material
	if (textureData != nullptr) {
		return { width, height, textureData };
	}
	else {
		return { 0, 0, nullptr };
	}
}

// animation data methods
void AssetManager::readHeirarchyData(Animation::NodeData& nodeData, const aiNode* node) {
	assert(node);
	nodeData.name = node->mName.data;
	nodeData.transformation = ConvertMatrixToGLMFormat(node->mTransformation);
	nodeData.childrenCount = node->mNumChildren;


	for (int i = 0; i < node->mNumChildren; i++)
	{
		Animation::NodeData newData;
		readHeirarchyData(newData, node->mChildren[i]);
		nodeData.children.push_back(newData);
	}
}

// form https://learnopengl.com/
glm::mat4 AssetManager::ConvertMatrixToGLMFormat(const aiMatrix4x4& from)
{
	glm::mat4 to;
	//the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
	to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
	to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
	to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
	to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
	return to;
}

AssetManager::~AssetManager() {

}
