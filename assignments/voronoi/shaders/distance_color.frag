#version 330 core
// FRAGMENT SHADER

// TODO voronoi 1.5
out vec4 fragColor;
in float zPos;
uniform vec3 color;

void main()
{
    fragColor = vec4(color.r * zPos, color.g * zPos, color.b * zPos, 1.0);


}