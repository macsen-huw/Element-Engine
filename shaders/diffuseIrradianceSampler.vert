#version 430

layout(location = 0) in vec3 vertexPos;

out vec3 textureCoords;
uniform mat4 transformationMatrix;

void main() {
    gl_Position = transformationMatrix * vec4(vertexPos, 1);
    textureCoords = vertexPos;
}