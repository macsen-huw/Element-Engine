#pragma once

#include "element.hpp"
#include "GameObject.hpp"
#include "Trigger.hpp"
#include "Platform.hpp"
#include "FallingRock.hpp"

// Individual level segment
struct Chunk {
	elmt::Model* model;
	GameObject* obj;
};

// Use this for handling the main level
class Level
{



	// Chunks
	std::vector<Chunk> chunks;

	// General models
	std::map<std::string, elmt::Model*> modelList;

	// Border
	elmt::Material borderMaterial;
	elmt::Material borderMaterialHeightmap;

	elmt::Mesh borderMesh = elmt::Mesh();
	std::vector<elmt::Mesh> borderMeshes;
	std::vector<elmt::Material> borderMaterials;

	std::vector<Platform*> pTemplates = {};
	std::vector<FallingRock*> rTemplates = {};

	// Floor
	Trigger* killPlane;

	Trigger* goalTrigger;
	Trigger* startTrigger;

	
public:
	const int borderW = 40, borderH = 40;
	const int chunkW = 40, chunkH = 40;

	const std::string genPath = getAssetBase() + "gen" + g::fileSep;

	Level();

	~Level();

	// Setup level
	int Setup(bool useProcGen);


	glm::vec3 chunkOffset = { 0.0, -1.0, 0.0 };

	const std::string getAssetBase();

	void addChunk(std::string chunkName, glm::vec3 pos, bool hasWalls);

	// Add model to list of models in the level (.obj is appended to the end)
	void addModel(std::string modelName);

	// Get a model from the list of models in the level
	elmt::Model* getModel(std::string modelName);
};

