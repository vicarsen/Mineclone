#version 450 core

layout(location = 0) in int encoded_batch1;

uniform mat4 u_model;
uniform mat4 u_vp;

uniform vec2 u_texture_size;
uniform ivec2 u_atlas_dimensions;

out vec2 frag_texture;
out vec2 frag_tex_coords;

float extractX(int encoded)
{
    return float((encoded >> 25));
}

float extractY(int encoded)
{
    return float((encoded >> 18) & ((1 << 7) - 1));
}

float extractZ(int encoded)
{
    return float((encoded >> 11) & ((1 << 7) - 1));
}

int extractTexture(int encoded)
{
    return encoded & ((1 << 10) - 1);
}

void main()
{
    const float x = extractX(encoded_batch1);
    const float y = extractY(encoded_batch1);
    const float z = extractZ(encoded_batch1);

    const int texture = extractTexture(encoded_batch1);
    
    frag_texture = vec2(texture / u_atlas_dimensions.y, texture % u_atlas_dimensions.y);

    const float u = (gl_VertexID & 3) == 0 || (gl_VertexID & 3) == 1 ? 0.1 : -0.1;
    const float v = (gl_VertexID & 3) == 0 || (gl_VertexID & 3) == 3 ? 0.1 : -0.1;

    frag_tex_coords = vec2(x, y) + ((encoded_batch1 & (1 << 10)) != 0 ? vec2(u, v) : vec2(v, u)) / vec2(u_texture_size);
    gl_Position = u_vp * u_model * vec4(x, y, z, 1.0);
}

