#pragma once

#include "Material.hpp"
#include "Model.hpp"
#include "../external/PerlinNoise.hpp"
#include "../external/glm-0.9.7.1/glm/glm.hpp"

#define colcom unsigned char

typedef void (*heightmapGenFunction)(colcom* pixelPointer, float x, float y, unsigned int seed, void* extraData);

/*
The default colcom values provided are always 255. To modify roughness/metal, only update the r component
*/
typedef void (*BRDFGenFunction)(colcom* diffusePointer, colcom* normalPointer, colcom* roughnessPointer, colcom* metalPointer, float x, float y, unsigned int seed, void* extraData);

namespace elmt
{
	//TODO consistant namespacing
	/*
	Info that can be passed into a perlin function to provide extra specificity
	*/
	struct perlinInfo {
		siv::PerlinNoise noiseEngine;
		unsigned int octaves = 4;
		float persistance = 0.5;
		float scale = 1.0;
		float xScale = 1.0;
		float yScale = 1.0;
	};

	/*
	Info that can be passed into a worley noise function
	*/
	struct worleyInfo {
		unsigned int cellsX = 10;
		unsigned int cellsY = 10;

		bool useSigmoid = true;

		float dOffset = 0.0;
		float dIOffset = 1.0;
		float dScale = 0.2;
		float hOffset = 0.0;
		float hScale = 0.25;
		float dMin = 0.1;
		float dMax = 2.1;

		bool invertDist = true;

	};

	// The shape of the code generated when using the cone heightmap callback
	enum class CONEMODE : unsigned int {
		// Height based linearly on distance from center
		CONE_SHARP, 
		// Height based on normal distribution with mean in center
		CONE_NORMAL
	};

	/*
	Info that can be passed into a cone height function
	*/
	struct coneInfo {
		float centerX = 0.5;
		float centerY = 0.5;
		float scale = 1.0;
		float offset = 0.0;
		// If set, generates a valley instead of a cone
		bool flip = false;
		
		CONEMODE mode = CONEMODE::CONE_SHARP;

		// Only for CONEMODE::CONE_SHARP
		float maxDist = 0.5;

		// Only for CONEMODE::CONE_NORMAL
		float standard_deviation = 0.45;
		// Amount distance is scaled by when sampled for normal
		float dScale = 1.0;
	};

	enum class MATERIALTEXTURE : unsigned int {
		NONE = 0,
		DIFFUSE = 1,
		ROUGHNESS = 2,
		METAL = 3,
		NORMAL = 4
	};

	/*
	Info that must be passed into material height function
	*/
	struct materialHeightInfo {
		Material* material = nullptr; // The material itself
		MATERIALTEXTURE texture = MATERIALTEXTURE::DIFFUSE; // Which material texture to sample from

		unsigned int imageChannels = 4; // Channels image has
		int sampleChannel = 0; // Channel to sample from, if -1, average brightness is sampled
		colcom maxValue = 255;
		
	};

	/*
	Info that can be passed into a solid BRDF function
	*/
	struct solidMaterialInfo {
		colcom r = 0;
		colcom g = 0;
		colcom b = 0;
		colcom a = 255;

		float roughness = 0.2;
		float metalicness = 0.2;
	};

	/*
	Info that can be passed into a gradient BRDF
	*/
	struct gradientMaterialInfo {
		solidMaterialInfo mat1 = { 0,0,0,128,0.0,0.0 };
		solidMaterialInfo mat2 = { 32,255,0,255, 1.0, 1.0 };

		bool useY = false; // Whether to use X (false) or Y (true) dimension
		bool useSmoothstep = true; // Whether to use linear or smooth interpolation
	};

	/*
	Info that can be passed into a wave BRDF function
	*/
	typedef struct waveMaterialInfo {
		// Bottom/valley of the wave
		colcom br = 0;
		colcom bg = 0;
		colcom bb = 255;
		colcom ba = 255;

		// Top/peak of the wave
		colcom tr = 0;
		colcom tg = 0;
		colcom tb = 128;
		colcom ta = 255;

		// Number of waves in the texture
		unsigned int waveNumber = 1;
		// horizontal wave offset
		float offset = 0.0;

		float roughness = 0.2;
		float metalicness = 0.2;

		// If true, normalmap will be generated as if the wave was sticking out of the texture
		bool generateNormalMap = true;

	};

