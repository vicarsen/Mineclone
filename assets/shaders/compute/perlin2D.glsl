#version 430 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout (r32f, binding = 0) uniform writeonly image2D noise;

layout (std430, binding = 0) readonly buffer gradients {
    vec2 grads[];
};

uniform ivec2 resolution;
uniform ivec2 gradients_dimensions;

vec2 GradientAt(ivec2 position)
{
    return grads[position.x + position.y * gradients_dimensions.x];
}

void main()
{
    const ivec2 tex_coords = ivec2(gl_GlobalInvocationID.xy);
    const ivec2 coords = tex_coords / resolution;

    const ivec2 v1 = ivec2(coords.x,     coords.y    );
    const ivec2 v2 = ivec2(coords.x + 1, coords.y    );
    const ivec2 v3 = ivec2(coords.x + 1, coords.y + 1);
    const ivec2 v4 = ivec2(coords.x,     coords.y + 1);

    const float dot1 = dot(GradientAt(v1), vec2(tex_coords - v1 * resolution) / vec2(resolution));
    const float dot2 = dot(GradientAt(v2), vec2(tex_coords - v2 * resolution) / vec2(resolution));
    const float dot3 = dot(GradientAt(v3), vec2(tex_coords - v3 * resolution) / vec2(resolution));
    const float dot4 = dot(GradientAt(v4), vec2(tex_coords - v4 * resolution) / vec2(resolution));

    const vec2 t = vec2(tex_coords - coords * resolution) / vec2(resolution);
    const vec2 smooth_t = vec2(smoothstep(0.0, 1.0, t.x), smoothstep(0.0, 1.0, t.y));

    const float color1 = mix(dot1,   dot2,   smooth_t.x);
    const float color2 = mix(dot4,   dot3,   smooth_t.x);
    const float color  = mix(color1, color2, smooth_t.y) * 0.5 + 0.5;
    //const float color = float(coords.x + coords.y * gradients_dimensions.x) / float(gradients_dimensions.x * gradients_dimensions.y);

    imageStore(noise, tex_coords, vec4(color, color, color, 1.0));
}

