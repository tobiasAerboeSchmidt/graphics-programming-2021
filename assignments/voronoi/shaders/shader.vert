#version 330 core
// VERTEX SHADER

// TODO voronoi 1.3
layout (location = 0) in vec3 aPos;
out float zPos;
uniform vec2 offset;

void main()
{
    // TODO voronoi 1.3
    zPos = aPos.z;
    gl_Position = vec4(aPos.x + offset.x, aPos.y + offset.y, aPos.z, 1.0);
}