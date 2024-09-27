#version 460

#define DIRECTIONAL_LIGHT_BINDING 0
#define POINT_LIGHT_BINDING 1
#define LIGHT_TRANSFORM_BINDING 4
#define CASCADE_BOUNDARY_BINDING 5

#define NUM_POISSON_SAMPLES 12
#define NUM_AMBIENT_OCCLUSION_SAMPLES 8


#define MAX_LIGHTS 1024
#define MAX_CASCADES 1024

#define RENDER_MODE_DEFAULT 0
#define RENDER_MODE_NORMALS 1
#define RENDER_MODE_AMBIENT_OCCLUSION 2
#define RENDER_MODE_ENVIRONMENT_LIGHT 3
#define RENDER_MODE_RGBA_TEXTURE 4
#define RENDER_MODE_SHADOWS_ONLY 5
#define RENDER_MODE_MIPMAP_LEVELS 6
#define RENDER_MODE_BLINN_PHONG 7
#define RENDER_MODE_DIFFUSE_ONLY 8
#define RENDER_MODE_SPECULAR_ONLY 9
#define RENDER_MODE_AMBIENT_ONLY 10
#define RENDER_MODE_ROUGHNESS_ONLY 11
#define RENDER_MODE_METALLICNESS_ONLY 12
#define RENDER_MODE_PARALLAX_ONLY 13


#define BLINN_PHONG_BRDF 0
#define COOK_TORRANCE_BRDF 1



struct PointLight{
    vec4 pos;
    vec4 colour;
};

struct DirectionalLight{
    vec4 dir;
    vec4 colour;
};



layout (std140, binding = DIRECTIONAL_LIGHT_BINDING) uniform directionalLightsBuffer{
    DirectionalLight directionalLights[MAX_LIGHTS];
};

layout (std140, binding = POINT_LIGHT_BINDING) uniform pointLightsBuffer{
    PointLight pointLights[MAX_LIGHTS];
};

layout (std140, binding = LIGHT_TRANSFORM_BINDING) uniform lightTransformsBuffer{
    mat4 lightTransforms[MAX_CASCADES];
};

layout (std140, binding = CASCADE_BOUNDARY_BINDING) uniform cascadeBoundaryBuffer{
    vec4 boundaries[MAX_CASCADES]; //ideally, this would be floats - but alignment forces us to use vec4
};


uniform ivec2 screenSize;


uniform int numDirectionalLights;
uniform int numPointLights;
uniform int numCascadePartitions;

uniform mat4 cameraMatrix;
uniform mat4 viewMatrix;

uniform vec3 diffuseColour;
uniform vec3 specularColour;
uniform vec3 ambientColour;
uniform vec3 emissiveColour;
uniform float shininess;
uniform float ambientIntensity;
uniform float pcfRadius;

uniform vec3 cameraPos;

uniform float viewSpaceBoundaries;

uniform vec3 mirrorNormal;
uniform vec3 mirrorPoint;

uniform bool useMirror;
uniform bool useTexture;
uniform bool useNormalMap;
uniform bool useParallaxMap;
uniform bool useAmbientOcclusionTexture;
uniform bool useSkybox;

uniform int brdf;


uniform vec2 poissonDisc[NUM_POISSON_SAMPLES];
uniform vec4 poissonSphere[NUM_AMBIENT_OCCLUSION_SAMPLES];
uniform int renderMode;

uniform float normalTextureLevel;
uniform float roughnessTextureLevel;
uniform float metallicnessTextureLevel;
uniform float parallaxTextureLevel;
uniform float textureLevel;
uniform float ambientOcclusionTextureLevel;

uniform int renderBoneId;
uniform float parallaxHeight;
uniform float parallaxAccuracy;

in vec3 fragNormal;
in vec3 fragTexCoords;
in vec4 fragPos;

uniform float lodAlphaBlend;

in mat3 tbnMatrix;
flat in ivec4 fragBoneIds;
in vec4 fragWeights;