	/*
	Info that can be passed into a white noise BRDF
	*/
	struct whiteNoiseMaterialInfo {
		solidMaterialInfo base;
		colcom variance = 255;
		bool clamp = true;
	};

	/*
	Info that can be passed into a perlin noise BRDF
	*/
	struct perlinNoiseMaterialInfo {
		solidMaterialInfo base;
		siv::PerlinNoise noiseEngine;
		unsigned int octaves = 4;
		float persistance = 0.5;
		colcom variance = 255;
		bool clamp = true;
	};

	/*
	Info that can be pased into a worley noise BRDF
	*/
	struct worleyNoiseMaterialInfo {
		solidMaterialInfo base;
		worleyInfo worley;
		bool clamp = true;
	};

	/*
	Info that can be passed into a brick BRDF
	*/
	struct brickMaterialInfo {
		// Brick
		solidMaterialInfo brickInfo = solidMaterialInfo{128, 32, 32, 255, 0.3, 0.3};

		// Filler
		solidMaterialInfo fillerInfo = solidMaterialInfo{ 196, 196, 196, 255, 0.8, 0.1 };

		float brickWidth = 0.15;
		float brickHeight = 0.1;

		float fillerWidth = 0.02;
		// Use a random value for the normal map in the filler region
		bool fillerRandomNormal = true;
	};

	/*
	Data for a single pattern used for patternMaterialInfo
	which is used by patternBRDFFunction
	*/
	const size_t maxPatternSize = (16 * 16) + 1;
	struct patternInfo {
		// String of pattern data, use 0 for colour 1 and 1 for colour 2
		char data[maxPatternSize] = "1001";
		unsigned int patternWidth = 2;
		unsigned int patternHeight = 2;
	};

	const inline static patternInfo stripePatternInfo{
		"11000001"
		"11100000"
		"01110000"
		"00111000"
		"00011100"
		"00001110"
		"00000111"
		"10000011",
		8,
		8
	};

	const inline static patternInfo brickPatternInfo{
		"11111111"
		"00000001"
		"00000001"
		"00000001"
		"11111111"
		"00010000"
		"00010000"
		"00010000",
		8,
		8
	};

	const inline static patternInfo circlePatternInfo{
		"0000011111100000"
		"0001100000011000"
		"0010000000000100"
		"0100000000000010"
		"1000000000000001"
		"1000000000000001"
		"1000000000000001"
		"1000000000000001"
		"1000000000000001"
		"1000000000000001"
		"1000000000000001"
		"1000000000000001"
		"0100000000000010"
		"0010000000000100"
		"0001100000011000"
		"0000011111100000",
		16,
		16
	};

	/*
	Info that can be passed into a pattern BRDF
	*/
	struct patternMaterialInfo {
		// For pattern samples of 0
		solidMaterialInfo mat1Info = solidMaterialInfo{ 128, 32, 32, 255, 0.3, 0.3 };

		// For pattern samples of 1
		solidMaterialInfo mat2Info = solidMaterialInfo{ 196, 196, 196, 255, 0.8, 0.1 };

		// Scale of the pattern across the generated material, not to be confused with patternInfo.patternWidth
		float patternWidth = 0.1;
		// Scale of the pattern across the generated material, not to be confused with patternInfo.patternHeight
		float patternHeight = 0.1;

		patternInfo pattern;
		
	};


	//Mode used for component masking
	enum class COLOURMASKMODE : unsigned int {
		COLOUR_MASK_ANY, // Any component can pass to pass mask
		COLOUR_MASK_ALL, // All components must pass to pass mask
		COLOUR_MASK_R, // Red components must pass to mass mask
		COLOUR_MASK_G, // Green components must pass to mass mask
		COLOUR_MASK_B, // Blue components must pass to mass mask
		COLOUR_MASK_A, // Alpha components must pass to mass mask
	};
	/*
	Info used for masking materials/heightmaps together
	*/
	struct MaskInfo {
		// Heightmap masking
		float minVal = 0.5; // Minimum (normalized) height value to pass mask
		float maxVal = 1.0; // Maximum (normalized) height value to pass mask
		float failVal = 0.0; // Value used if mask fails
		
		// Material masking
		COLOURMASKMODE compareMode = COLOURMASKMODE::COLOUR_MASK_ANY;
		float minR = 0.5, maxR = 1.0;
		float minG = 0.5, maxG = 1.0;
		float minB = 0.5, maxB = 1.0;
		float minA = 0.5, maxA = 1.0;

