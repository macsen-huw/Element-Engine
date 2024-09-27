#version 430

layout(location = 0) in vec3 vertexPos;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 textureCoords;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 biTangent;

layout(location = 5) in ivec4 boneIds;
layout(location = 6) in vec4 weights;

//use a shader storage buffer object to pass bone matrices instead of regular uniforms
//pros: uses alternative storage, so we shouldn't overcrowd uniform storage (ie registers)
//cons: slightly slower, uses a binding point
layout(std430, binding = 10) buffer boneMatricesLayout {
    mat4 boneMatrices[];
};


uniform mat4 rotationMatrix;
uniform mat4 instanceMatrix;

uniform mat4 transformationMatrix;
uniform mat4 modelMatrix;

uniform float textureLevel;

// Max number of bones supported for animation
// Use same value as in Render Manager
const int MAX_BONES = 256; 

// Max vertices a bone can affect
// Use same value as in Mesh Class
const int MAX_BONE_INFLUENCE = 4;

uniform int boneCount;

out vec3 fragNormal;
out vec3 fragTexCoords;
out vec4 fragPos;

out mat3 tbnMatrix;
flat out ivec4 fragBoneIds;
out vec4 fragWeights;

void main() {
    // calculate vertex positions wrt bones
    vec4 transformedVertexPos =  vec4(vertexPos, 1.0f);
    vec3 transformedNormal = vertexNormal;

    if (boneCount > 0) {
        mat4 BoneTransform = boneMatrices[boneIds[0]] * weights[0];
        BoneTransform     += boneMatrices[boneIds[1]] * weights[1];
        BoneTransform     += boneMatrices[boneIds[2]] * weights[2];
        BoneTransform     += boneMatrices[boneIds[3]] * weights[3];

        transformedVertexPos = BoneTransform * transformedVertexPos;
        transformedNormal = mat3(BoneTransform) * transformedNormal;
    }

    gl_Position =  transformationMatrix * transformedVertexPos;

    fragPos = modelMatrix * transformedVertexPos;
    fragNormal = normalize(vec3(rotationMatrix * vec4(transformedNormal, 1)));
    fragTexCoords = vec3(textureCoords, textureLevel);

    vec3 t = normalize(mat3(modelMatrix) * tangent);
    vec3 b = normalize(mat3(modelMatrix) * biTangent);
    vec3 n = normalize(mat3(modelMatrix) * fragNormal);

    tbnMatrix = mat3(t, b, n);
    fragBoneIds = boneIds;
    fragWeights = weights;
}
