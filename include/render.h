#pragma once

#include <string>
#include <memory>

#include <glm/glm.hpp>

#include "config.h"
#include "logger.h"
#include "blocks.h"
#include "player.h"

DECLARE_LOG_CATEGORY(OpenGLInternal);
DECLARE_LOG_CATEGORY(Shaders);
DECLARE_LOG_CATEGORY(DynamicFaceBuffer);
DECLARE_LOG_CATEGORY(ChunkMesh);
DECLARE_LOG_CATEGORY(Renderer);
DECLARE_LOG_CATEGORY(ChunkRenderer);

namespace Game
{
    class World;
};

namespace Render
{
    struct Vertex
    {
        int encoded;
    };

    struct FaceMesh
    {
        Vertex v[4];
    };

    enum class FaceOrientation { XOY, YOZ, ZOX };
    
    class DynamicFaceBuffer
    {
    public:
        inline static const unsigned int STEP = 32 * 32;
        inline static const unsigned int HALF_STEP = STEP / 2;

        DynamicFaceBuffer(unsigned int _reserved = STEP);
        DynamicFaceBuffer(DynamicFaceBuffer&& other);
        DynamicFaceBuffer(const DynamicFaceBuffer& other);
        ~DynamicFaceBuffer();

        DynamicFaceBuffer& operator=(DynamicFaceBuffer&& other);
        DynamicFaceBuffer& operator=(const DynamicFaceBuffer& other);

        void Insert(unsigned int faceID, const FaceMesh& face);
        void Remove(unsigned int faceID);

        void InsertMany(unsigned int count, const unsigned int* faceIDs, const FaceMesh* faces);
        void RemoveMany(unsigned int count, const unsigned int* faceIDs);

        void Clear();

        inline bool Has(unsigned int faceID) const { return sparse[faceID] != 0xffff; }

        void Bind() const;

        inline unsigned int Size() const noexcept { return size; }

    private:
        void ValidateAndReallocate(unsigned int additional = 1);

    private:
        unsigned int reserved, size;
        unsigned int vao, buffer;
        unsigned short sparse[(CHUNK_SIZE + 1) * (CHUNK_SIZE + 1) * (CHUNK_SIZE + 1)];
    };

    class ChunkMesh
    {
    public:
        ChunkMesh();
        ChunkMesh(ChunkMesh&& other);
        ~ChunkMesh();

        ChunkMesh& operator=(ChunkMesh&& other);

        void PlaceFace(const glm::ivec3& position, FaceOrientation orientation, Game::Face face);
        void RemoveFace(const glm::ivec3& position, FaceOrientation orientation);

        void PlaceManyFaces(unsigned int count, const glm::ivec3* positions, FaceOrientation orientation, const Game::Face* faces);
        void PlaceManyFaces(unsigned int count, const glm::ivec3* positions, FaceOrientation orientation, Game::Face face);
        void RemoveManyFaces(unsigned int count, const glm::ivec3* positions, FaceOrientation orientation);

        void ClearFaces(FaceOrientation orientation);

        bool HasFace(FaceOrientation orientation, const glm::ivec3& position);

        void Bind(unsigned int buffer) const;

        inline unsigned int Size() const noexcept { return 3; }
        inline unsigned int Size(unsigned int i) const noexcept { return buffers[i].Size(); }

    private:
        DynamicFaceBuffer buffers[3];
    };

    class VFShader
    {
    public:
        VFShader() = default;
        ~VFShader() = default;

        void Init(const std::string& vertexSrc, const std::string& fragmentSrc);
        void Destroy();

        void Bind() const;

        int GetUniformLocation(const std::string& name);

    private:
        unsigned int program;
    };

    enum class TextureStyle { PIXELATED, SMOOTH };

    class Texture2D
    {
    public:
        Texture2D() = default;
        ~Texture2D() = default;

        void Init(unsigned int levels, unsigned int channels, unsigned int width, unsigned int height);
        void Destroy();

        void SetStyle(TextureStyle style);

        void SetData(const unsigned char* data, unsigned int level, unsigned int channels);
        void SetData(const unsigned char* data, unsigned int level, unsigned int channels, unsigned int x, unsigned int y, unsigned int width, unsigned int height);
        void CopyData(const Texture2D& other, unsigned int srcLevel, unsigned int srcX, unsigned int srcY, unsigned int dstLevel, unsigned int dstX, unsigned int dstY, unsigned int width, unsigned int height);
        void GenerateMipmaps();