		// Values used if mask fails
		colcom failR = 0, failG = 0, failB = 0, failA = 0;
	};
	
	// Mode used when blending two textures together, e.g. in mixMaterials
	enum class BLENDMODE : unsigned int {
		MODE_ADD,
		MODE_SUB,
		MODE_MUL,
		MODE_MAX,
		MODE_MIN,
		MODE_MASK
	};
	/*
	Blend info for mixing materials togeter
	*/
	struct BlendInfo{
		BLENDMODE blendMode = BLENDMODE::MODE_ADD;
		float mat1Weight = 0.5;
		float mat2Weight = 0.5;
		// Horizontal offset to blend the second material at
		float xOffset = 0.0;
		// Vertical offset to blend the second material at
		float yOffset = 0.0;
		bool clamp = true;

		// Whether material 1 should be flipped vertically, set if output appears flipped
		bool mat1Flip = false;
		// Whether material 2 should be flipped vertically, set if output appears flipped TODO fix
		bool mat2Flip = false;
		// Channels in the material data of material 1. 3 if generated as a heightmap, likely to be 4 otherwise
		unsigned int mat1Channels = 3;
		// Channels in the material data of material 2. 3 if generated as a heightmap, likely to be 4 otherwise
		unsigned int mat2Channels = 3;

		// If BLENDMODE set to MODE_MAX, info used for masking TODO implement for materials
		MaskInfo maskInfo;
	};

	class ProcManager
	{
		// Methods
	public:
		ProcManager() {};

		void scalarToRGB(float val, float maxVal, unsigned char* r, unsigned char* g, unsigned char* b);
		float RGBToScalar(float maxVal, unsigned char r, unsigned char g, unsigned char b);

		// Convert a normal vector into RGB, used for normal maps
		void normalToRGB(glm::vec3 normal, unsigned char* r, unsigned char* g, unsigned char* b);
		glm::vec3 RGBToNormal(unsigned char r, unsigned char g, unsigned char b);

		/*
		Basic perlin noise based procgen function
		Use perlinInfo struct to pass in additional data
		*/
		static void perlinHeightFunction(colcom* pixelPointer, float x, float y, unsigned int seed, void* extraData);
		// Generate value for worley noise
		float worley(worleyInfo info, float x, float y, unsigned int seed);
		/*
		Basic worley noise based procgen function
		Use worleyInfo struct to pass in additional data
		*/
		static void worleyHeightFunction(colcom* pixelPointer, float x, float y, unsigned int seed, void* extraData);
		/*
		Generates a heightmap with a maximum value in the center
		Use coneInfo struct to pass in additional data
		*/
		static void coneHeightFunction(colcom* pixelPointer, float x, float y, unsigned int seed, void* extraData);
		/*
		Generates a heightmap based on the single component of the provided image.
		Useful for generating heightmaps from existing images, though it will have less precision than the 3-component heightmap format
		Must use materialHeightInfo struct to provide image data
		*/
		static void materialHeightFunction(colcom* pixelPointer, float x, float y, unsigned int seed, void* extraData);

		// Test height function
		static void heightFunctionTest(colcom* pixelPointer, float x, float y, unsigned int seed, void* extraData);

		static void solidBRDFFunction(colcom* diffusePointer, colcom* normalPointer, colcom* roughnessPointer, colcom* metalPointer, float x, float y, unsigned int seed, void* extraData);
		static void gradientBRDFFunction(colcom* diffusePointer, colcom* normalPointer, colcom* roughnessPointer, colcom* metalPointer, float x, float y, unsigned int seed, void* extraData);
		static void whiteNoiseBRDFFunction(colcom* diffusePointer, colcom* normalPointer, colcom* roughnessPointer, colcom* metalPointer, float x, float y, unsigned int seed, void* extraData);
		static void perlinNoiseBRDFFunction(colcom* diffusePointer, colcom* normalPointer, colcom* roughnessPointer, colcom* metalPointer, float x, float y, unsigned int seed, void* extraData);
		static void worleyNoiseBRDFFunction(colcom* diffusePointer, colcom* normalPointer, colcom* roughnessPointer, colcom* metalPointer, float x, float y, unsigned int seed, void* extraData);
		static void waveBRDFFunction(colcom* diffusePointer, colcom* normalPointer, colcom* roughnessPointer, colcom* metalPointer, float x, float y, unsigned int seed, void* extraData);
		static void brickBRDFFunction(colcom* diffusePointer, colcom* normalPointer, colcom* roughnessPointer, colcom* metalPointer, float x, float y, unsigned int seed, void* extraData);
		static void patternBRDFFunction(colcom* diffusePointer, colcom* normalPointer, colcom* roughnessPointer, colcom* metalPointer, float x, float y, unsigned int seed, void* extraData);

		
		static void BRDFFunctionTest(colcom* diffusePointer, colcom* normalPointer, colcom* roughnessPointer, colcom* metalPointer, float x, float y, unsigned int seed, void* extraData);

		
		Material generateBRDFMaterial(const char* outName, const char* outFormat, unsigned int width, unsigned int height, BRDFGenFunction genFunction, unsigned int seed, void* extraData, bool loadIfExists);
		Material generateHeightmap(const char* outPath, unsigned int width, unsigned int height, heightmapGenFunction genFunction, unsigned int seed, void* extraData, bool loadIfExists);