layout (binding = 2) uniform sampler2DArray diffuseTexture;
layout (binding = 3) uniform sampler2DArrayShadow shadowTexture;
layout (binding = 6) uniform sampler2D screenSpaceAmbientOcclusionTexture;
layout (binding = 7) uniform samplerCube skyboxTexture;
layout (binding = 8) uniform sampler2DArray normalMapTexture;
layout (binding = 9) uniform sampler2DArray roughnessTexture;
layout (binding = 10) uniform sampler2DArray metallicnessTexture;
layout (binding = 11) uniform sampler2DArray parallaxTexture;
layout (binding = 12) uniform sampler2DArray ambientOcclusionTexture;


layout (location = 0) out vec4 fragColour;

vec2 textureSampleCoords = vec2(0, 0);

const float blendZone = 1.2f;
float blend = 0.f;

vec3 viewDirection = vec3(0, 0, 0);
vec3 calculatedNormal = vec3(1.0, 0.0, 0.0);

//pseudorandom number generator, returns value in range [0, 1]
//is only random between fragments, if you call this multiple times for the same fragment you will get the same output
//taken from https://thebookofshaders.com/10/
float random (vec2 resolution) {
    vec2 st = gl_FragCoord.xy / resolution;
    st *= dot(vec3(st, 1), fragPos.xyz);
    return fract(cos(dot(st.xy, vec2(12.9898,78.233)))* 43758.5453123);
}


//secant method for parallax mapping
//the goal is to find the root of the function f(x)
//where f(x) = (x * v.z) - h(x * v.x, x * v.y)
//ie, f(x) is the difference between the height of the view vector and the height of the height map for view vector of length x
//when f(x) = 0, x represents the exact length of the view vector such that it perfectly aligns with the height map

float sampleHeightMap(vec2 sampleCoords){
    return (texture(parallaxTexture, vec3(sampleCoords, parallaxTextureLevel)).x) * parallaxHeight;
}

float parallaxEval(vec3 viewVec, float x){
    vec2 sampleCoords = fragTexCoords.xy + (viewVec.xy * x);
    return (x * viewVec.z) - sampleHeightMap(sampleCoords);
}

float parallaxSecantMethod(vec3 viewVec, float x0, float x1){

    float xCurrent = 0; // = Xn
    float xOld1 = x1; // = Xn-1
    float xOld2 = x0; // = Xn-2

    for (int i = 0; i < 10 && abs(xCurrent - xOld1) > 0; i++){
        float top = xOld1 - xOld2;
        float fxOld1 = parallaxEval(viewVec, xOld1);
        float bottom = fxOld1 - parallaxEval(viewVec, xOld2);
        float ratio = fxOld1 * (top / bottom);
        xCurrent = xOld1 - ratio;
        xOld2 = xOld1;
        xOld1 = xCurrent;
    }

    return xCurrent;
}


float parallaxBisectionMethod(vec3 viewVec, float x0, float x1) {

    float xLeft = x0;
    float xRight = x1;

    float xCurrent = 0;

    for (int i = 0; i < 10; i++) {
        xCurrent = (xLeft + xRight) / 2.f;
        float val = parallaxEval(viewVec, xCurrent);

        float newValsLeft[2] = {xLeft, xCurrent};
        float newValsRight[2] = {xCurrent, xRight};

        xLeft = newValsLeft[int(val < 0)];
        xRight = newValsLeft[int(val < 0)];
    }

    return xCurrent;
}


float getParallaxRayMarchStart(vec3 viewDir){
    return parallaxHeight / viewDir.z;
}


