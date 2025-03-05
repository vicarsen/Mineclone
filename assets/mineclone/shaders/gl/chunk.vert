#version 460 core

layout(location = 0) out vec3 outUVW;
layout(location = 1) out float outAO;

struct Face {
  uint geometry;
  uint shading;
};

struct Chunk {
  mat4 model;
  uint normal;
};

layout(binding = 1, std430) readonly buffer faces_ssbo {
  Face faces[];
};

layout(binding = 2, std430) readonly buffer chunks_ssbo {
  Chunk chunks[];
};

uniform mat4 u_view;
uniform mat4 u_projection;

uint extract_x(uint geometry)
{
  return geometry & ((1 << 6) - 1);
}

uint extract_y(uint geometry)
{
  return (geometry >> 6) & ((1 << 6) - 1);
}

uint extract_z(uint geometry)
{
  return (geometry >> 12) & ((1 << 6) - 1);
}

uint extract_ao(uint geometry)
{
  return (geometry >> 18) & ((1 << 8) - 1);
}

uint extract_tex(uint shading)
{
  return shading;
}

const int dx[] = { 1, 0, 0, 0, 0, 0 };
const int dy[] = { 0, 0, 1, 0, 0, 0 };
const int dz[] = { 0, 0, 0, 0, 1, 0 };

const int dleftx[] = { 0, 0, 1, 0, 0, 1 };
const int dlefty[] = { 0, 1, 0, 0, 1, 0 };
const int dleftz[] = { 1, 0, 0, 1, 0, 0 };
const int dleftu[] = { 0, 1, 1, 0, 1, 0 };
const int dleftv[] = { 1, 0, 0, 1, 0, 1 };

const int dtopx[] = { 0, 0, 0, 1, 1, 0 };
const int dtopy[] = { 1, 0, 0, 0, 0, 1 };
const int dtopz[] = { 0, 1, 1, 0, 0, 0 };
const int dtopu[] = { 1, 0, 0, 1, 0, 1 };
const int dtopv[] = { 0, 1, 1, 0, 1, 0 };

void main()
{
  Face face = faces[gl_VertexID / 6];
  uint x = extract_x(face.geometry);
  uint y = extract_y(face.geometry);
  uint z = extract_z(face.geometry);
  uint ao = extract_ao(face.geometry);

  uint u = 0;
  uint v = 0;
  uint tex = extract_tex(face.shading);

  mat4 model = chunks[gl_DrawID].model;
  uint normal = chunks[gl_DrawID].normal;

  uint idx = gl_VertexID % 6;

  x += dx[normal];
  y += dy[normal];
  z += dz[normal];

  if (1 <= idx && idx <= 3) {
    x += dleftx[normal];
    y += dlefty[normal];
    z += dleftz[normal];
    u += dleftu[normal];
    v += dleftv[normal];
  }

  if (2 <= idx && idx <= 4) {
    x += dtopx[normal];
    y += dtopy[normal];
    z += dtopz[normal];
    u += dtopu[normal];
    v += dtopv[normal];
  }

  if (idx == 1) {
    ao >>= 2;
  } else if (idx == 2 || idx == 3) {
    ao >>= 4;
  } else if (idx == 4) {
    ao >>= 6;
  }

  ao &= 3;

  outUVW = vec3(float(u), float(v), float(tex));
  outAO = float(ao) / 4.0;

  gl_Position = u_projection * u_view * model * vec4(float(x), float(y), float(z), 1.0);
}

