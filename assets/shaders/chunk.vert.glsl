#version 450 core

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoords;

uniform mat4 u_model, u_vp;

out vec2 texCoords;

void main()
{
    texCoords = inTexCoords;
    gl_Position = u_vp * u_model * vec4(inPosition, 1.0);
}

