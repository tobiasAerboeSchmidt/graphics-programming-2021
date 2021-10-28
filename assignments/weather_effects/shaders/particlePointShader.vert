#version 330 core
layout (location = 0) in vec3 pos;

uniform mat4 model;
uniform vec3 camPosition;
uniform vec3 forwardOffset;
uniform vec3 combinedOffset;
uniform float precipitationSize;
uniform float boxSize;


void main()
{
    vec3 newPos = mod(pos + combinedOffset, boxSize);
    // Convert world space coordinates to screen space
    newPos += camPosition + forwardOffset - boxSize/2;

    gl_Position = model * vec4(newPos, 1.0);


    // Make droplets close to the camera larger than those further away from the camera (distance equal to world space coords to camera position)
    float distanceToCamera = distance(newPos, camPosition);
    gl_PointSize = precipitationSize*20 - mix(precipitationSize, precipitationSize*6, distanceToCamera / 10);
}