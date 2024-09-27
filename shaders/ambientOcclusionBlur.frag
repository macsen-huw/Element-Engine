#version 430

layout (binding = 0) uniform sampler2D depthBufferTexture;
layout (binding = 1) uniform sampler2D ambientOcclusionTexture;

out vec4 fragColour;

uniform ivec2 screenSize;
uniform vec2 sampleDir;

const float gaussianValues[9] = {
    0.0002,
    0.0060,
    0.0606,
    0.2417,
    0.3829,
    0.2417,
    0.0606,
    0.0060,
    0.0002
};


void main() {
    //    cameraToWorldMatrix = inverse(cameraMatrix);

    vec4 colour = vec4(0, 0, 0,0);
    vec2 sampleCoords = gl_FragCoord.xy - (vec2(4, 4) * sampleDir);
    for (int i = 0; i < 9; i++){
        colour += gaussianValues[i] * texture(ambientOcclusionTexture, (sampleCoords + (vec2(i, i) * sampleDir)) / screenSize);
    }

    fragColour = colour;
//    fragColour = texture(ambientOcclusionTexture, gl_FragCoord.xy / screenSize);


    //    vec4 screenPos = vec4(screenCoords, depth, 1);
//    fragPos = cameraToWorldMatrix * screenPos;
//    fragPos.xyz /= fragPos.w;

}
