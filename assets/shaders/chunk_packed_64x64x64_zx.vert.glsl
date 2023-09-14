#version 450 core

layout(location = 0) in int encoded_batch1;

uniform mat4 u_model;
uniform mat4 u_vp;

uniform vec2 u_textureSize;
uniform ivec2 u_atlasDimensions;

out vec2 frag_texture;
out vec2 frag_texCoords;

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
    
    frag_texture = vec2(texture / u_atlasDimensions.y, texture % u_atlasDimensions.y);

    const float u = (gl_VertexID & 3) == 0 || (gl_VertexID & 3) == 1 ? 0.1 : -0.1;
    const float v = (gl_VertexID & 3) == 0 || (gl_VertexID & 3) == 3 ? 0.1 : -0.1;

    frag_texCoords = vec2(z, x) + ((encoded_batch1 & (1 << 10)) != 0 ? vec2(u, v) : vec2(v, u)) / vec2(u_textureSize);
    gl_Position = u_vp * u_model * vec4(x, y, z, 1.0);
}

