#version 430


in vec3 textureCoords;

layout (binding = 0) uniform samplerCube skyboxTexture;

layout (location = 0) out vec4 fragColour;

void main() {
    fragColour = texture(skyboxTexture, textureCoords);
}
