#version 460 core

out vec4 FragColor;

uniform vec2 center;
uniform float zoom;
uniform vec2 resolution;
uniform int maxIterations;

void main()
{
    vec2 uv = gl_FragCoord.xy / resolution;

    float width = 3.5 / zoom;
    float height = 3.0 / zoom;  

    float real = center.x - width / 2.0 + uv.x * width;
    float imag = center.y - height / 2.0 + uv.y * height;

    vec2 c = vec2(real, imag);

    vec2 z = vec2(0.0);

    int iter = 0;

    for (iter = 0; iter < maxIterations; iter++)
    {
        float x = z.x;
        float y = z.y;

        float newX = x * x - y * y + c.x;
        float newY = 2.0 * x * y + c.y;

        z = vec2(newX, newY);

        if (dot(z, z) > 4.0)
            break;
    }
    if (iter == maxIterations)
    {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
    else
    {
        FragColor = vec4(1.0, 1.0, 1.0, 1.0);
    }
}