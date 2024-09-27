#version 430

layout(location = 0) in vec3 vertexPos;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 textureCoords;
layout(location = 5) in ivec4 boneIds;
layout(location = 6) in vec4 weights;


uniform mat4 transformationMatrix;
uniform mat4 modelMatrix;
uniform float textureLevel;

// Use same value as in Render Manager
const int MAX_BONES = 256;

// Max vertices a bone can affect
// Use same value as in Mesh Class
const int MAX_BONE_INFLUENCE = 4;

uniform int boneCount;
layout(std430, binding = 1) buffer boneMatricesLayout {
    mat4 boneMatrices[];
};


out vec4 fragPos;
out vec3 fragNormal;
out vec3 fragTexCoords;

void main() {
    vec4 transformedVertexPos = vec4(vertexPos, 1.0f);
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
    fragNormal = mat3(modelMatrix) * normalize(transformedNormal);
    fragTexCoords = vec3(textureCoords, textureLevel);
}



