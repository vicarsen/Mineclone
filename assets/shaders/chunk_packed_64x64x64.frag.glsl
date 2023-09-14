#version 450 core

in vec2 frag_texture;
in vec2 frag_texCoords;

uniform vec2 u_textureSize;
uniform ivec2 u_atlasDimensions;
uniform sampler2D u_texture;

layout(location = 0) out vec4 out_color;

vec2 half_pixel_correction(vec2 coords)
{
    return coords + vec2(0.5, 0.5) / (u_atlasDimensions * u_textureSize);
}

void main()
{
    const vec2 coords = (frag_texture + fract(frag_texCoords)) / u_atlasDimensions;
    out_color = texture(u_texture, coords);
    //out_color = vec4(coords, 0.0, 1.0);
}

