#version 460 core

layout(location = 0) in vec3 inUVW;
layout(location = 1) in float inAO;

layout(location = 0) out vec4 outColor;

uniform sampler2DArray u_texture;

void main()
{
  const vec3 black = vec3(0.0, 0.0, 0.0);
  
  vec4 color = texture(u_texture, inUVW);
  vec3 shaded = mix(color.rgb, black, inAO);
  outColor = vec4(shaded, color.a);
}

