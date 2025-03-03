#include "mineclonelib/world/chunk.h"
#include "mineclonelib/world/blocks.h"

namespace mc
{
namespace world
{
// Precalculated ambient occlusion data.
// Mask = abcdefghi
// a b c
// d e f
// g h i
// Note: e is the face to find the ambient occlusion of, a is the block "above", with the other 2 coordinates decremented,
// c is the block "above", with the first coordinate decremented and the second incremented, and so on.
static const uint8_t c_ao[] = {
	0,   64,  80,  144, 16,	 80,  96,  160, 65,  129, 209, 209, 81,	 145,
	225, 225, 0,   64,  80,	 144, 16,  80,	96,  160, 65,  129, 209, 209,
	81,  145, 225, 225, 20,	 84,  116, 180, 36,  100, 116, 180, 85,	 149,
	245, 245, 101, 165, 245, 245, 20,  84,	116, 180, 36,  100, 116, 180,
	85,  149, 245, 245, 101, 165, 245, 245, 1,   65,  81,  145, 17,	 81,
	97,  161, 66,  130, 210, 210, 82,  146, 226, 226, 1,   65,  81,	 145,
	17,  81,  97,  161, 66,	 130, 210, 210, 82,  146, 226, 226, 21,	 85,
	117, 181, 37,  101, 117, 181, 86,  150, 246, 246, 102, 166, 246, 246,
	21,  85,  117, 181, 37,	 101, 117, 181, 86,  150, 246, 246, 102, 166,
	246, 246, 5,   69,  85,	 149, 21,  85,	101, 165, 71,  135, 215, 215,
	87,  151, 231, 231, 5,	 69,  85,  149, 21,  85,  101, 165, 71,	 135,
	215, 215, 87,  151, 231, 231, 29,  93,	125, 189, 45,  109, 125, 189,
	95,  159, 255, 255, 111, 175, 255, 255, 29,  93,  125, 189, 45,	 109,
	125, 189, 95,  159, 255, 255, 111, 175, 255, 255, 6,   70,  86,	 150,
	22,  86,  102, 166, 71,	 135, 215, 215, 87,  151, 231, 231, 6,	 70,
	86,  150, 22,  86,  102, 166, 71,  135, 215, 215, 87,  151, 231, 231,
	30,  94,  126, 190, 46,	 110, 126, 190, 95,  159, 255, 255, 111, 175,
	255, 255, 30,  94,  126, 190, 46,  110, 126, 190, 95,  159, 255, 255,
	111, 175, 255, 255, 4,	 68,  84,  148, 20,  84,  100, 164, 69,	 133,
	213, 213, 85,  149, 229, 229, 4,   68,	84,  148, 20,  84,  100, 164,
	69,  133, 213, 213, 85,	 149, 229, 229, 24,  88,  120, 184, 40,	 104,
	120, 184, 89,  153, 249, 249, 105, 169, 249, 249, 24,  88,  120, 184,
	40,  104, 120, 184, 89,	 153, 249, 249, 105, 169, 249, 249, 5,	 69,
	85,  149, 21,  85,  101, 165, 70,  134, 214, 214, 86,  150, 230, 230,
	5,   69,  85,  149, 21,	 85,  101, 165, 70,  134, 214, 214, 86,	 150,
	230, 230, 25,  89,  121, 185, 41,  105, 121, 185, 90,  154, 250, 250,
	106, 170, 250, 250, 25,	 89,  121, 185, 41,  105, 121, 185, 90,	 154,
	250, 250, 106, 170, 250, 250, 9,   73,	89,  153, 25,  89,  105, 169,
	75,  139, 219, 219, 91,	 155, 235, 235, 9,   73,  89,  153, 25,	 89,
	105, 169, 75,  139, 219, 219, 91,  155, 235, 235, 29,  93,  125, 189,
	45,  109, 125, 189, 95,	 159, 255, 255, 111, 175, 255, 255, 29,	 93,
	125, 189, 45,  109, 125, 189, 95,  159, 255, 255, 111, 175, 255, 255,
	10,  74,  90,  154, 26,	 90,  106, 170, 75,  139, 219, 219, 91,	 155,
	235, 235, 10,  74,  90,	 154, 26,  90,	106, 170, 75,  139, 219, 219,
	91,  155, 235, 235, 30,	 94,  126, 190, 46,  110, 126, 190, 95,	 159,
	255, 255, 111, 175, 255, 255, 30,  94,	126, 190, 46,  110, 126, 190,
	95,  159, 255, 255, 111, 175, 255, 255
};

chunk_draw_data simple_chunk_draw_data_generator::generate(chunk *ch)
{
	static const int dx[] = { 1, -1, 0, 0, 0, 0 };
	static const int dy[] = { 0, 0, 1, -1, 0, 0 };
	static const int dz[] = { 0, 0, 0, 0, 1, -1 };

	static const int dleftx[] = { 0, 0, 0, 0, 1, -1 };
	static const int dlefty[] = { 1, -1, 0, 0, 0, 0 };
	static const int dleftz[] = { 0, 0, 1, -1, 0, 0 };

	static const int dtopx[] = { 0, 0, 1, -1, 0, 0 };
	static const int dtopy[] = { 0, 0, 0, 0, 1, -1 };
	static const int dtopz[] = { 1, -1, 0, 0, 0, 0 };

	chunk_draw_data data;

	face_registry *freg = faces::get_registry();
	block_registry *breg = blocks::get_registry();
	for (int k = 0; k < 6; k++) {
		block_face kf = static_cast<block_face>(k);
		for (int x = CHUNK_BEGIN; x < CHUNK_END; x++) {
			for (int y = CHUNK_BEGIN; y < CHUNK_END; y++) {
				for (int z = CHUNK_BEGIN; z < CHUNK_END; z++) {
					int nx = x + dx[k], ny = y + dy[k],
					    nz = z + dz[k];

					face_id faceid =
						breg->get(ch->get(x, y, z))
							->get_face(kf);

					face_id nfaceid =
						breg->get(ch->get(nx, ny, nz))
							->get_face(kf);

					face *f = freg->get(faceid);
					face *nf = freg->get(nfaceid);

					if (f->get_texture() == nullptr ||
					    nf->get_texture() != nullptr) {
						continue;
					}

					uint16_t mask = 0;

					int di = -1, dj = -1, bit = 1;
					while (di <= 1) {
						int nnx = nx + dleftx[k] * di +
							  dtopx[k] * dj,
						    nny = ny + dlefty[k] * di +
							  dtopy[k] * dj,
						    nnz = nz + dleftz[k] * di +
							  dtopz[k] * dj;

						block *block = breg->get(
							ch->get(nnx, nny, nnz));

						if (block->is_opaque()) {
							mask |= bit;
						}

						if (++dj > 1) {
							di++;
							dj = -1;
						}

						bit <<= 1;
					}

					data.faces.emplace_back(faceid, kf,
								c_ao[mask], x,
								y, z);
				}
			}
		}
	}

	return data;
}
}
}
