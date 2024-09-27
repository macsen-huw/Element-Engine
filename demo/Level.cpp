#include "Level.hpp"
#include "globals.hpp"

#include "element.hpp"
#include "GameObject.hpp"
#include "Platform.hpp"

#include "Player.hpp"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <glm.hpp>

#endif

using namespace globals;

Level::Level()
{
	
}

Level::~Level()
{
	for (Chunk& c : chunks) {
		delete c.model;
		delete c.obj;
	}

	for (auto mp : modelList) {
		elmt::Model* m = mp.second;
		delete m;
	}

	for (auto p : pTemplates) {
		delete p;
	}

	for (auto r : rTemplates) {
		delete r;
	}

	delete killPlane;
	delete goalTrigger;
	delete startTrigger;

}


struct FloorGenInfo {
	bool makeWalls = true;
};

void floorHeightFunction(colcom* pixelPointer, float x, float y, unsigned int seed, void* extraData)
{
	FloorGenInfo* data;
	if (extraData) {
 		data = (FloorGenInfo*)extraData;
	}

	const float h = 2.0 / 64.0;

	double height;
	if (data->makeWalls && (y <= h  || y >= 1.0-h) ) {
		height = 1.0;
	}
	else {
		height = 0.0;
	}
	

	colcom r, g, b;

	g::procMan->scalarToRGB(height, 1.0, &r, &g, &b);

	pixelPointer[0] = r;
	pixelPointer[1] = g;
	pixelPointer[2] = b;

}

int Level::Setup(bool useProcGen)
{

	auto mdl1 = Platform::generateModel("plat1", "rock2.jpg", 0);
	auto mdl2 = Platform::generateModel("plat2", "rock2.jpg", 10);
	Platform::models.push_back(mdl1);
	Platform::models.push_back(mdl2);
	
	
	// Generate borders
	unsigned int bw = 64, bh = 32;

	borderMaterial = g::assMan->createMaterialDiffuse((getAssetBase() + "border_tex.png").c_str());

	elmt::Material bmh = g::assMan->createMaterialDiffuse( (getAssetBase() + "border.png").c_str()  );
	 
	elmt::materialHeightInfo genInfo;
	genInfo.material = &bmh;
	genInfo.imageChannels = 4;
	genInfo.sampleChannel = 1;
	
	borderMaterialHeightmap = g::procMan->generateHeightmap( (genPath + "border_h_out.png").c_str(), bw, bh, elmt::ProcManager::materialHeightFunction, 0, &genInfo);

	// Create Border mesh
	elmt::ProcManager::MeshCreateInfo borderCI{
		glm::vec3(0.0, 0.0, 0.0),
		borderW, borderH, // dimensions
		bw, bh, // cells
		0.0, 40.0, // height range
		0, // material index
		elmt::ProcManager::SampleFilter::FILT_SAMPLE_LINEAR, // filtering
		3, // channels
		1.0, 1.0 // tex scale
		//glm::mat3(glm::eulerAngleXYZ(glm::pi<float>() * -0.25, 0.0, 0.0)) // rotation
	};

	borderMesh = g::procMan->createMesh(borderMaterialHeightmap, borderCI);
	borderMeshes = std::vector<elmt::Mesh>{ borderMesh };
	borderMaterials = std::vector<elmt::Material>{ borderMaterial };
	modelList.insert({ "border", new elmt::Model(borderMeshes, borderMaterials) });

	// Floor
	int fw = 2, fh = 64;
	FloorGenInfo floorGenInfo;
	floorGenInfo.makeWalls = true;
	auto floorHeightmap1 = g::procMan->generateHeightmap((genPath + "floor_h_walls_out.png").c_str(), fw, fh, floorHeightFunction, 0, &floorGenInfo);
	floorGenInfo.makeWalls = false;
	auto floorHeightmap2 = g::procMan->generateHeightmap((genPath + "floor_h_nowalls_out.png").c_str(), fw, fh, floorHeightFunction, 0, &floorGenInfo);

	elmt::ProcManager::MeshCreateInfo floorCI = borderCI;
	floorCI.width = chunkW;
	floorCI.length = chunkH;
	floorCI.nPointsX = fw;
	floorCI.nPointsY = fh;
	floorCI.maxHeight = 10.0;


	auto floorMesh1 = g::procMan->createMesh(floorHeightmap1, floorCI);
	auto floorMesh2 = g::procMan->createMesh(floorHeightmap2, floorCI);

	auto floorMaterial = g::assMan->createMaterialDiffuse((getAssetBase() + "floor.png").c_str());
	auto floorMaterials = std::vector<elmt::Material>{ floorMaterial };

	auto floorMeshes = std::vector<elmt::Mesh>{ floorMesh1 };
	modelList.insert({ "floor walls", new elmt::Model(floorMeshes, floorMaterials) });

	floorMeshes = std::vector<elmt::Mesh>{ floorMesh2 };
	modelList.insert({ "floor nowalls", new elmt::Model(floorMeshes, floorMaterials) });

	chunks = {};


	addChunk("chunk1.obj", chunkOffset + glm::vec3(0.0, 0.0, -chunkW * 0.0), false);
	//addChunk("chunk2.obj", chunkOffset + glm::vec3(0.0, 0.0, -chunkW * 1.0), false);
	//addChunk("platforms", chunkOffset + glm::vec3(0.0, 0.0, -chunkW * 2.0), true);
	//addChunk("chunk3.obj", chunkOffset + glm::vec3(0.0, 0.0, -chunkW * 3.0), false);
	//addChunk("chunk4.obj", chunkOffset + glm::vec3(0.0, 0.0, -chunkW * 4.0), false);
	//addChunk("rocks", chunkOffset + glm::vec3(0.0, 0.0, -chunkW * 5.0), true);
	//addChunk("chunk5.obj", chunkOffset + glm::vec3(0.0, 0.0, -chunkW * 5.0), false);


	glm::vec3 killPlaneDim = glm::vec3(chunkW * 1, -4.0, chunkH * 4);
	//killPlane = new Trigger(g::level->chunkOffset + glm::vec3(0.0, 0.0, -g::level->chunkH * 4), killPlaneDim, { {"Camera",g::player->killPlaneCallback}, {"Player",g::player->killPlaneCallback} });

	glm::vec3 goalDim = glm::vec3(chunkW * 1, 10, chunkH * 1);
	//goalTrigger = new Trigger(g::level->chunkOffset + glm::vec3(0.0, 0.0, -g::level->chunkH * 3), goalDim, { {"Camera",g::player->winCallback}, {"Player",g::player->winCallback} });

	glm::vec3 startDim = glm::vec3(chunkW * 1, 10, chunkH * 1);
	//startTrigger = new Trigger(g::level->chunkOffset + glm::vec3(0.0, 0.0, -g::level->chunkH * 2), startDim, { {"Camera",g::player->startCallback}, {"Player",g::player->startCallback} });

	return 0;
}