vec2 parallaxSampleCoords(){
    mat3 transposeTbn = transpose(tbnMatrix);
    vec3 cameraPosTangentSpace = transposeTbn * cameraPos;
    vec3 fragPosTangentSpace = transposeTbn * fragPos.xyz;
    vec3 viewDirTangentSpace = normalize(cameraPosTangentSpace - fragPosTangentSpace); //to do: make sure tbn is orthogonal and we can just transpose

    const float start = getParallaxRayMarchStart(viewDirTangentSpace);
    const float incrementSize = (1.f / parallaxAccuracy) * max(0.002f, start / 200.f);

    float current = start - incrementSize;
    while (parallaxEval(viewDirTangentSpace, current) > 0){
        current -= incrementSize;
    }

    float x0 = current;
    float x1 = current + incrementSize;

    float parallaxHeight = parallaxBisectionMethod(viewDirTangentSpace, x0, x1);

    vec2 shift = parallaxHeight * viewDirTangentSpace.xy;
    return shift;
}



void calculateNormal(){
    if (useNormalMap) {
        vec3 normal = texture(normalMapTexture, vec3(textureSampleCoords, normalTextureLevel)).rgb;
        normal = (normal * 2) - vec3(1);
        calculatedNormal = normalize(tbnMatrix * normal);
        return;
    }
    calculatedNormal = fragNormal;
}


//creates 2d rotation matrix, remember opengl matrices column major (why??)
mat2 getRotationMatrix(float angle){
    mat2 rotationMatrix;
    rotationMatrix[0] = vec2(cos(angle), sin(angle));
    rotationMatrix[1] = vec2(-sin(angle), cos(angle));
    return rotationMatrix;
}


vec3 getDiffuseColour(){
    return useTexture ? texture(diffuseTexture, vec3(textureSampleCoords, textureLevel)).rgb : diffuseColour;
}


vec3 getAmbientColour(){
    return useTexture ? texture(diffuseTexture, vec3(textureSampleCoords, textureLevel)).rgb : ambientColour;
}

vec3 getEnvironmentColour(){
    return useSkybox ? normalize(texture(skyboxTexture, calculatedNormal).rgb) : vec3(1, 1, 1);
}

float getViewSpaceDepth(){
    vec4 viewSpacePos = viewMatrix * fragPos;
    return viewSpacePos.z;
}


//iterates through the boundaries of the view frustum in view space to get the index of the appropriate shadow map
int getShadowLayer(int light){

    float depth = getViewSpaceDepth();

    for (int i = 0; i < numCascadePartitions; i++){
        float ratio = depth / boundaries[i].x;
        if (ratio < 1) return i;
        if (ratio < blendZone) {
            blend = (ratio - 1) / (blendZone - 1);
            return i;
        }
    }

    return numCascadePartitions - 1;
}




//returns 0 if the fragment is in shade, else returns 1
float checkShadow(vec3 coords, int lightIndex){

//    if (coords.x > 1 || coords.x < 0 || coords.y > 1 || coords.y < 0) return 1;

    float bias = 0.0005;
    vec4 sampleCoords = vec4(coords.xy, lightIndex, coords.z - bias);
    float mult = texture(shadowTexture, sampleCoords);
    return mult;
}


//returns the sample coords for a given light
vec3 getShadowCoords(int lightIndex){
    vec4 lightSpaceFragPos = lightTransforms[lightIndex] * fragPos;
    vec3 coords = vec3((lightSpaceFragPos.xyz) / lightSpaceFragPos.w); //perspective divsion

    coords = (coords + 1) * 0.5; //transform from [-1, 1] to [0, 1]

    return coords;
}



float sampleShadow(int lightIndex, vec3 sampleCoords){
    float val = 0;

    vec2 shadowSize = vec2(1024, 1024);

    for (int i = 0; i < NUM_POISSON_SAMPLES; i++){
        vec2 poissonSample = vec2(poissonDisc[i].x, poissonDisc[i].y);
        mat2 rotationMatrix = getRotationMatrix(2 * radians(180.f) * random(shadowSize));
        poissonSample = rotationMatrix * poissonSample;

        vec3 offset = vec3(poissonSample.xy, 0.f) * pcfRadius;
        val += checkShadow(sampleCoords + offset, lightIndex);

    }

    return val / float(NUM_POISSON_SAMPLES);
}


