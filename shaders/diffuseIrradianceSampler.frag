#version 430

in vec3 textureCoords;

uniform int numSamples;

layout (binding = 0) uniform samplerCube skyboxTexture;
layout (location = 0) out vec4 fragColour;


vec3 sphericalToCartesian(vec2 dir){
    vec3 cartesian = vec3(
        sin(dir.x) * cos(dir.y),
        cos(dir.x),
        sin(dir.x) * sin(dir.y)
    );

    return cartesian;
}


//function calcualtes a rotation matrix to align 2 vectors
//taken from https://gist.github.com/kevinmoran/b45980723e53edeb8a5a43c49f134724
mat4 rotateAlign( vec3 v1, vec3 v2) {


    mat4 I = mat4(
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1);

    if (v1 == v2) return I;
    if (v1 == -v2) return -I;

    vec3 axis = cross(v1, v2);

    const float cosA = dot(v1, v2);
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




void main() {

    vec3 fragNormal = normalize(textureCoords);
    
    //practical global illumination with irradiance caching recommends that the number of samples in the theta axis (n)
    //should be roughly equal to pi * m, where m is the number of samples in the azimuthal direction
    //this is in the context of irradiance caching, so may not necessarily be the best choice here
    //but its what im familiar with, so its being used for now at least
    const float pi = radians(180.f);

    vec3 irradiance = vec3(0, 0, 0);

    mat4 rotationMatrix = rotateAlign(vec3(0, 1, 0), fragNormal);

    float thetaIncrement = 2.f * pi / float(numSamples * pi);
    float azimuthIncrement = 1.f / float(numSamples);
    float theta = thetaIncrement / 2.f;
    float azimuth = azimuthIncrement / 2.f;


    for (int i = 0; i < numSamples * pi; i++){
        for (int j = 0; j < numSamples; j++){

            vec2 sphericalSample = vec2(
                acos(sqrt(azimuth)),
		        theta
            );

            vec3 cartesianDir = normalize(sphericalToCartesian(sphericalSample));
            cartesianDir = vec3(rotationMatrix * vec4(cartesianDir, 1));

            irradiance += texture(skyboxTexture, cartesianDir).rgb;
            azimuth += azimuthIncrement;
        }
        azimuth = azimuthIncrement / 2.f;
        theta += thetaIncrement;
    }

    fragColour = vec4(irradiance / float(numSamples * numSamples * pi), 1);
}