const std::string Level::getAssetBase()
{
	const std::string assetPath = g::BASE_PATH + "level" + g::fileSep;
	return assetPath;
}




void Level::addChunk(std::string chunkName, glm::vec3 pos, bool hasWalls)
{
	auto fullChunkName = ("Chunk " + chunkName);

	auto nameLength = chunkName.size();

	char ext[5];
	if (nameLength >= 4) {
		ext[3] = chunkName[nameLength - 1];
		ext[2] = chunkName[nameLength - 2];
		ext[1] = chunkName[nameLength - 3];
		ext[0] = chunkName[nameLength - 4];

		ext[4] = '\0';
	}


	if ( !chunkName.empty() ) {
		if (std::string(ext) == ".obj") {
			auto model = g::assMan->importAsset(getAssetBase(), chunkName);
			Chunk newChunk;
			newChunk.model = model;
			newChunk.obj = new GameObject(fullChunkName.c_str(), pos, model, true);
			newChunk.obj->getPhysComp()->setKinematic();
			chunks.push_back(newChunk);

		}
		else if (chunkName == "platforms") {

			Platform* pl1 = new Platform({ 0.0, 0.0, 20.0 }, Platform::models[0]);
			pl1->setParent(nullptr);
			Platform* pl2 = new Platform({ 20.0, 0.0, 0.0 }, Platform::models[1]);
			pl2->setParent(nullptr);
			pTemplates.push_back(pl1);
			pTemplates.push_back(pl2);

			float plWidth = 5.0;
			float plHeight = 5.0;

			elmt::ProcManager::RandomSpawnInfo spawnInfo;
			spawnInfo.pos = pos;
			spawnInfo.pos.z -= chunkH;
			spawnInfo.pos += glm::vec3(plWidth, 0.0, plHeight) / 2.0f;

			spawnInfo.width = chunkW - plWidth;
			spawnInfo.length = chunkH - plHeight;
			spawnInfo.height = 1.0;
			spawnInfo.spawnType = elmt::ProcManager::RandomSpawnType::SPWN_CELLS;
			spawnInfo.cellSpawnInfo.cellsX = 4;
			spawnInfo.cellSpawnInfo.cellsY = 1;
			spawnInfo.cellSpawnInfo.cellsZ = 4;
			spawnInfo.cellSpawnInfo.pattern = elmt::ProcManager::CellSpawnPattern::PTRN_ADJACENT;
			glm::uvec3 presetPositions[] = { {0, 0, 0} };
			spawnInfo.cellSpawnInfo.presetPositions = presetPositions;
			spawnInfo.cellSpawnInfo.presetPositionsAmount = sizeof(presetPositions) / sizeof(glm::uvec3);
			std::vector<Platform*> spawnedPlatforms = g::procMan->spawnEntities<Platform>({ pl1, pl2 },
				6, 6, 12, spawnInfo);

			for (auto p : spawnedPlatforms) {
				p->setParent(g::platformsBase);
			}
		}
		else if (chunkName == "rocks") {
			FallingRock* fallingRockTemplate = new FallingRock(glm::vec3(0.0, 0.0, 0.0));
			fallingRockTemplate->getPhysComp()->setKinematic();
			fallingRockTemplate->setParent(nullptr);
			rTemplates.push_back(fallingRockTemplate);

			elmt::ProcManager::RandomSpawnInfo spawnInfo;
			spawnInfo.pos = pos + glm::vec3(0, 10, 0);
			spawnInfo.pos.z -= chunkH;

			spawnInfo.width = 20;
			spawnInfo.length = 20;
			spawnInfo.height = 5;
			spawnInfo.spawnType = elmt::ProcManager::RandomSpawnType::SPWN_CELLS;
			spawnInfo.cellSpawnInfo.cellsX = 20;
			spawnInfo.cellSpawnInfo.cellsY = 1;
			spawnInfo.cellSpawnInfo.cellsZ = 20;
			spawnInfo.cellSpawnInfo.pattern = elmt::ProcManager::CellSpawnPattern::PTRN_RANDOM;
			glm::uvec3 presetPositions[] = { {0, 0, 0}, { 2,0,0 }, { 5, 0, 2 } };
			spawnInfo.cellSpawnInfo.presetPositions = presetPositions;
			spawnInfo.cellSpawnInfo.presetPositionsAmount = sizeof(presetPositions) / sizeof(glm::uvec3);
			spawnInfo.parent = g::platformsBase;

			std::vector<FallingRock*> spawnedRocks = g::procMan->spawnEntities<FallingRock>({ fallingRockTemplate },
				10, 10, 12, spawnInfo);
			// Update physics components
			for (auto rock : spawnedRocks) {
				auto physComp = ((FallingRock*)rock)->getPhysComp();
				physComp->setPosition(rock->pos);
			}
		}
		
	}

	//glm::vec3 dim = model->bvh->root.b2 - model->bvh->root.b1;
	
	auto chunkBorder1 = new GameObject( (fullChunkName + " Border").c_str(), pos + glm::vec3(chunkW + borderW,-10.0,0.0), modelList["border"], true);
	chunkBorder1->getPhysComp()->setKinematic();
	chunkBorder1->rotateHorizontal(glm::pi<float>() * 1.0);
	
	auto chunkBorder2 = new GameObject((fullChunkName + " Border2").c_str(), pos + glm::vec3(-chunkW, -10.0, -chunkH), modelList["border"], true);
	chunkBorder2->getPhysComp()->setKinematic();

	elmt::Model* floorModel;
	if (hasWalls) {
		floorModel = modelList["floor walls"];
	} else {
		floorModel = modelList["floor nowalls"];
	}

	auto floor = new GameObject((fullChunkName + " Floor").c_str(), pos + glm::vec3(0.0, -10.0, -chunkH), floorModel, true);
}

void Level::addModel(std::string modelName)
{
	auto model = g::assMan->importAsset(getAssetBase(), modelName + ".obj");
	modelList.insert({ modelName, model });
}

elmt::Model* Level::getModel(std::string modelName)
{
	// Add if doesn't exist
	auto it = modelList.find(modelName);
	if (it == modelList.end()) {
		elmt::Logger::Print("Model name \"" + modelName + "\" was not found in level model list, adding now", elmt::LOGCAT::CAT_LOADING, elmt::LOGSEV::SEV_INFO);
		addModel(modelName);
	}

	return modelList[modelName];
}
