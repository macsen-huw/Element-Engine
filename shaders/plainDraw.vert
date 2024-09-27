#version 430

layout(location = 0) in vec3 vertexPos;
layout(location = 1) in float vertexVal;

out float fragVal;
uniform mat4 transformationMatrix;

void main() {
    gl_Position = transformationMatrix * vec4(vertexPos, 1);
    fragVal = vertexVal;
}