		Material generateBRDFMaterial(const char* outName, const char* outFormat, unsigned int width, unsigned int height, BRDFGenFunction genFunction, unsigned int seed, void* extraData) {
			return generateBRDFMaterial(outName, outFormat, width, height, genFunction, seed, extraData, true);
		};
		Material generateHeightmap(const char* outPath, unsigned int width, unsigned int height, heightmapGenFunction genFunction, unsigned int seed, void* extraData) {
			return generateHeightmap(outPath, width, height, genFunction, seed, extraData, true);
		};

		/*
		Mix two materials together
		*/
		Material mixMaterials(
			Material& mat1,
			Material& mat2,
			const char* outName, const char* outFormat, unsigned int outWidth, unsigned int outHeight,
			BlendInfo info
		);

		/*
		Mix two heightmaps together
		*/
		Material mixHeightmapMaterials(
			Material& mat1, float mat1MaxHeight,
			Material& mat2, float mat2MaxHeight,
			const char* outPath, unsigned int outWidth, unsigned int outHeight, float outMaxHeight,
			BlendInfo& info
			);

		/*
		Mix two heightmap images together. This method is outdated, prefer to use mixHeightmapMaterials instead
		*/
		colcom* mixHeightmaps(colcom* h1, unsigned int h1Width, unsigned int h1Height, float h1Weight, float h1MaxHeight,
			colcom* h2, unsigned int h2Width, unsigned int h2Height, float h2Weight, float h2MaxHeight,
			const char* outPath, unsigned int outWidth, unsigned int outHeight, float outMaxHeight);

		/*
		The blending type used when sampling a heightmap for mesh creation
		*/
		enum class SampleFilter : unsigned int {
			// Sample the nearest pixel
			FILT_SAMPLE_NEAREST,
			// Blend linearly between the nearest 2x2 pixels
			FILT_SAMPLE_LINEAR,
			// Take the highest value from the nearest 2x2 pixels
			FILT_SAMPLE_POOL_MAX,
			// Take the lowest value from the nearest 2x2 pixels
			FILT_SAMPLE_POOL_MIN
		};

		/*
		The thing the sampling is for
		*/
		enum class SampleType : unsigned int {
			SAMPLE_RGBA,
			SAMPLE_HEIGHT,
			SAMPLE_NORMAL
		};

		/*
		Info used when sampling texture data
		*/
		struct SampleInfo {
			unsigned int texWidth = 0;
			unsigned int texHeight = 0;
			unsigned int channels = 4;

			SampleFilter filterMode = SampleFilter::FILT_SAMPLE_NEAREST;
			SampleType sampleType = SampleType::SAMPLE_RGBA;

			//Used when sampleType = SAMPLE_HEIGHT
			float heightRange;
			// Used for linear filtering, distance between the points
			//float filterWidth = 0.0;
			// Used for linear filtering, distance between the points
			//float filterHeight = 0.0;

			
		};

		/*
		Winding order of created mesh triangles
		*/
		enum class WindingType : unsigned int{
			WIND_NORMAL,
			WIND_REVERSE,
			WIND_BOTH
		};