//returns a float corresponding to how in shadow the fragment is for the given light
//a value of 0 means its completely in shade
//a value of 1 means its not in any shade

float getShadowMult(int light){
    int lightIndex = getShadowLayer(light);
    vec3 sampleCoords = getShadowCoords(lightIndex);

    if (blend == 0.f) return sampleShadow(lightIndex, sampleCoords);

    vec3 sampleCoords2 = getShadowCoords(lightIndex + 1);

    return ((1.f - blend) * sampleShadow(lightIndex, sampleCoords)) + (blend * sampleShadow(lightIndex + 1, sampleCoords2));
}


vec3 schlickApproximationFresnel(vec3 fresnelVal, float cosA){
    float mult = pow(clamp(1.0 - cosA, 0.0, 1.0), 5.0);
    return fresnelVal + (1.0 - fresnelVal) * mult;
}

vec3 calculateFresnelVal(vec3 f0, vec3 surfaceColour, float metallicValue){
    return mix(f0, surfaceColour, metallicValue);
}

//taken from learnopengl
float calculateNormalTerm(float cosA, float roughness){
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = cosA;
    float NdotH2 = NdotH*NdotH;

    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = radians(180.f) * denom * denom;

    return num / denom;
}


float schlickGGX(vec3 normal, vec3 viewDir, float roughnessConstant){
    float nDotV = max(dot(normal, viewDir), 0.f);
    return nDotV / (nDotV * (1 - roughnessConstant) + roughnessConstant);
}


float calculateGeometryTerm(vec3 normal, vec3 viewDir, vec3 lightDir, float roughnessConstant){
    return schlickGGX(normal, viewDir, roughnessConstant) * schlickGGX(normal, lightDir, roughnessConstant);
}


vec3 calculateCookTorrance(vec3 lightDir, vec3 lightColour, int light){

    float shadowMult = getShadowMult(light);

    if (renderMode == RENDER_MODE_SHADOWS_ONLY){
        return vec3(shadowMult);
    }

    vec3 viewDir = -1 * viewDirection;
    vec3 halfVector = normalize(viewDir + lightDir);

    if (shadowMult == 0) return vec3(0, 0, 0);

    float roughnessConstant = texture(roughnessTexture, vec3(textureSampleCoords, roughnessTextureLevel)).x;
    float metallicValue = texture(metallicnessTexture, vec3(textureSampleCoords, metallicnessTextureLevel)).x;

    float geometryVal = calculateGeometryTerm(calculatedNormal, viewDir, lightDir,  (roughnessConstant + 1) * (roughnessConstant + 1) / 8.f);
    float normalVal = calculateNormalTerm(max(dot(halfVector, calculatedNormal), 0.f), roughnessConstant);

    vec3 f0 = calculateFresnelVal(vec3(0.04), getDiffuseColour(), metallicValue);
    vec3 fresnelVal = schlickApproximationFresnel(f0, max(dot(halfVector, viewDir), 0.f));

    vec3 top = geometryVal * normalVal * fresnelVal;
    float bottom = 4 * max(dot(viewDir, calculatedNormal), 0.f) * max(dot(lightDir, calculatedNormal), 0.f);

    float epsilon = 0.0005f;

    vec3 cookTorrance = top / (bottom + epsilon);
    vec3 diffuse = getDiffuseColour();

    if (renderMode == RENDER_MODE_DIFFUSE_ONLY){
        cookTorrance = vec3(0, 0, 0);
    }

    if (renderMode == RENDER_MODE_SPECULAR_ONLY){
        diffuse = vec3(0, 0, 0);
    }



    return (cookTorrance + diffuse) * lightColour * max(dot(calculatedNormal, lightDir), 0.f) * shadowMult;
}




//calculates the blinn phong shading model

