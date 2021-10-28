#version 330 core
in float lenColorScale;
out vec4 fragColor;


void main()
{
    fragColor = vec4(1.0, 1.0, 1.0, lenColorScale);
}