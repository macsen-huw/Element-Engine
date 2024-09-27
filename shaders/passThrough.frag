#version 430

layout (binding = 2) uniform sampler2DArray diffuseTexture;

in vec3 fragTexCoords;

void main() {
    if (texture(diffuseTexture, fragTexCoords).a < 0.1) discard;
}