		/*
		Info used for the createMesh function
		*/
		struct MeshCreateInfo {
			glm::vec3 offset = { 0.0,0.0,0.0 }; //XYZ of mesh topleft corner
			float width = 10.0; // Width of mesh
			float length = 10.0; // Height of mesh
			unsigned int nPointsX = 64; // Number of points to sample the heightmax in X direction
			unsigned int nPointsY = 64; // Number of points to sample the heightmax in Y direction
			float minHeight = 0; // Minimum height of mesh
			float maxHeight = 10.0; //Maximum height of mesh
			std::uint32_t materialIndex;
			SampleFilter filterType; // Filter type when sampling
			unsigned int channels = 3; // Number of channels in the input
			// Texture width across the mesh. 1.0 covers the whole mesh with the whole texture. Smaller values repeat the texture
			float texScaleX = 1.0;
			// Texture height across the mesh. 1.0 covers the whole mesh with the whole texture. Smaller values repeat the texture
			float texScaleY = 1.0;
			glm::mat3 rotation = glm::mat3(1.0);
			// If set, generated vertices are oriented along the normals sampled from this materials normal texture
			Material* normalMaterial = nullptr;
			// similar to texScaleX, but for normal sampling
			float normTexScaleX = 1.0;
			// similar to texScaleY, but for normal sampling
			float normTexScaleY = 1.0;
			// Whether or not to drop vertices based on a masking material
			Material* maskMaterial = nullptr;
			MaskInfo maskInfo;
			unsigned int maskChannels = 3;
			// Whether or not triangle winding order should be reversed. Set this if the mesh should be viewed from below
			WindingType windingOrder = WindingType::WIND_NORMAL;

		};

		/*
		Create a model mesh from a heightmap material
		*/
		Mesh createMesh(const Material& mat, const MeshCreateInfo& createInfo);

		/*
		Create a full model from a heightmap material and texture
		*/
		Model createModel(const Material& matHeight, const Material& matTexture, const MeshCreateInfo& createInfo);

		/*
		Spawn boundaries for random object spawning
		*/
		enum class RandomSpawnType : unsigned int {
			SPWN_BOX, // Spawn within box
			SPWN_ELLIPSE, // Spawn within ellipse
			SPWN_CELLS //Spawn in a pigeonhole manner, with the space split into cells and a maximum of one object per cell
		};

		enum class CellSpawnPattern : unsigned int {
			PTRN_RANDOM, // Fill cells randomly
			PTRN_ADJACENT, // Every cell besides the first must be next to a filled cell
			PTRN_PATH, // Every cell besides the first must be next to the previously filled cell
		};

		/*
		Info used by RandomSpawnInfo, when spawnType is set to SPWN_CELL
		*/
		struct CellSpawnInfo {
			unsigned int cellsX = 5;
			unsigned int cellsY = 5;
			unsigned int cellsZ = 5;
			CellSpawnPattern pattern = CellSpawnPattern::PTRN_RANDOM;

			// Set some number of these to predetermine the filling of certain cells
			glm::uvec3* presetPositions = nullptr;
			unsigned int presetPositionsAmount = 0;
		};

		/*
		Info used for random spawning
		*/
		struct RandomSpawnInfo {
			// Pos
			glm::vec3 pos{ 0.0,0.0,0.0 };

			// Bounding
			float width = 1.0;
			float length = 1.0;
			float height = 1.0;
			
			RandomSpawnType spawnType = RandomSpawnType::SPWN_BOX; // Spawn type
			CellSpawnInfo cellSpawnInfo;

			unsigned int maxSpawnAttempts = 10; // Spawn attempts per Entity

			float minDistanceBetweenObjects = 0.0;

			// If set, new parent to be given to the Entity
			Entity* parent = nullptr;
		};

