#pragma once
#include <tuple>

//#include "assimp/assimp/include/assimp/Importer.hpp"
//#include "assimp/assimp/include/assimp/scene.h"
//#include "assimp/assimp/include/assimp/postprocess.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "core.hpp"
#include "RenderManager.hpp"
#include "Mesh.hpp"
#include "SkeletonRig.hpp"
#include "Material.hpp"
#include "Model.hpp"
#include "Animation.hpp"

namespace elmt {
	struct CreateMaterialInfo {
		glm::vec3 diffuse = glm::vec3(1, 1, 1);
		glm::vec3 specular = glm::vec3(1, 1, 1);
		glm::vec3 emissive = glm::vec3(0, 0, 0);
		glm::vec3 ambient = glm::vec3(1, 1, 1);

		float shininess = 1.0;
		bool translucent = false;

		const char* diffuseTexture = nullptr;
		const char* normalTexture = nullptr;
		const char* roughnessTexture = nullptr;
		const char* metallicnessTexture = nullptr;

		unsigned int channels = 4;
		bool flipVertically = true;
	};

	class AssetManager
	{
#define ARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))
	public:
		AssetManager(RenderManager* renderManager) {
			this->renderManager = renderManager;
		}

		Model* importAsset(const std::string& filePath, const std::string & fileName, bool mirror = false);

		std::tuple<std::vector<Mesh>, SkeletonRig> importMesh(const std::string& filePath, const std::string& fileName);

		Material createMaterial(CreateMaterialInfo createMaterialInfo);

		// Create a simple, diffuse only material
		Material createMaterialDiffuse(const char* texturePath, unsigned int channels, bool flipVertically);
		Material createMaterialDiffuse(const char* texturePath) { return createMaterialDiffuse(texturePath, 4, true); };

		Animation importAnimation(const std::string& filePath, const std::string& fileName);

        ~AssetManager();

	private:
		// Source: https://learnopengl.com/Model-Loading/Assimp
		std::tuple<std::vector<Mesh>, SkeletonRig> convertMesh(const aiScene *scene);
		void processMeshNode(aiNode* node, const aiScene* scene, std::vector<Mesh>* meshData, SkeletonRig& skeletonRig);
		Mesh processMesh(aiMesh* mesh, const aiScene* scene, SkeletonRig& skeletonRig);
        std::vector<Material> importMaterial(const aiScene* scene, const char* filePath);

		std::vector<Material> convertMaterial(const aiScene* scene, const char* filePath);
		std::tuple<int, int, uint8_t*> loadTexture(const char* texturePath);
		std::tuple<int, int, uint8_t*> loadTexture(const char* texturePath, const CreateMaterialInfo& info);
		std::tuple<int, int, uint8_t*> loadEmbeddedTexture(const aiTexture* aiTexture);

		// animation data methods
		SkeletonRig loadSkeletonRig(const aiScene *scene);
		void processSkeletonNode(aiNode* node, const aiScene* scene, SkeletonRig& skeletonRig);
		void readSkeletonRigData(SkeletonRig& skeletonRig, aiMesh* mesh);
		void readHeirarchyData(Animation::NodeData& nodeData, const aiNode* node);

		glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4& from);

		RenderManager* renderManager;
	};
}

