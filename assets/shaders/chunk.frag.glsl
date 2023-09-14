#version 450 core

layout(location = 0) out vec4 outColor;

in vec2 texCoords;

void main()
{
    outColor = vec4(texCoords, 0.0, 1.0);
}

