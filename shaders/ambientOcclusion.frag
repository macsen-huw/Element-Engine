#version 430

#define NUM_AMBIENT_OCCLUSION_SAMPLES 8

uniform vec4 poissonSphere[NUM_AMBIENT_OCCLUSION_SAMPLES];
uniform mat4 cameraMatrix;
uniform ivec2 screenSize;
uniform bool mirror;

layout (binding = 0) uniform sampler2D depthBufferTexture;
layout (binding = 1) uniform sampler2DArray diffuseTexture;

in vec4 fragPos;
in vec3 fragNormal;
in vec3 fragTexCoords;

out vec4 fragColour;



//pseudorandom number generator, returns value in range [0, 1]
//is only random between fragments, if you call this multiple times for the same fragment you will get the same output
//taken from https://thebookofshaders.com/10/
float random (vec2 resolution) {
    vec2 st = gl_FragCoord.xy / resolution;
    st *= dot(vec3(st, 1), fragPos.xyz);
    return fract(cos(dot(st.xy, vec2(12.9898,78.233)))* 43758.5453123);
}


//function calcualtes a rotation matrix to align 2 vectors
//taken from https://gist.github.com/kevinmoran/b45980723e53edeb8a5a43c49f134724
mat4 rotateAlign( vec3 v1, vec3 v2) {

    vec3 axis = cross( v1, v2 );

    const float cosA = dot( v1, v2 );
    const float k = 1.0f / (1.0f + cosA);

    vec4 c1 = vec4(
        (axis.x * axis.x * k) + cosA,
        (axis.y * axis.x * k) - axis.z,
        (axis.z * axis.x * k) + axis.y,
        0
    );

    vec4 c2 = vec4(
        (axis.x * axis.y * k) + axis.z,
        (axis.y * axis.y * k) + cosA,
        (axis.z * axis.y * k) - axis.x,
        0
    );

    vec4 c3 = vec4(
        (axis.x * axis.z * k) - axis.y,
        (axis.y * axis.z * k) + axis.x,
        (axis.z * axis.z * k) + cosA,
        0
    );

    vec4 c4 = vec4(0, 0, 0, 1);


    mat4 result;
    result[0] = c1;
    result[1] = c2;
    result[2] = c3;
    result[3] = c4;
    result = transpose(result);

    return result;
}

mat4 getNormalRotationmatrix(float angle){
    mat4 rotationMatrix;
    float cosA = cos(angle);
    float sinA = sin(angle);
    rotationMatrix[0] = vec4(cosA,
                             0,
                             sinA,
                             0);

    rotationMatrix[1] = vec4(0, 1, 0, 0);

    rotationMatrix[2] = vec4(-sinA,
                             0,
                             cosA,
                             0);

    rotationMatrix[3] = vec4(0, 0, 0, 1);
    return rotationMatrix;
}




float calculateAmbientOcclusion(){

    vec3 normal = normalize(fragNormal);

    vec3 startingNormal = vec3(0, 1, 0);
    mat4 rotationMatrix1 = mat4( 1, 0, 0, 0,
                                 0, -1, 0, 0,
                                 0, 0, 1, 0,
                                 0, 0, 0, 1);

    mat4 rotationMatrix2 = rotateAlign(startingNormal, normal);

    mat4 matrices[2] = {rotationMatrix1, rotationMatrix2};
    mat4 rotationMatrix = mat4(matrices[int(dot(startingNormal, normal) > -0.999f)]);

    float sampleRadius = 0.25;
    rotationMatrix = rotationMatrix * getNormalRotationmatrix(2 * radians(180.f) * random(screenSize));

    float ambientOcclusion = 0.f;

    for (int i = 0; i < NUM_AMBIENT_OCCLUSION_SAMPLES; i++){

        vec4 point = vec4(rotationMatrix * poissonSphere[i]) * sampleRadius;

        vec4 sampleCoordsWorld = fragPos + point;
        vec4 sampleCoordsProj = cameraMatrix * sampleCoordsWorld;
        sampleCoordsProj.xyz /= sampleCoordsProj.w;
        sampleCoordsProj = (sampleCoordsProj + 1) * 0.5; //transform from [-1, 1] to [0, 1]

        vec2 depthSample = sampleCoordsProj.xy;
        float sampleDepth = texture(depthBufferTexture, depthSample).x;

        float mult = 1;
        if (sampleDepth < sampleCoordsProj.z){
            mult = distance(vec3(sampleCoordsWorld.xyz), vec3(fragPos.xyz)) / (sampleRadius * 8);
        }

        ambientOcclusion += mult;
    }
    return ambientOcclusion / float(NUM_AMBIENT_OCCLUSION_SAMPLES);
}



void main() {
    float alpha = texture(diffuseTexture, fragTexCoords).a;
    if (alpha < 0.1) discard;
    fragColour = vec4(vec3(calculateAmbientOcclusion()), 1);
}
