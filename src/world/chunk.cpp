#include "world/chunk.h"

namespace mc
{
  inline static packed_chunk_vertex_t __pack_vertex(u16 x, u16 y, u16 z)
  {
    return (x * CHUNK_FULL + y) * CHUNK_FULL + z;
  }

  inline static bool __chunk_get_face_x(bool chunk[CHUNK_FULL][CHUNK_FULL][CHUNK_FULL], u32 x, u32 y, u32 z)
  {
    return ((u32)(chunk[x + 1][y][z] && !chunk[x][y][z]) << 1) | (!chunk[x + 1][y][z] && chunk[x][y][z]);
  }

  inline static bool __chunk_can_extend_x(bool chunk[CHUNK_FULL][CHUNK_FULL][CHUNK_FULL],
                                          bool checked[CHUNK_FULL][CHUNK_FULL],
                                          u32 face, u32 x, u32 y, u32 z0, u32 z1)
  {
    for (u32 z = z0; z < z1; z++) {
      if (checked[y][z] || face != __chunk_get_face_x(chunk, x, y, z)) {
        return false;
      }
    }

    return true;
  }

  inline static void __chunk_extend_x(bool chunk[CHUNK_FULL][CHUNK_FULL][CHUNK_FULL],
                                      bool checked[CHUNK_FULL][CHUNK_FULL],
                                      u32 y, u32 z0, u32 z1)
  {
    for (u32 z = z0; z < z1; z++) {
      checked[y][z] = true;
    }
  }

  static void __chunk_generate_mesh_x(bool chunk[CHUNK_FULL][CHUNK_FULL][CHUNK_FULL],
                                    std::vector<packed_chunk_vertex_t>& pos,
                                    std::vector<packed_chunk_vertex_t>& neg,
                                    bool checked[CHUNK_FULL][CHUNK_FULL])
  {
    pos.clear();
    neg.clear();

    for (u32 x = 0; x < CHUNK_SIZE; x++) {
      for (u32 y = 0; y < CHUNK_SIZE; y++) {
        for (u32 z = 0; z < CHUNK_SIZE; z++) {
          checked[y][z] = false;
        }
      }

      for (u32 y = 0; y < CHUNK_SIZE; y++) {
        for (u32 z = 0; z < CHUNK_SIZE; z++) {
          if (!checked[y][z]) {
            u32 face = __chunk_get_face_x(chunk, x, y, z);
            if (face == 0) {
              continue;
            }

            u32 z1 = z + 1;
            while (z1 <= CHUNK_SIZE && !checked[y][z1] && face == __chunk_get_face_x(chunk, x, y, z1)) {
              checked[y][z1++] = true;
            }

            u32 y1 = y + 1;
            while (y1 <= CHUNK_SIZE && __chunk_can_extend_x(chunk, checked, face, x, y1, z, z1)) {
              __chunk_extend_x(chunk, checked, y1++, z, z1);
            }

            if (face == 1) {
              pos.push_back(__pack_vertex(x, y, z));
              pos.push_back(__pack_vertex(x, y, z1));
              pos.push_back(__pack_vertex(x, y1, z1));
              pos.push_back(__pack_vertex(x, y1, z));
            } else {
              neg.push_back(__pack_vertex(x, y, z));
              neg.push_back(__pack_vertex(x, y1, z));
              neg.push_back(__pack_vertex(x, y1, z1));
              neg.push_back(__pack_vertex(x, y, z1));
            }
          }
        }
      }
    }
  }

  inline static bool __chunk_get_face_y(bool chunk[CHUNK_FULL][CHUNK_FULL][CHUNK_FULL], u32 x, u32 y, u32 z)
  {
    return ((u32)(chunk[x][y + 1][z] && !chunk[x][y][z]) << 1) | (!chunk[x][y + 1][z] && chunk[x][y][z]);
  }

  inline static bool __chunk_can_extend_y(bool chunk[CHUNK_FULL][CHUNK_FULL][CHUNK_FULL],
                                          bool checked[CHUNK_FULL][CHUNK_FULL],
                                          u32 face, u32 x, u32 y, u32 z0, u32 z1)
  {
    for (u32 z = z0; z < z1; z++) {
      if (checked[x][z] || face != __chunk_get_face_y(chunk, x, y, z)) {
        return false;
      }
    }

    return true;
  }

  inline static void __chunk_extend_y(bool chunk[CHUNK_FULL][CHUNK_FULL][CHUNK_FULL],
                                      bool checked[CHUNK_FULL][CHUNK_FULL],
                                      u32 x, u32 z0, u32 z1)
  {
    for (u32 z = z0; z < z1; z++) {
      checked[x][z] = true;
    }
  }

