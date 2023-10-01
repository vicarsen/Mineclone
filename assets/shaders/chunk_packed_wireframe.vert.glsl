#version 450 core

layout(location = 0) in int encoded_batch1;

uniform mat4 u_model;
uniform mat4 u_vp;

out vec3 barycentric;

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

void main()
{
    const float x = extractX(encoded_batch1);
    const float y = extractY(encoded_batch1);
    const float z = extractZ(encoded_batch1);

    barycentric = vec3(((gl_VertexID & 3) == 0 ? 1.0 : 0.0), ((gl_VertexID & 3) == 1 || (gl_VertexID & 3) == 3 ? 1.0 : 0.0), ((gl_VertexID & 3) == 2 ? 1.0 : 0.0));
    gl_Position = u_vp * u_model * vec4(x, y, z, 1.0);
}