vec3 calculateBlinnPhong(vec3 lightDir, vec3 lightColour, int light){

    if (dot(lightDir, fragNormal) < 0.005){
        return vec3(0, 0, 0);
    }

    float shadowMult = getShadowMult(light);
    if (isnan(shadowMult) || shadowMult < 0) shadowMult = 0;
    if (shadowMult > 1) shadowMult = 1;

    if (renderMode == RENDER_MODE_SHADOWS_ONLY){
        return vec3(shadowMult);
    }

    if (shadowMult == 0) return vec3(0, 0, 0);

    float diffuseMult = max(dot(calculatedNormal, lightDir), 0.f);
    vec3 diffuseContribution = diffuseMult * getDiffuseColour();

    vec3 halfVector = normalize((-1 * viewDirection) + lightDir);
    float specularMult = max(dot(halfVector, calculatedNormal), 0.f);

    float shine = shininess == 0.f ? 1.f : shininess;
    specularMult = pow(specularMult, shine);
    vec3 specularContribuion = specularMult * specularColour * getDiffuseColour();


    if (renderMode == RENDER_MODE_DIFFUSE_ONLY){
        specularContribuion = vec3(0, 0, 0);
    }

    if (renderMode == RENDER_MODE_SPECULAR_ONLY){
        diffuseContribution = vec3(0, 0, 0);
    }


    vec3 lightContribution = (diffuseContribution + specularContribuion) * lightColour * shadowMult;
    return lightContribution;
}


float calculateAmbientOcclusion(){
    float ssao = texture(screenSpaceAmbientOcclusionTexture, gl_FragCoord.xy / screenSize).x;
    if (!useAmbientOcclusionTexture) return ssao;

    float aoMap = texture(ambientOcclusionTexture, vec3(textureSampleCoords, ambientOcclusionTextureLevel)).x;
    return ssao * aoMap;
}


