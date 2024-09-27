#version 430

layout(location = 0) in vec3 vertexPos;

out vec3 textureCoords;

uniform mat4 transformationMatrix;
uniform mat4 instanceMatrix;

void main() {
    textureCoords = vertexPos;
    gl_Position = transformationMatrix * instanceMatrix * vec4(vertexPos, 1);
}