        void Bind(unsigned int slot) const;

        inline unsigned int GetLevels() const noexcept { return levels; }
        inline unsigned int GetWidth() const noexcept { return width; }
        inline unsigned int GetHeight() const noexcept { return height; }
        inline unsigned int GetChannelCount() const noexcept { return channels; }
        inline TextureStyle GetStyle() const noexcept { return style; }

    private:
        unsigned int texture;
        unsigned int width, height, channels, levels, format;
        TextureStyle style;
    };

    class TextureAtlas
    {
    public:
        static inline const unsigned int STEP = 16;
        static inline const unsigned int HALF_STEP = 8;

        TextureAtlas() = default;
        ~TextureAtlas() = default;

        void Init(unsigned int levels, unsigned int channels, unsigned int subwidth, unsigned int subheight, unsigned int reserved = STEP);
        void Destroy();

        int Add(const Texture2D& texture);
        int Add(const std::vector<Texture2D>& textures);

        void Bind(unsigned int slot) const;

        inline glm::ivec2 GetTextureSize() const noexcept { return { subwidth, subheight }; }
        inline glm::ivec2 GetDimensions() const noexcept { return { length, length }; }

    private:
        void ValidateAndReallocate(unsigned int additional = 1);
        int InternalAdd(const Texture2D& texture);

    private:
        Texture2D texture;
        unsigned int subwidth, subheight;
        unsigned int length, size;
    };

    class Renderer
    {
    public:
        Renderer() = delete;

        static void Init();
        static void Destroy();

        static void Render(Game::World& world, const Game::Player& player);
    };

    class ChunkRenderer
    {
    public:
        ChunkRenderer() = delete;

        static void Init();
        static void Destroy();

        static void Begin();
        static void Render(const Game::World& world);
        static void End();

        static int GetTextureID(const Game::Face& face);

        static inline void SetViewMatrix(const glm::mat4& matrix) { view_matrix = matrix; }
        static inline void SetProjectionMatrix(const glm::mat4& matrix) { projection_matrix = matrix; }

    private:
        static int LoadTexture(const std::string& path);

    private:
        struct BlockData
        {
            int north = -1, south = -1, east = -1, west = -1, up = -1, down = -1;
        };

        static TextureAtlas atlas;
        static VFShader shader[3];
        static int model_location[3], vp_location[3], atlas_dimensions_location[3], texture_size_location[3], texture_location[3];
        static glm::mat4 view_matrix, projection_matrix;
        static std::unordered_map<std::string, int> texture_IDs;
        static std::vector<BlockData> blocks;
    };

    class RenderThread
    {
    public:
        static void Start();
        static void Stop();

        static void Submit(const std::shared_ptr<ChunkMesh>& mesh);

    private:
        RenderThread() = default;
        ~RenderThread() = default;

        static void Run();

    private:
        static RenderThread instance;
        static std::thread thread;
        static bool done;
    };
};

namespace fmt
{
    template<>
    struct formatter<Render::Vertex>
    {
        constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
        {
            return ctx.end();
        }

        template<typename FormatContext>
        auto format(const Render::Vertex& vertex, FormatContext& ctx) -> decltype(ctx.out())
        {
            return fmt::format_to(ctx.out(), "{{encoded:{}}}", vertex.encoded);
        }
    };

    template<>
    struct formatter<Render::FaceMesh>
    {
        constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
        {
            return ctx.end();
        }

        template<typename FormatContext>
        auto format(const Render::FaceMesh& face, FormatContext& ctx) -> decltype(ctx.out())
        {
            return fmt::format_to(ctx.out(), "{{v[0]:{}, v[1]:{}, v[2]:{}, v[3]:{}}}", face.v[0], face.v[1], face.v[2], face.v[3]);
        }
    };

    template<>
    struct formatter<Render::FaceOrientation>
    {
        constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
        {
            return ctx.end();
        }

        template<typename FormatContext>
        auto format(const Render::FaceOrientation& orientation, FormatContext& ctx) -> decltype(ctx.out())
        {
            switch(orientation)
            {
            case Render::FaceOrientation::XOY: return fmt::format_to(ctx.out(), "XOY");
            case Render::FaceOrientation::YOZ: return fmt::format_to(ctx.out(), "YOZ");
            case Render::FaceOrientation::ZOX: return fmt::format_to(ctx.out(), "ZOX");
            default: return fmt::format_to(ctx.out(), "");
            }
        }
    };
};

