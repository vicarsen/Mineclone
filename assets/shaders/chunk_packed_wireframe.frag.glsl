#version 450 core

#define THICKNESS 0.005

in vec3 barycentric;

layout(location = 0) out vec4 out_color;

void main()
{
    float f_closest_edge = min(barycentric.x, min(barycentric.y, barycentric.z));
    float f_width = fwidth(f_closest_edge);
    float f_alpha = smoothstep(THICKNESS, THICKNESS + f_width, f_closest_edge);
    out_color = vec4(1.0 - f_alpha, 1.0 - f_alpha, 1.0 - f_alpha, 1.0);
}