void main() {

    viewDirection = normalize(fragPos.xyz - cameraPos);

    vec2 shift = useParallaxMap ? parallaxSampleCoords() : vec2(0, 0);
    textureSampleCoords = fragTexCoords.xy + shift;

    calculateNormal();

    //check we're on the right side of the mirror (if mirror rendering)
    //this is very hacky, and should be done more cleanly (geometry shader?) in the future, but i am sleepy
    if (useMirror){
        vec3 fragDir = normalize(fragPos.xyz - mirrorPoint);
        float x = dot(fragDir, mirrorNormal);
        if (x > 0) discard;
    }


    //check if the fragment is transparent
    float alpha = 1;

    if (useTexture) alpha = texture(diffuseTexture, vec3(textureSampleCoords, textureLevel)).a;

    alpha *= lodAlphaBlend;

    if (alpha <= 0.001) {
        discard;
    }

    if (renderMode == RENDER_MODE_NORMALS){
        fragColour = vec4(abs(calculatedNormal), 1);
        return;
    }

    if (renderMode == RENDER_MODE_ENVIRONMENT_LIGHT){
        fragColour = vec4(getEnvironmentColour(), 1);
        return;
    }

    if (renderMode == RENDER_MODE_AMBIENT_OCCLUSION){
        fragColour = vec4(vec3(calculateAmbientOcclusion()), 1);
        return;
    }

    if (renderMode == RENDER_MODE_RGBA_TEXTURE){
        fragColour = vec4(getDiffuseColour(), 1);
        return;
    }

    if (renderMode == RENDER_MODE_MIPMAP_LEVELS){
        int numLevels = textureQueryLevels(diffuseTexture);
        float mipmapLevel = textureQueryLod(diffuseTexture, textureSampleCoords).x;
        fragColour = vec4(vec3(1.f - ((mipmapLevel + 1) / float(numLevels))), 1);
        return;
    }

    if (renderMode == RENDER_MODE_ROUGHNESS_ONLY){
        if (brdf == BLINN_PHONG_BRDF){
            fragColour = vec4(0, 0, 0, 1);
            return;
        }
        float rougness = texture(roughnessTexture, vec3(textureSampleCoords.xy, roughnessTextureLevel)).x;
        fragColour = vec4(vec3(rougness), 1);
        return;
    }

    if (renderMode == RENDER_MODE_METALLICNESS_ONLY){
        if (brdf == BLINN_PHONG_BRDF){
            fragColour = vec4(0, 0, 0, 1);
            return;
        }

        float metallicness = texture(metallicnessTexture, vec3(textureSampleCoords.xy, metallicnessTextureLevel)).x;
        fragColour = vec4(vec3(metallicness), 1);
        return;
    }

    if (renderMode == RENDER_MODE_PARALLAX_ONLY){
        float height = texture(parallaxTexture, vec3(textureSampleCoords.xy, parallaxTextureLevel)).x;
        fragColour = vec4(vec3(height), 1);
        return;
    }


    vec4 environmentColour = vec4(getEnvironmentColour(), 1);
    vec4 ambientColour = calculateAmbientOcclusion() * ambientIntensity * vec4(getAmbientColour(), 1) * environmentColour;

    if (renderMode == RENDER_MODE_AMBIENT_ONLY){
        fragColour = vec4(ambientColour.xyz, alpha);
        return;
    }

    if (renderMode == RENDER_MODE_DIFFUSE_ONLY || renderMode == RENDER_MODE_SPECULAR_ONLY){
        ambientColour = vec4(0, 0, 0, 1);
    }

    vec3 totalLight = vec3(0, 0, 0);

    //calculate directional light shading
    for (int i = 0; i < numDirectionalLights; i++) {
        vec3 lightDir = (-1 * directionalLights[i].dir).xyz;
        vec3 lightColour = (directionalLights[i].colour.xyz) * directionalLights[i].colour.w;

        if (renderMode == RENDER_MODE_BLINN_PHONG || brdf == BLINN_PHONG_BRDF) {
            totalLight += calculateBlinnPhong(lightDir, lightColour, i);
        }
        else {
            totalLight += calculateCookTorrance(lightDir, lightColour, i);
        }
    }

    //and point light shading
    for (int i = 0; i < numPointLights; i++) {
        vec3 lightDir = pointLights[i].pos.xyz - fragPos.xyz;
        vec3 lightColour = (pointLights[i].colour.xyz) * pointLights[i].colour.w;

        float lightDistance = length(lightDir);
        float lightMult = 1.f / (1.f + (lightDistance * lightDistance));

        if (renderMode == RENDER_MODE_BLINN_PHONG || brdf == BLINN_PHONG_BRDF) {
            totalLight += lightMult * calculateBlinnPhong(normalize(lightDir), lightColour, i);
        }
        else {
            totalLight += lightMult * calculateCookTorrance(lightDir, lightColour, i);
        }
    }

    if (renderMode == RENDER_MODE_SHADOWS_ONLY){
        fragColour.rgb = totalLight;
        fragColour.a = alpha;
        return;
    }

//    bool boneFound = false;
//    for (int i = 0; i < 10; i++) {
//        if (fragBoneIds[i] == renderBoneId) { // bone index
//            if (fragWeights[i] >= 0.7) fragColour = vec4(1.f, 0.f, 0.f, 1.f) * fragWeights[i];
//            else if (fragWeights[i] >= 0.4 && fragWeights[i] < 0.7) fragColour = vec4(0.f, 1.f, 0.f, 1.f) * fragWeights[i];
//            else if (fragWeights[i] >= 0.1) fragColour = vec4(1.f, 1.f, 0.f, 1.f) * fragWeights[i];
//            else fragColour = vec4(0.f, 0.f, 1.f, 1.f);
//            boneFound = true;
//        }
//    }

//    if (!boneFound) {
        //keep the light calculations and the transparency separate
        fragColour.rgb = totalLight + emissiveColour + ambientColour.rgb;
        fragColour.rgb = min(fragColour.rgb, 1.f);
        fragColour.a = alpha;
        
//        fragColour = vec4(0.f, 0.f, 1.f, 1.f);
//    }

}


