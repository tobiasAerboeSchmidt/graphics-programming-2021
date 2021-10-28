#version 330 core

out vec4 fragColor;

const float midAge = 5.0;
const float maxAge = 10.0;

void main()
{
    vec2 vecFromCenter = (gl_PointCoord - vec2(.5, .5)) * 2;
    float distance = sqrt(dot(vecFromCenter, vecFromCenter));


    float alpha = 1.0 - distance;

    fragColor = vec4(1.0, 1.0, 1.0, .5);

}