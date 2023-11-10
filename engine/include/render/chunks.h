#pragma once

#include "../logger.h"
#include "../mathematics.h"

#include "../utils/array.h"
#include "../utils/hash.h"
#include "../utils/memory.h"
#include "../utils/optional.h"

#include "buffer.h"
#include "shader.h"
#include "texture.h"
#include "texture_atlas.h"

DECLARE_LOG_CATEGORY(ChunkMesh);
DECLARE_LOG_CATEGORY(ChunkRenderer);

namespace Game
{
    class Face;
};

namespace Render
{
    namespace Chunks
    {
        struct Vertex
        {
            unsigned int encoded;
        };

        struct FaceMesh
        {
            Vertex v[4];
        };

        enum class FaceOrientation { XOY, YOZ, ZOX };

        typedef int MeshHandle;
        static const MeshHandle NULL_MESH = -1;
        
        class FaceBuffer
        {
        public:
            FaceBuffer();
            FaceBuffer(FaceBuffer&& other);
            FaceBuffer(const FaceBuffer& other);
            ~FaceBuffer();

            FaceBuffer& operator=(FaceBuffer&& other);
            FaceBuffer& operator=(const FaceBuffer& other);

            void SetData(unsigned int count, const FaceMesh* meshes);
            void Clear();

            inline void Bind() const { vao.Bind(); }
            inline void Unbind() const { vao.Unbind(); }

            inline unsigned int Size() const noexcept { return size; }

        private:
            void BuildVAO();

        private:
            VertexArray vao;
            Buffer buffer;
            unsigned int size;
        };

        class Mesh
        {
        public:
            Mesh();
            Mesh(Mesh&& other);
            Mesh(const Mesh& other);

            ~Mesh();

            Mesh& operator=(Mesh&& other);
            Mesh& operator=(const Mesh& other);

            void SetFaces(unsigned int count, const FaceMesh* meshes, FaceOrientation orientation);
            void Clear(FaceOrientation orientation);

            inline void Bind(unsigned int buffer) const { buffers[buffer].Bind(); }
            inline void Unbind(unsigned int buffer) const { buffers[buffer].Unbind(); }

            inline unsigned int Size() const noexcept { return 3; }
            inline unsigned int BufferSize(unsigned int i) const noexcept { return buffers[i].Size(); }

            inline void SetModelMatrix(const ::Math::mat4& model_matrix) noexcept { this->model_matrix = model_matrix; }
            inline const ::Math::mat4& GetModelMatrix() const noexcept { return model_matrix; }

            inline void SetHandle(MeshHandle handle) noexcept { this->handle = handle; }
            inline MeshHandle GetHandle() const noexcept { return handle; }

        private:
            FaceBuffer buffers[3];
            ::Math::mat4 model_matrix;
            MeshHandle handle;
        };

        class WeakMeshReference
        {
        public:
            inline WeakMeshReference() = default;
            inline WeakMeshReference(WeakMeshReference&& other) = default;
            inline WeakMeshReference(const WeakMeshReference& other) = default;

            inline WeakMeshReference(const ::Utils::SharedPointer<Mesh>& mesh) : handle(mesh->GetHandle()), mesh(mesh) {}

            inline ~WeakMeshReference() = default;

            inline WeakMeshReference& operator=(WeakMeshReference&& other) = default;
            inline WeakMeshReference& operator=(const WeakMeshReference& other) = default;

            inline WeakMeshReference& operator=(const ::Utils::SharedPointer<Mesh>& mesh) { handle = mesh->GetHandle(); this->mesh = mesh; return *this; }

            inline void reset() noexcept { handle = NULL_MESH; mesh.reset(); }
            inline void swap(WeakMeshReference& other) noexcept { ::std::swap(handle, other.handle); mesh.swap(other.mesh); }

            inline long use_count() const noexcept { return mesh.use_count(); }
            inline bool expired() const noexcept { return mesh.expired(); }
            inline ::Utils::SharedPointer<Mesh> lock() const noexcept { return mesh.lock(); }

            inline MeshHandle GetHandle() const noexcept { return handle; }
            inline operator MeshHandle() const noexcept { return handle; }

        private:
            MeshHandle handle = NULL_MESH;
            ::Utils::WeakPointer<Mesh> mesh;
        };

        class MeshPool
        {
        public:
            MeshPool();
            MeshPool(MeshPool&& other);
            MeshPool(const MeshPool& other);

            ~MeshPool();

            MeshPool& operator=(MeshPool&& other);
            MeshPool& operator=(const MeshPool& other);

