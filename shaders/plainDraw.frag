#version 430

in float fragVal;

uniform vec4 colour;

out vec4 fragColour;

void main() {

    if (fragVal < 0.005) discard;
    fragColour = colour;
}