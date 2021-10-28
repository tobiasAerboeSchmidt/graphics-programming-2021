#version 330 core
layout (location = 0) in vec3 pos;
out float lenColorScale;
uniform mat4 model;
uniform mat4 prevModel;
uniform vec3 camPosition;
uniform vec3 forwardOffset;
uniform vec3 combinedOffset;
uniform float boxSize;
uniform float heightScale;
uniform vec3 g_vVelocity;


// Simulate camera motion blur and camera exposure time by drawing the rain as lines
void main()
{

    vec3 newPos = mod(pos + combinedOffset, boxSize);

    newPos += camPosition + forwardOffset - boxSize/2;

    vec4 worldPos = vec4(newPos, 1);
    vec4 worldPosPrev = vec4(newPos, 1) + vec4(g_vVelocity, 1.0) * heightScale;

    worldPosPrev.w = 1.0f;

    vec4 bottom = model * worldPos;

    vec4 top = model * worldPosPrev;
    vec4 topPrev = prevModel * worldPosPrev;

    vec4 finalPos = mix(topPrev, bottom, gl_VertexID % 2);

    vec2 dir = (top.xy/top.w) - (bottom.xy/bottom.w);
    vec2 dirPrev = (topPrev.xy/topPrev.w) - (bottom.xy/bottom.w);

    float len = length(dir);
    float lenPrev = length(dirPrev);

    lenColorScale = clamp(len/lenPrev, 0.0, 1.0);

    gl_Position = finalPos;
}