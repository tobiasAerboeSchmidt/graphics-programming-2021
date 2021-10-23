#version 330 core
// FRAGMENT SHADER

// TODO voronoi 1.3
out vec4 fragColor;
uniform vec3 color;

void main()
{
    fragColor = vec4(color, 1.0); // CODE HERE
}