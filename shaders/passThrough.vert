#version 430

layout(location = 0) in vec3 vertexPos;
layout(location = 1) in vec2 textureCoords;
layout(location = 2) in ivec4 boneIds;
layout(location = 3) in vec4 weights;

uniform mat4 transformationMatrix;
uniform float textureLevel;

// Max number of bones supported for animation
// Use same value as in Render Manager
const int MAX_BONES = 256;

// Max vertices a bone can affect
// Use same value as in Mesh Class
const int MAX_BONE_INFLUENCE = 4;

uniform int boneCount;
layout(std430, binding = 1) buffer boneMatricesLayout {
    mat4 boneMatrices[];
};

out vec3 fragTexCoords;

void main() {
    // calculate vertex positions wrt bones
    vec4 transformedVertexPos = vec4(vertexPos, 1.0f);

    if (boneCount > 0) {
        mat4 BoneTransform = boneMatrices[boneIds[0]] * weights[0];
        BoneTransform     += boneMatrices[boneIds[1]] * weights[1];
        BoneTransform     += boneMatrices[boneIds[2]] * weights[2];
        BoneTransform     += boneMatrices[boneIds[3]] * weights[3];

        transformedVertexPos = BoneTransform * transformedVertexPos;
    }

    gl_Position =  transformationMatrix * transformedVertexPos;

    fragTexCoords = vec3(textureCoords, textureLevel);
}