  static void __chunk_generate_mesh_y(bool chunk[CHUNK_FULL][CHUNK_FULL][CHUNK_FULL],
                                    std::vector<packed_chunk_vertex_t>& pos,
                                    std::vector<packed_chunk_vertex_t>& neg,
                                    bool checked[CHUNK_FULL][CHUNK_FULL])
  {
    pos.clear();
    neg.clear();

    for (u32 y = 0; y < CHUNK_SIZE; y++) {
      for (u32 x = 0; x < CHUNK_SIZE; x++) {
        for (u32 z = 0; z < CHUNK_SIZE; z++) {
          checked[x][z] = false;
        }
      }

      for (u32 x = 0; x < CHUNK_SIZE; x++) {
        for (u32 z = 0; z < CHUNK_SIZE; z++) {
          if (!checked[x][z]) {
            u32 face = __chunk_get_face_y(chunk, x, y, z);
            if (face == 0) {
              continue;
            }

            u32 z1 = z + 1;
            while (z1 <= CHUNK_SIZE && !checked[x][z1] && face == __chunk_get_face_y(chunk, x, y, z1)) {
              checked[x][z1++] = true;
            }

            u32 x1 = x + 1;
            while (x1 <= CHUNK_SIZE && __chunk_can_extend_y(chunk, checked, face, x1, y, z, z1)) {
              __chunk_extend_y(chunk, checked, x1++, z, z1);
            }

            if (face == 1) {
              pos.push_back(__pack_vertex(x, y, z));
              pos.push_back(__pack_vertex(x1, y, z));
              pos.push_back(__pack_vertex(x1, y, z1));
              pos.push_back(__pack_vertex(x, y, z1));
            } else {
              neg.push_back(__pack_vertex(x, y, z));
              neg.push_back(__pack_vertex(x, y, z1));
              neg.push_back(__pack_vertex(x1, y, z1));
              neg.push_back(__pack_vertex(x1, y, z));
            }
          }
        }
      }
    }
  }

  inline static bool __chunk_get_face_z(bool chunk[CHUNK_FULL][CHUNK_FULL][CHUNK_FULL], u32 x, u32 y, u32 z)
  {
    return ((u32)(chunk[x][y][z + 1] && !chunk[x][y][z]) << 1) | (!chunk[x][y][z + 1] && chunk[x][y][z]);
  }

  inline static bool __chunk_can_extend_z(bool chunk[CHUNK_FULL][CHUNK_FULL][CHUNK_FULL],
                                          bool checked[CHUNK_FULL][CHUNK_FULL],
                                          u32 face, u32 x, u32 y0, u32 y1, u32 z)
  {
    for (u32 y = y0; y < y1; y++) {
      if (checked[x][y] || face != __chunk_get_face_z(chunk, x, y, z)) {
        return false;
      }
    }

    return true;
  }

  inline static void __chunk_extend_z(bool chunk[CHUNK_FULL][CHUNK_FULL][CHUNK_FULL],
                                      bool checked[CHUNK_FULL][CHUNK_FULL],
                                      u32 x, u32 y0, u32 y1)
  {
    for (u32 y = y0; y < y1; y++) {
      checked[x][y] = true;
    }
  }

  static void __chunk_generate_mesh_z(bool chunk[CHUNK_FULL][CHUNK_FULL][CHUNK_FULL],
                                    std::vector<packed_chunk_vertex_t>& pos,
                                    std::vector<packed_chunk_vertex_t>& neg,
                                    bool checked[CHUNK_FULL][CHUNK_FULL])
  {
    pos.clear();
    neg.clear();

    for (u32 z = 0; z < CHUNK_SIZE; z++) {
      for (u32 x = 0; x < CHUNK_SIZE; x++) {
        for (u32 y = 0; y < CHUNK_SIZE; y++) {
          checked[x][y] = false;
        }
      }

      for (u32 x = 0; x < CHUNK_SIZE; x++) {
        for (u32 y = 0; y < CHUNK_SIZE; y++) {
          if (!checked[x][y]) {
            u32 face = __chunk_get_face_z(chunk, x, y, z);
            if (face == 0) {
              continue;
            }

            u32 y1 = y + 1;
            while (y1 <= CHUNK_SIZE && !checked[x][y1] && face == __chunk_get_face_z(chunk, x, y1, z)) {
              checked[x][y1++] = true;
            }

            u32 x1 = x + 1;
            while (x1 <= CHUNK_SIZE && __chunk_can_extend_z(chunk, checked, face, x1, y, y1, z)) {
              __chunk_extend_z(chunk, checked, x1++, y, y1);
            }

            if (face == 1) {
              pos.push_back(__pack_vertex(x, y, z));
              pos.push_back(__pack_vertex(x1, y, z));
              pos.push_back(__pack_vertex(x1, y1, z));
              pos.push_back(__pack_vertex(x, y1, z));
            } else {
              neg.push_back(__pack_vertex(x, y, z));
              neg.push_back(__pack_vertex(x, y1, z));
              neg.push_back(__pack_vertex(x1, y1, z));
              neg.push_back(__pack_vertex(x1, y, z));
            }
          }
        }
      }
    }
  }

  void chunk_generate_mesh(chunk_t *chunk, chunk_mesh_t *mesh)
  {
    thread_local static bool checked[CHUNK_FULL][CHUNK_FULL];

    __chunk_generate_mesh_x(chunk->chunk, mesh->east, mesh->west, checked);
    __chunk_generate_mesh_y(chunk->chunk, mesh->up, mesh->down, checked);
    __chunk_generate_mesh_z(chunk->chunk, mesh->north, mesh->south, checked);
  }
};

