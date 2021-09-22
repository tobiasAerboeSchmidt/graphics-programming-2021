#version 330 core
// VERTEX SHADER

// TODO voronoi 1.3
// Receives position in 'vec3', the position variable has attribute 'location = 0'
layout (location = 0) in vec3 aPos;
// You have to declare an 'out float' to send the z-coordinate of the position
// to the fragment shader (voronoi 1.4 and 1.5)
out float zPos;
// You have to set an 'uniform vec2' to receive the position offset of the object
uniform vec2 offset;

void main()
{
    // TODO voronoi 1.3
    // Set the vertex->fragment shader 'out' variable
    zPos = aPos.z;
    // Set the 'gl_Position' built-in variable using a 'vec4(vec3 position you compute, 1.0)',
    // Remeber to use the 'uniform vec2' to move the vertex before you set 'gl_Position'.
    gl_Position = vec4(aPos.x + offset.x, aPos.y + offset.y, aPos.z, 1.0);
}