#version 450 core

in vec2 frag_texture;
in vec2 frag_tex_coords;

uniform vec2 u_texture_size;
uniform ivec2 u_atlas_dimensions;
uniform sampler2D u_texture;

layout(location = 0) out vec4 out_color;

vec2 half_pixel_correction(vec2 coords)
{
    return coords + vec2(0.5, 0.5) / (u_atlas_dimensions * u_texture_size);
}

void main()
{
    const vec2 coords = (frag_texture + fract(frag_tex_coords)) / u_atlas_dimensions;
    out_color = texture(u_texture, coords);
    //out_color = vec4(coords, 0.0, 1.0);
}