		/*
		Spawn a random number of cloned Entity objects from the provided list
		*/
		template <typename T>
		std::vector<T*> spawnEntities(std::vector<T*> entities,
			unsigned int minObjects, unsigned int maxObjects, unsigned int seed, RandomSpawnInfo spawnInfo) {
			srand(seed);

			std::vector<T*> spawnedEntities;

			float minDistanceBetweenObjects = spawnInfo.minDistanceBetweenObjects;

			unsigned int objectsToSpawn;
			if (minObjects == maxObjects) {

				objectsToSpawn = minObjects;
			}
			else {
				objectsToSpawn = minObjects + (rand() % (maxObjects - minObjects));
			}

			int maxAttempts = spawnInfo.maxSpawnAttempts;

			// Used for SPWN_CELLS, cells that have been filled while spawning
			std::vector<unsigned int> filledCells;
			unsigned int cellCount;
			float cellW, cellH, cellL;
			CellSpawnInfo& cellInfo = spawnInfo.cellSpawnInfo;
			if (spawnInfo.spawnType == RandomSpawnType::SPWN_CELLS) {
				filledCells = {};
				cellCount = cellInfo.cellsX * cellInfo.cellsY * cellInfo.cellsZ;

				cellW = spawnInfo.width / (float)cellInfo.cellsX;
				cellH = spawnInfo.height / (float)cellInfo.cellsY;
				cellL = spawnInfo.length / (float)cellInfo.cellsZ;
			}

			for (int i = 0; i < objectsToSpawn; i++) {
				// Choose object
				T* ent = entities[rand() % entities.size()];


				bool validPos;

				// Attempt to spawn
				glm::vec3 spawnPos = spawnInfo.pos;
				for (int attempt = 0; attempt < maxAttempts; attempt++) {
					validPos = true;

					// Choose pos
					if (spawnInfo.spawnType == RandomSpawnType::SPWN_BOX) {
						spawnPos.x = ((float)rand() / (float)RAND_MAX) - 0.5;
						spawnPos.y = ((float)rand() / (float)RAND_MAX) - 0.5;
						spawnPos.z = ((float)rand() / (float)RAND_MAX) - 0.5;

						spawnPos.x *= spawnInfo.width;
						spawnPos.z *= spawnInfo.length;
						spawnPos.y *= spawnInfo.height;

						spawnPos += spawnInfo.pos;
					}
					else if (spawnInfo.spawnType == RandomSpawnType::SPWN_ELLIPSE) {
						while (true) {
							//TODO improve when using non-uniform ellipse
							spawnPos.x = ((float)rand() / (float)RAND_MAX) - 0.5;
							spawnPos.y = ((float)rand() / (float)RAND_MAX) - 0.5;
							spawnPos.z = ((float)rand() / (float)RAND_MAX) - 0.5;

							// Ensure we are in ellipse
							if (glm::length(spawnPos) > 1.0) {
								continue;
							}

							spawnPos.x *= spawnInfo.width;
							spawnPos.z *= spawnInfo.length;
							spawnPos.y *= spawnInfo.height;

							spawnPos += spawnInfo.pos;
							break;
						}
					}
					else if (spawnInfo.spawnType == RandomSpawnType::SPWN_CELLS) {
						unsigned int chosen;
						if (i < cellInfo.presetPositionsAmount) {
							glm::uvec3& presetVec = cellInfo.presetPositions[i];
							chosen = cellToIndex(presetVec.x, presetVec.y, presetVec.z, cellInfo);
						}
						else {
							if (cellInfo.pattern == CellSpawnPattern::PTRN_RANDOM || filledCells.empty()) {
								// Fill random cell
								unsigned int cx, cy, cz;
								cx = rand() % cellInfo.cellsX;
								cy = rand() % cellInfo.cellsY;
								cz = rand() % cellInfo.cellsZ;

								chosen = cellToIndex(cx, cy, cz, cellInfo);  //(cz * cellInfo.cellsY * cellInfo.cellsX) + (cy * cellInfo.cellsX) + cx;
							}
							else if (cellInfo.pattern == CellSpawnPattern::PTRN_ADJACENT || cellInfo.pattern == CellSpawnPattern::PTRN_PATH) {
								// Take existing filled cell move
								unsigned int existing = 0;
								if (cellInfo.pattern == CellSpawnPattern::PTRN_ADJACENT) {
									existing = filledCells[rand() % filledCells.size()];
								}
								else if (cellInfo.pattern == CellSpawnPattern::PTRN_PATH) {
									existing = filledCells[filledCells.size() - 1];
								}


								glm::uvec3 cell = indexToCell(existing, cellInfo);
								unsigned int cx, cy, cz;
								cx = cell.x;
								cy = cell.y;
								cz = cell.z;

								// Choose new direction
								unsigned int validDirections = 0;
								unsigned int directions[6];
								int dx, dy, dz;
								for (unsigned di = 0; di < 6; di++) {
									validDirections++;

									switch (di) {
									case 0:
										dx = 0; dy = -1; dz = 0; break;
									case 1:
										dx = 1; dy = 0; dz = 0; break;
									case 2:
										dx = 0; dy = 1; dz = 0; break;
									case 3:
										dx = -1; dy = 0; dz = 0; break;
									case 4:
										dx = 0; dy = 0; dz = -1; break;
									case 5:
										dx = 0; dy = 0; dz = 1; break;
									}

									if (cx + dx < 0 || cx + dx >= cellInfo.cellsX || cy + dy < 0 || cy + dy >= cellInfo.cellsY || cz + dz < 0 || cz + dz >= cellInfo.cellsZ) {
										validDirections--;
									}
									else {
										directions[validDirections - 1] = cellToIndex(cx + dx, cy + dy, cz + dz, cellInfo);;
									}
								}
								if (validDirections) {
									chosen = directions[rand() % validDirections];
								}
								else {
									chosen = existing;
								}
							}
						}

						if (std::find(filledCells.begin(), filledCells.end(), chosen) != filledCells.end()) {
							// Cell is already filled
							validPos = false;
						}
						else {
							filledCells.push_back(chosen);

							glm::uvec3 cell = indexToCell(chosen, cellInfo);
							unsigned int cx, cy, cz;
							cx = cell.x;
							cy = cell.y;
							cz = cell.z;

							float sx = cellW * (float)cx;
							float sy = cellH * (float)cy;
							float sz = cellL * (float)cz;

							spawnPos.x = sx;
							spawnPos.y = sy;
							spawnPos.z = sz;

							spawnPos += spawnInfo.pos;
						}
					}

					if (!validPos) {
						continue;
					}

					// Ensure objects are spaced far enough apart
					glm::vec3 diff;

					for (Entity* oldSpawnedEntity : spawnedEntities) {
						diff = oldSpawnedEntity->pos - spawnPos;
						if (glm::length(diff) < minDistanceBetweenObjects) {
							validPos = false;
							break;
						}
					}

					if (!validPos) {
						continue;
					}

					// Actually spawn
					//TODO improve
					std::string suffix = ent->getName() + "_" + std::to_string(i + 1);

					std::cout << typeid(ent).name() << std::endl;
					decltype(ent) spawnedEntity = nullptr;
					if (spawnInfo.parent) {
						ent->clone(spawnedEntity, suffix.c_str(), spawnInfo.parent);
					}
					else {
						ent->clone(spawnedEntity, suffix.c_str());
					}

					if (spawnedEntity) {
						spawnedEntity->pos = spawnPos;
						spawnedEntities.push_back(spawnedEntity);
					}
					else {
						Logger::Print("Failed to clone Entity of type \"" + std::string(typeid(ent).name()) + "\" ", LOGCAT::CAT_LOADING | LOGCAT::CAT_CORE, LOGSEV::SEV_WARNING);
					}



					break;
				}
				if (!validPos) {
					Logger::Print("Failed to spawn Entity " + std::to_string(i) + " after " + std::to_string(maxAttempts) + " attempts. Giving up", LOGCAT::CAT_LOADING | LOGCAT::CAT_CORE, LOGSEV::SEV_INFO);
				}
			}

			return spawnedEntities;
		}

