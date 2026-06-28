#version 460 core

out vec4 FragColor;

uniform dvec2 center;
uniform double zoom;
uniform vec2 resolution;
uniform int maxIterations;

vec3 palette[16] = vec3[](
    vec3(0.0,   7.0,   100.0) / 255.0,
    vec3(12.0,  44.0,  138.0) / 255.0,
    vec3(24.0,  82.0,  177.0) / 255.0,
    vec3(57.0,  125.0, 209.0) / 255.0,
    vec3(134.0, 181.0, 229.0) / 255.0,
    vec3(211.0, 236.0, 248.0) / 255.0,
    vec3(241.0, 250.0, 255.0) / 255.0,
    vec3(255.0, 255.0, 255.0) / 255.0,
    vec3(255.0, 238.0, 170.0) / 255.0,
    vec3(255.0, 215.0, 85.0) / 255.0,
    vec3(255.0, 170.0, 0.0) / 255.0,
    vec3(230.0, 135.0, 0.0) / 255.0,
    vec3(190.0, 95.0, 0.0) / 255.0,
    vec3(140.0, 70.0, 0.0) / 255.0,
    vec3(90.0, 40.0, 0.0) / 255.0,
    vec3(0.0, 7.0, 100.0) / 255.0
);

void main()
{
    vec2 uv = gl_FragCoord.xy / resolution;

    const double BASE_HEIGHT = 3.0;

    double aspect = double(resolution.x) / double(resolution.y);

    double height = BASE_HEIGHT / zoom;
    double width  = height * aspect;  

    double real = center.x - width / 2.0 + double(uv.x) * width;
    double imag = center.y - height / 2.0 + double(uv.y) * height;

    dvec2 c = dvec2(real, imag);

    dvec2 z = dvec2(0.0);

    int iter = 0;

    for (iter = 0; iter < maxIterations; iter++)
    {
        double x = z.x;
        double y = z.y;

        double newX = x * x - y * y + c.x;
        double newY = 2.0 * x * y + c.y;

        z = dvec2(newX, newY);

        if (dot(z, z) >= 4.0)
            break;
    }
    double t;

    if (iter == maxIterations)
    {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
    else
    {
        double mag2 = dot(z, z);

        float smoothIter =
            float(iter) + 1.0
            - log2(0.5 * log(float(mag2)));

        t = double(smoothIter) / double(maxIterations);

        const double PALETTE_REPEAT = 6.0;

        double p = t * PALETTE_REPEAT;
        p = fract(p);

        double position = p * 15.0;

        int left = int(floor(position));
        int right = min(left + 1, 15);

        float fraction = float(fract(position));

        vec3 color = mix(
            palette[left],
            palette[right],
            fraction
        );

        color = pow(color, vec3(0.85));

        FragColor = vec4(color, 1.0);
    }
}