            MeshHandle New();
            void Free(MeshHandle handle);

            inline bool Has(MeshHandle handle) const { return meshes.find(handle) != meshes.end(); }

            inline ::Utils::SharedPointer<Mesh>& At(MeshHandle handle) { return meshes.at(handle); }
            inline const ::Utils::SharedPointer<Mesh>& At(MeshHandle handle) const { return meshes.at(handle); }

        private:
            ::Utils::HashMap<MeshHandle, ::Utils::SharedPointer<Mesh>> meshes;
        };

        namespace __detail
        {
            namespace __iterator
            {
                class RenderQueueIterator;
                class RenderQueueConstIterator;
            };

            class RenderQueue
            {
            public:
                typedef __iterator::RenderQueueIterator Iterator;
                typedef __iterator::RenderQueueConstIterator ConstIterator;

                RenderQueue() = default;
                RenderQueue(RenderQueue&& other) = default;
                RenderQueue(const RenderQueue& other) = default;

                ~RenderQueue() = default;

                RenderQueue& operator=(RenderQueue&& other) = default;
                RenderQueue& operator=(const RenderQueue& other) = default;

                inline void Add(const ::Utils::WeakPointer<Mesh>& mesh) { render_queue.Push(mesh); }

                inline void Clear() { render_queue.Clear(); }

                Iterator Begin() noexcept;
                Iterator End() noexcept;

                Iterator CBegin() const noexcept;
                Iterator CEnd() const noexcept;

            private:
                ::Utils::Array<::Utils::WeakPointer<Mesh>> render_queue;

                friend class __iterator::RenderQueueIterator;
                friend class __iterator::RenderQueueConstIterator;
            };

            class RenderContext
            {
            public:
                RenderContext();
                RenderContext(RenderContext&& other) = delete;
                RenderContext(const RenderContext& other) = delete;

                ~RenderContext();

                RenderContext& operator=(RenderContext&& other) = delete;
                RenderContext& operator=(const RenderContext& other) = delete;

                MeshHandle NewMesh();
                void FreeMesh(MeshHandle handle);

                inline const ::Utils::SharedPointer<Mesh>& GetMesh(MeshHandle handle) const { return mesh_pool.At(handle); }

                inline RenderQueue& GetRenderQueue() noexcept { return render_queue; }
                inline const RenderQueue& GetRenderQueue() const noexcept { return render_queue; }

                inline const ::Math::mat4& GetVPMatrix() const noexcept { return vp_matrix; }
                inline void SetVPMatrix(const ::Math::mat4& vp_matrix) noexcept { this->vp_matrix = vp_matrix; }

            private:
                MeshPool mesh_pool;
                RenderQueue render_queue;

                ::Math::mat4 vp_matrix;
            };
        };

        class Renderer
        {
        public:
            typedef __detail::RenderContext RenderContext;

            enum class DrawMode { NORMAL, WIREFRAME };

            Renderer();
            Renderer(Renderer&& other) = delete;
            Renderer(const Renderer& other) = delete;

            ~Renderer();

            Renderer& operator=(Renderer&& other) = delete;
            Renderer& operator=(const Renderer& other) = delete;

            void Begin();
            void Begin(FaceOrientation orientation);
            void Render(const Mesh& mesh);
            void End();

            int GetTextureID(const ::Game::Face& face);

            inline TextureAtlas& GetAtlas() { return atlas.value(); }

            inline RenderContext& GetRenderContext() noexcept { return render_context; }
            inline const RenderContext& GetRenderContext() const noexcept { return render_context; }

            inline DrawMode GetDrawMode() const noexcept { return draw_mode; }
            inline void SetDrawMode(DrawMode mode) noexcept { draw_mode = mode; }

        private:
            struct BlockData
            {
                int north = -1, south = -1, east = -1, west = -1, up = -1, down = -1;
            };

            struct ChunkShader
            {
                VFShader shader;
                int model_location;
                int vp_location;
                int atlas_dimensions_location;
                int texture_size_location;
                int texture_location;
            };

        private:
            ::Utils::Optional<Texture2D> LoadTexture(const char* path);
            ::Utils::Optional<ChunkShader> LoadShader(const char* vertex_path, const char* fragment_path);

        private:
            ::Utils::Optional<TextureAtlas> atlas;
            ::Utils::Optional<ChunkShader> chunk_shaders[3];
            ::Utils::Optional<ChunkShader> wireframe_shader;

            ::Utils::Array<BlockData> blocks;

            RenderContext render_context;

            unsigned int active_pipeline;
            DrawMode draw_mode;
        };
    };
};