		/*
		Sample texture data at a given(normalised) x and y
		If SampleType is SAMPLE_HEIGHT, the returned value is the sampled height
		If SampleType is SAMPLE_RGBA or SAMPLE_NORMAL, the passed r,g,b,a pointers are filled as required
		*/
		float sampleAt(colcom* texData, float x, float y, colcom* r, colcom* g, colcom* b, colcom* a, const SampleInfo& info);


	private:
		// Internal function for adding a given change to some BRDF pointers
		void addChange(bool clamp, colcom change, colcom* diffusePointer, colcom* normalPointer, colcom* roughnessPointer, colcom* metalPointer);

		glm::uvec3 indexToCell(unsigned int index, CellSpawnInfo& info);
		unsigned int cellToIndex(unsigned int cx, unsigned int cy, unsigned int cz, CellSpawnInfo& info);

		// Internal function used for setting write flipping
		void setWriteFlip(bool flip);
		void setWriteFlip() { return setWriteFlip(false); }
		
		// Internal function used for getting an index into an image pointer
		unsigned int getIndex(unsigned int px, unsigned int py, unsigned int width, unsigned int height, unsigned int channels, bool flipX, bool flipY);
		unsigned int getIndex(unsigned int px, unsigned int py, unsigned int width, unsigned int height, unsigned int channels){
			return getIndex(px, py, width, height, channels, false, false);
		}

	};

}