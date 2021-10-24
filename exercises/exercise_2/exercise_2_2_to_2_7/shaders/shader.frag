#version 330 core

out vec4 fragColor;

// TODO 2.6: should receive the age of the particle as an input variable
in float age;

void main()
{
    // TODO 2.4 set the alpha value to 0.2 (alpha is the 4th value of the output color)
    // float alpha = 0.2f;
    // TODO 2.5 and 2.6: improve the particles appearance
    vec2 vecFromCenter = (gl_PointCoord - vec2(.5, .5)) * 2;
    float distance = sqrt(dot(vecFromCenter, vecFromCenter));
    float alpha = 1.0f - distance;

    // remember to replace the default output (vec4(1.0,1.0,1.0,1.0)) with the color and alpha values that you have computed
    fragColor = vec4(1.0, 1.0, 1.0, alpha);

}