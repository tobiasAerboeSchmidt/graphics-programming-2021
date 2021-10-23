#version 330 core
// FRAGMENT SHADER

// TODO voronoi 1.4
out vec4 fragColor;
in float zPos;

void main()
{
    float color = zPos * -1 + 1;
    fragColor = vec4(color, color ,color, 1.0); // CODE HERE
}