#include "render.h"

#include <glad/gl.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <cassert>

#include "files.h"
#include "window.h"
#include "stb_image.h"

#include "world.h"

DEFINE_LOG_CATEGORY(OpenGLInternal, spdlog::level::trace, LOGFILE("Render/OpenGLInternal.txt"));
DEFINE_LOG_CATEGORY(Shaders, spdlog::level::trace, LOGFILE("Render/Shaders.txt"));
DEFINE_LOG_CATEGORY(DynamicFaceBuffer, spdlog::level::trace, LOGFILE("Render/DynamicFaceBuffer.txt"));
DEFINE_LOG_CATEGORY(ChunkMesh, spdlog::level::trace, LOGFILE("Render/ChunkMesh.txt"));
DEFINE_LOG_CATEGORY(Renderer, spdlog::level::trace, LOGFILE("Render/Renderer.txt"));
DEFINE_LOG_CATEGORY(ChunkRenderer, spdlog::level::trace, LOGFILE("Render/ChunkRenderer.txt"));

namespace Render
{
    static unsigned int g_ebo, g_service_buffer;
    
    DynamicFaceBuffer::DynamicFaceBuffer(unsigned int _reserved)
    {
        TRACE(DynamicFaceBuffer, "[{}:constructor] (#_reserved:{})", (unsigned long long) this, _reserved);

        reserved = _reserved; size = 0;
        glCreateBuffers(1, &buffer);

        TRACE(DynamicFaceBuffer, "[{}:constructor] (buffer:{}) glCreateBuffer", (unsigned long long) this, buffer);

        glNamedBufferData(buffer, reserved * sizeof(FaceMesh), NULL, GL_DYNAMIC_DRAW);

        glCreateVertexArrays(1, &vao);

        TRACE(DynamicFaceBuffer, "[{}:constructor] (vao:{}) glCreateVertexArrays", (unsigned long long) this, vao);

        glVertexArrayVertexBuffer(vao, 0, buffer, 0, sizeof(Vertex));
        glVertexArrayElementBuffer(vao, g_ebo);

        glEnableVertexArrayAttrib(vao, 0);
        glVertexArrayAttribIFormat(vao, 0, 1, GL_INT, offsetof(Vertex, encoded));
        glVertexArrayAttribBinding(vao, 0, 0);

        std::memset(sparse, 0xff, sizeof(sparse));

        TRACE(DynamicFaceBuffer, "[{}:constructor] return", (unsigned long long) this);
    }

    DynamicFaceBuffer::DynamicFaceBuffer(DynamicFaceBuffer&& other)
    {
        TRACE(DynamicFaceBuffer, "[{}:move_constructor] (#other:{})", (unsigned long long) this, (unsigned long long) &other);

        std::memcpy(this, &other, sizeof(DynamicFaceBuffer));
        other.vao = other.buffer = 0;
    
        TRACE(DynamicFaceBuffer, "[{}:move_constructor] (buffer:{}) (vao:{}) return", (unsigned long long) this, buffer, vao);
    }

    DynamicFaceBuffer::DynamicFaceBuffer(const DynamicFaceBuffer& other) :
        DynamicFaceBuffer(other.reserved)
    {
        glCopyNamedBufferSubData(other.buffer, buffer, 0, 0, other.size * sizeof(FaceMesh));

        size = other.size;
        std::memcpy(sparse, other.sparse, sizeof(other.sparse));
    }

    DynamicFaceBuffer::~DynamicFaceBuffer()
    {
        TRACE(DynamicFaceBuffer, "[{}:destructor] (buffer:{}) (vao:{})", (unsigned long long) this, buffer, vao);

        if(vao) glDeleteVertexArrays(1, &vao);
        if(buffer) glDeleteBuffers(1, &buffer);

        TRACE(DynamicFaceBuffer, "[{}:destructor] return", (unsigned long long) this);
    }

    DynamicFaceBuffer& DynamicFaceBuffer::operator=(DynamicFaceBuffer&& other)
    {
        TRACE(DynamicFaceBuffer, "[{}:move_assignment] (#other:{}) (buffer:{}) (vao:{})", (unsigned long long) this, (unsigned long long) &other, buffer, vao);

        if(vao) glDeleteVertexArrays(1, &vao);
        if(buffer) glDeleteBuffers(1, &buffer);

        std::memcpy(this, &other, sizeof(DynamicFaceBuffer));
        other.vao = other.buffer = 0;

        TRACE(DynamicFaceBuffer, "[{}:move_assignment] return", (unsigned long long) this);
        return *this;
    }

    DynamicFaceBuffer& DynamicFaceBuffer::operator=(const DynamicFaceBuffer& other)
    {
        if(reserved < other.size)
        {
            glNamedBufferData(buffer, other.reserved * sizeof(FaceMesh), NULL, GL_DYNAMIC_DRAW);
            reserved = other.reserved;
        }

        glCopyNamedBufferSubData(other.buffer, buffer, 0, 0, other.size * sizeof(FaceMesh));

        size = other.size;
        std::memcpy(sparse, other.sparse, sizeof(other.sparse));

        return *this;
    }

    void DynamicFaceBuffer::Insert(unsigned int faceID, const FaceMesh& face)
    {
        TRACE(DynamicFaceBuffer, "[{}:Insert] (#faceID:{}) (#face:{})", (unsigned long long) this, faceID, face);

        ValidateAndReallocate(1);
        glNamedBufferSubData(buffer, sizeof(FaceMesh) * size, sizeof(FaceMesh), &face);
        sparse[faceID] = size++;

        TRACE(DynamicFaceBuffer, "[{}:Insert] (size:{}) return", (unsigned long long) this, size);
    }

    void DynamicFaceBuffer::Remove(unsigned int faceID)
    {
        TRACE(DynamicFaceBuffer, "[{}:Remove] (#faceID:{})", (unsigned long long) this, faceID);

        if(sparse[faceID] != size - 1)
            glCopyNamedBufferSubData(buffer, buffer, sizeof(FaceMesh) * (size - 1), sizeof(FaceMesh) * sparse[faceID], sizeof(FaceMesh));
        
        sparse[faceID] = -1;
        size--;
        ValidateAndReallocate();

        TRACE(DynamicFaceBuffer, "[{}:Remove] (size:{}) return", (unsigned long long) this, size);
    }

    void DynamicFaceBuffer::InsertMany(unsigned int count, const unsigned int* faceIDs, const FaceMesh* faces)
    {
        TRACE(DynamicFaceBuffer, "[{}:InsertMany] (#count:{}) (#faceIDs:{}) (#faces:{})", (unsigned long long) this, count, (unsigned long long) faceIDs, (unsigned long long) faces);

        ValidateAndReallocate(count);
        glNamedBufferSubData(buffer, sizeof(FaceMesh) * size, sizeof(FaceMesh) * count, faces);
        for(unsigned int i = 0; i < count; i++)
            sparse[faceIDs[i]] = sizeof(FaceMesh) * (size + i);
        size += count;

        TRACE(DynamicFaceBuffer, "[{}:InsertMany] (size:{}) return", (unsigned long long) this, size);
    }

    void DynamicFaceBuffer::RemoveMany(unsigned int count, const unsigned int* faceIDs)
    {
        TRACE(DynamicFaceBuffer, "[{}:RemoveMany] (#count:{}) (faceIDs:{})", (unsigned long long) this, count, (unsigned long long) faceIDs);

        for(unsigned int i = 0; i < count; i++)
        {
            if(sparse[faceIDs[i]] != size - 1)
                glCopyNamedBufferSubData(buffer, buffer, sizeof(FaceMesh) * (size - 1), sizeof(FaceMesh) * sparse[faceIDs[i]], sizeof(FaceMesh));
            sparse[faceIDs[i]] = -1;
            size--;
        }

        ValidateAndReallocate();

        TRACE(DynamicFaceBuffer, "[{}:RemoveMany] (size:{})", (unsigned long long) this, size);
    }

    void DynamicFaceBuffer::Clear()
    {
        TRACE(DynamicFaceBuffer, "[{}:Clear] <>", (unsigned long long) this);

        size = 0;
    }

    void DynamicFaceBuffer::Bind() const
    {
        glBindVertexArray(vao);
    }

    void DynamicFaceBuffer::ValidateAndReallocate(unsigned int additional)
    {
        TRACE(DynamicFaceBuffer, "[{}:ValidateAndReallocate] (#additional:{}) (size:{}) (reserved:{})", (unsigned long long) this, additional, size, reserved);

        if(size + additional > reserved)
        {
            reserved = size + additional + STEP;
            glCopyNamedBufferSubData(buffer, g_service_buffer, 0, 0, size * sizeof(FaceMesh));
            glNamedBufferData(buffer, reserved * sizeof(FaceMesh), NULL, GL_DYNAMIC_DRAW);
            glCopyNamedBufferSubData(g_service_buffer, buffer, 0, 0, size * sizeof(FaceMesh));
        }
        else if(size + additional + STEP + HALF_STEP < reserved)
        {
            reserved = size + additional + HALF_STEP;
            glCopyNamedBufferSubData(buffer, g_service_buffer, 0, 0, size * sizeof(FaceMesh));
            glNamedBufferData(buffer, reserved * sizeof(FaceMesh), NULL, GL_DYNAMIC_DRAW);
            glCopyNamedBufferSubData(g_service_buffer, buffer, 0, 0, size * sizeof(FaceMesh));
        }

        TRACE(DynamicFaceBuffer, "[{}:ValidateAndReallocate] (reserved:{}) return", (unsigned long long) this, reserved);
    }

    ChunkMesh::ChunkMesh()
    {
        TRACE(ChunkMesh, "[{}:constructor] <>", (unsigned long long) this);
    }

    ChunkMesh::ChunkMesh(ChunkMesh&& other) :
        buffers
        {
            DynamicFaceBuffer(std::move(other.buffers[0])),
            DynamicFaceBuffer(std::move(other.buffers[1])),
            DynamicFaceBuffer(std::move(other.buffers[2]))
        }
    {
        TRACE(ChunkMesh, "[{}:move_constructor] (other:{}) <>", (unsigned long long) this, (unsigned long long) &other);
    }

    ChunkMesh::~ChunkMesh()
    {
        TRACE(ChunkMesh, "[{}:destructor] <>", (unsigned long long) this);
    }

    ChunkMesh& ChunkMesh::operator=(ChunkMesh&& other)
    {
        TRACE(ChunkMesh, "[{}:move_assignment] (other:{}) <>", (unsigned long long) this, (unsigned long long) &other);

        buffers[0] = std::move(other.buffers[0]);
        buffers[1] = std::move(other.buffers[1]);
        buffers[2] = std::move(other.buffers[2]);
        return *this;
    }

    static constexpr int encode(int x, int y, int z, int texture)
    {
        return (texture) | (z << 11) | (y << 18) | (x << 25);
    }

    static FaceMesh GenerateFaceMesh(const glm::ivec3& position, FaceOrientation orientation, const Game::Face& face)
    {
        FaceMesh mesh;

        int encoded = encode(position.x, position.y, position.z, ChunkRenderer::GetTextureID(face));
        switch(face.GetDirection())
        {
        case Game::Direction::SOUTH:
        case Game::Direction::WEST:
        case Game::Direction::DOWN:
            encoded |= (1 << 10);
            break;
        default:
            break;
        }

        switch(orientation)
        {
        case FaceOrientation::XOY:
            mesh.v[0].encoded = encoded + encode(0, 0, 0, 0);
            mesh.v[1].encoded = encoded + encode(1, 0, 0, 0);
            mesh.v[2].encoded = encoded + encode(1, 1, 0, 0);
            mesh.v[3].encoded = encoded + encode(0, 1, 0, 0);
            break;
        case FaceOrientation::YOZ:
            mesh.v[0].encoded = encoded + encode(0, 0, 0, 0);
            mesh.v[1].encoded = encoded + encode(0, 1, 0, 0);
            mesh.v[2].encoded = encoded + encode(0, 1, 1, 0);
            mesh.v[3].encoded = encoded + encode(0, 0, 1, 0);
            break;
        case FaceOrientation::ZOX:
            mesh.v[0].encoded = encoded + encode(0, 0, 0, 0);
            mesh.v[1].encoded = encoded + encode(0, 0, 1, 0);
            mesh.v[2].encoded = encoded + encode(1, 0, 1, 0);
            mesh.v[3].encoded = encoded + encode(1, 0, 0, 0);
            break;
        }

        switch(face.GetDirection())
        {
        case Game::Direction::SOUTH:
        case Game::Direction::WEST:
        case Game::Direction::DOWN:
            std::swap(mesh.v[1], mesh.v[3]);
            break;
        default:
            break;
        }

        return mesh;
    }

    static unsigned int FaceID(const glm::ivec3& position)
    {
        return position.z + (CHUNK_SIZE + 1) * (position.y + (CHUNK_SIZE + 1) * position.x);
    }

    void ChunkMesh::PlaceFace(const glm::ivec3& position, FaceOrientation orientation, Game::Face face)
    {
        TRACE(ChunkMesh, "[{}:PlaceFace] (#position:{}) (#orientation:{}) (#face:{})", (unsigned long long) this, position, orientation, face);

        FaceMesh mesh = GenerateFaceMesh(position, orientation, face);
        buffers[(unsigned int) orientation].Insert(FaceID(position), mesh);

        TRACE(ChunkMesh, "[{}:PlaceFace] return", (unsigned long long) this);
    }

    void ChunkMesh::RemoveFace(const glm::ivec3& position, FaceOrientation orientation)
    {
        TRACE(ChunkMesh, "[{}:RemoveFace] (#position:{}) (#orientation:{})", (unsigned long long) this, position, orientation);

        buffers[(unsigned int) orientation].Remove(FaceID(position));

        TRACE(ChunkMesh, "[{}:RemoveFace] return", (unsigned long long) this);
    }

    void ChunkMesh::PlaceManyFaces(unsigned int count, const glm::ivec3* positions, FaceOrientation orientation, const Game::Face* faces)
    {
        TRACE(ChunkMesh, "[{}:PlaceManyFaces] (#count:{}) (#positions:{}) (#orientation:{}) (#faces:{})", (unsigned long long) this, count, (unsigned long long) positions, orientation, (unsigned long long) faces);

        std::vector<FaceMesh> meshes(count);
        std::vector<unsigned int> faceIDs(count);
        for(unsigned int i = 0; i < count; i++)
        {
            meshes[i] = GenerateFaceMesh(positions[i], orientation, faces[i]);
            faceIDs[i] = FaceID(positions[i]);
        }

        buffers[(unsigned int) orientation].InsertMany(count, faceIDs.data(), meshes.data());

        TRACE(ChunkMesh, "[{}:PlaceManyFaces] return", (unsigned long long) this);
    }

    void ChunkMesh::PlaceManyFaces(unsigned int count, const glm::ivec3* positions, FaceOrientation orientation, Game::Face face)
    {
        TRACE(ChunkMesh, "[{}:PlaceManyFaces] (#count:{}) (#positions:{}) (#orientation:{}) (#face:{})", (unsigned long long) this, count, (unsigned long long) positions, orientation, face);

        std::vector<FaceMesh> meshes(count);
        std::vector<unsigned int> faceIDs(count);
        for(unsigned int i = 0; i < count; i++)
        {
            meshes[i] = GenerateFaceMesh(positions[i], orientation, face);
            faceIDs[i] = FaceID(positions[i]);
        }

        buffers[(unsigned int) orientation].InsertMany(count, faceIDs.data(), meshes.data());

        TRACE(ChunkMesh, "[{}:PlaceManyFaces] return", (unsigned long long) this);
    }

    void ChunkMesh::RemoveManyFaces(unsigned int count, const glm::ivec3* positions, FaceOrientation orientation)
    {
        TRACE(ChunkMesh, "[{}:RemoveManyFaces] (#count:{}) (#positions:{}) (#orientation:{})", (unsigned long long) this, count, (unsigned long long) positions, orientation);

        std::vector<unsigned int> faceIDs(count);
        for(unsigned int i = 0; i < count; i++)
            faceIDs[i] = FaceID(positions[i]);

        buffers[(unsigned int) orientation].RemoveMany(count, faceIDs.data());

        TRACE(ChunkMesh, "[{}:RemoveManyFaces] return", (unsigned long long) this);
    }

    void ChunkMesh::ClearFaces(FaceOrientation orientation)
    {
        TRACE(ChunkMesh, "[{}:ClearFaces] (#orientation:{}) <>", (unsigned long long) this, orientation);

        buffers[(unsigned int) orientation].Clear();
    }

    bool ChunkMesh::HasFace(FaceOrientation orientation, const glm::ivec3& position)
    {
        return buffers[(unsigned int) orientation].Has(FaceID(position));
    }

    void ChunkMesh::Bind(unsigned int buffer) const
    {
        buffers[buffer].Bind();
    }

    void Texture2D::Init(unsigned int _levels, unsigned int _channels, unsigned int _width, unsigned int _height)
    {
        levels = _levels; channels = _channels; width = _width; height = _height;
        format = (channels == 3 ? GL_RGB8 : GL_RGBA8);

        glCreateTextures(GL_TEXTURE_2D, 1, &texture);

        glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTextureStorage2D(texture, levels, format, width, height);
    }

    void Texture2D::Destroy()
    {
        glDeleteTextures(1, &texture);
    }

    void Texture2D::SetStyle(TextureStyle _style)
    {
        style = _style;

        switch(style)
        {
        case TextureStyle::PIXELATED:
            glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
            glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;
        case TextureStyle::SMOOTH:
            glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
        }
    }

    void Texture2D::SetData(const unsigned char* data, unsigned int level, unsigned int channels)
    {
        GLenum format = (channels == 3 ? GL_RGB : GL_RGBA);
        glTextureSubImage2D(texture, level, 0, 0, width >> level, height >> level, format, GL_UNSIGNED_BYTE, data);
    }

    void Texture2D::SetData(const unsigned char* data, unsigned int level, unsigned int channels, unsigned int x, unsigned int y, unsigned int width, unsigned int height)
    {
        GLenum format = (channels == 3 ? GL_RGB : GL_RGBA);
        glTextureSubImage2D(texture, level, x >> level, y >> level, width >> level, height >> level,format, GL_UNSIGNED_BYTE, data);
    }

    void Texture2D::CopyData(const Texture2D& other, unsigned int srcLevel, unsigned int srcX, unsigned int srcY, unsigned int dstLevel, unsigned int dstX, unsigned int dstY, unsigned int width, unsigned int height)
    {
        glCopyImageSubData(other.texture, GL_TEXTURE_2D, srcLevel, srcX >> srcLevel, srcY >> srcLevel, 0, texture, GL_TEXTURE_2D, dstLevel, dstX >> dstLevel, dstY >> dstLevel, 0, width >> srcLevel, height >> srcLevel, 1);
    }

    void Texture2D::GenerateMipmaps()
    {
        glGenerateTextureMipmap(texture);
    }

    void Texture2D::Bind(unsigned int slot) const
    {
        glBindTextureUnit(slot, texture);
        
    }

    void TextureAtlas::Init(unsigned int levels, unsigned int channels, unsigned int _subwidth, unsigned int _subheight, unsigned int reserved)
    {
        subwidth = _subwidth; subheight = _subheight;
        length = reserved; size = 0;
        texture.Init(levels, channels, subwidth * length, subheight * length);
        texture.SetStyle(TextureStyle::PIXELATED);
    }

    void TextureAtlas::Destroy()
    {
        texture.Destroy();
    }

    int TextureAtlas::Add(const Texture2D& texture)
    {
        ValidateAndReallocate();
        return InternalAdd(texture);
    }

    int TextureAtlas::Add(const std::vector<Texture2D>& textures)
    {
        int idx = size;
        ValidateAndReallocate(textures.size());
        for(const Texture2D& texture : textures)
            InternalAdd(texture);
        return idx;
    }

    void TextureAtlas::Bind(unsigned int slot) const
    {
        texture.Bind(slot);
    }

    void TextureAtlas::ValidateAndReallocate(unsigned int additional)
    {
        unsigned int newLength = length;
        while(size + additional > newLength * newLength)
            newLength += STEP;
        //while(size + additional + 2 * newLength * (STEP + HALF_STEP) < newLength * newLength + (STEP + HALF_STEP) * (STEP + HALF_STEP))
        //    newLength -= STEP;

        if(length != newLength)
        {
            Texture2D newTexture;
            newTexture.Init(texture.GetLevels(), texture.GetChannelCount(), subwidth * newLength, subheight * newLength);
            newTexture.SetStyle(TextureStyle::PIXELATED);
            for(unsigned int level = 0; level < texture.GetLevels(); level++)
                for(unsigned int i = 0; i < size; i++)
                {
                    glm::ivec2 position = { i / length, i % length };
                    glm::ivec2 newPosition = { i / newLength, i % newLength };

                    newTexture.CopyData(texture, level, position.x * subwidth, position.y * subheight, level, newPosition.x * subwidth, newPosition.y * subheight, subwidth, subheight);
                }

            texture.Destroy();
            texture = newTexture;
        }
    }

    int TextureAtlas::InternalAdd(const Texture2D& _texture)
    {
        int idx = size++;
        glm::ivec2 position = glm::ivec2 { idx / length, idx % length } * glm::ivec2 { subwidth, subheight };
        for(unsigned int level = 0; level < texture.GetLevels(); level++)
            texture.CopyData(_texture, level, 0, 0, level, position.x, position.y, subwidth, subheight);
        return idx;
    }

    static const char* ShaderTypeToString(unsigned int type)
    {
        switch(type)
        {
        case GL_VERTEX_SHADER: return "vertex";
        case GL_FRAGMENT_SHADER: return "fragment";
        default: return "";
        }
    }

    static unsigned int CreateShader(const std::string& src, unsigned int type)
    {
        TRACE(Shaders, "[CreateShader] (#src:{}) (#type:{})", src, type);

        const char* data = src.data();
        int length = src.length();

        unsigned int shader = glCreateShader(type);
        TRACE(Shaders, "[CreateShader] (shader:{})", shader);

        glShaderSource(shader, 1, &data, &length);

        TRACE(Shaders, "[CreateShader] compile", shader);

        glCompileShader(shader);

        TRACE(Shaders, "[CreateShader] compile return");

        int status;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
        if(status != GL_TRUE)
        {
            int log_length;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);

            std::vector<char> message(log_length + 1, 0);
            glGetShaderInfoLog(shader, log_length, &log_length, message.data());

            ERROR(Shaders, "[CreateShader] (message:{}) compile error", message.data());

            glDeleteShader(shader);
            return 0;
        }

        TRACE(Shaders, "[CreateShader] return");
        return shader;
    }

    void VFShader::Init(const std::string& vertexSrc, const std::string& fragmentSrc)
    {
        TRACE(Shaders, "[{}:Init] (#vertexSrc:{}) (#fragmentSrc:{})", (unsigned long long) this, vertexSrc, fragmentSrc);

        unsigned int vertexShader = CreateShader(vertexSrc, GL_VERTEX_SHADER);
        unsigned int fragmentShader = CreateShader(fragmentSrc, GL_FRAGMENT_SHADER);

        TRACE(Shaders, "[{}:Init] (vertexShader:{}) (fragmentShader:{})", (unsigned long long) this, vertexShader, fragmentShader);

        if(vertexShader == 0)
        {
            ERROR(Shaders, "[{}:Init] vertex shader create error", (unsigned long long) this);
            return;
        }

        if(fragmentShader == 0)
        {
            ERROR(Shaders, "[{}:Init] fragment shader create error", (unsigned long long) this);
            return;
        }

        program = glCreateProgram();

        TRACE(Shaders, "[{}:Init] (program:{})", (unsigned long long) this, program);
        
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);

        TRACE(Shaders, "[{}:Init] link", (unsigned long long) this);

        glLinkProgram(program);

        TRACE(Shaders, "[{}:Init] link return", (unsigned long long) this);

        glDetachShader(program, vertexShader);
        glDetachShader(program, fragmentShader);

        TRACE(Shaders, "[{}:Init] (vertexShader:{}) (fragmentShader:{}) vertex, fragment deleted");

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        int status;
        glGetProgramiv(program, GL_LINK_STATUS, &status);
        if(status != GL_TRUE)
        {
            int log_length;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);

            std::vector<char> message(log_length + 1, 0);
            glGetProgramInfoLog(program, log_length, &log_length, message.data());

            ERROR(Shaders, "[{}:Init] (message:{}) link error", (unsigned long long) this, message.data());

            glDeleteProgram(program);
            return;
        }

        TRACE(Shaders, "[{}:Init] return", (unsigned long long) this);
    }

    void VFShader::Destroy()
    {
        TRACE(Shaders, "[{}:Destroy] (program:{}) <>", (unsigned long long) this, program);

        glDeleteProgram(program);
    }

    void VFShader::Bind() const
    {
        glUseProgram(program);
    }

    int VFShader::GetUniformLocation(const std::string& name)
    {
        int location = glGetUniformLocation(program, name.c_str());

        TRACE(Shaders, "[{}:GetUniformLocation] (#name:{}) (location: {}) <>", (unsigned long long) this, name, location);
        return location;
    }

    static void MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar const* message, void const* user_param)
    {
        if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

        const char* src_str;
        switch(source)
        {
        case GL_DEBUG_SOURCE_API: src_str = "API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM: src_str = "WINDOW SYSTEM"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: src_str = "SHADER COMPILER"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY: src_str = "THIRD PARTY"; break;
        case GL_DEBUG_SOURCE_APPLICATION: src_str = "APPLICATION"; break;
        case GL_DEBUG_SOURCE_OTHER: src_str = "OTHER"; break;
        }

        const char* type_str;
        switch(type)
        {
        case GL_DEBUG_TYPE_ERROR: type_str = "ERROR"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: type_str = "DEPRECATED BEHAVIOR"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: type_str = "UNDEFINED BEHAVIOR"; break;
        case GL_DEBUG_TYPE_PORTABILITY: type_str = "PORTABILITY"; break;
        case GL_DEBUG_TYPE_PERFORMANCE: type_str = "PERFORMANCE"; break;
        case GL_DEBUG_TYPE_MARKER: type_str = "MARKER"; break;
        case GL_DEBUG_TYPE_OTHER: type_str = "OTHER"; break;
        }

        switch(severity)
        {
        case GL_DEBUG_SEVERITY_NOTIFICATION: INFO(OpenGLInternal, "[Notification] (source:{}) (type:{}) (ID:{}) {}", src_str, type_str, id, message); break;
        case GL_DEBUG_SEVERITY_LOW: WARN(OpenGLInternal, "[Low] (source:{}) (type:{}) (ID:{}) {}", src_str, type_str, id, message); break;
        case GL_DEBUG_SEVERITY_MEDIUM: ERROR(OpenGLInternal, "[Medium] (source:{}) (type:{}) (ID:{}) {}", src_str, type_str, id, message); break;
        case GL_DEBUG_SEVERITY_HIGH: CRITICAL(OpenGLInternal, "[High] (source:{}) (type:{}) (ID:{}) {}", src_str, type_str, id, message); break;
        default: CRITICAL(OpenGLInternal, "[Other] (source:{}) (type:{}) (ID:{}) {}", src_str, type_str, id, message); break;
        }
    }

    void Renderer::Init()
    {
        TRACE(Renderer, "[Init]");

        TRACE(Renderer, "[Init] load OpenGL");
        GLFWwindow* window = Window::GetInternalWindow();

        glfwMakeContextCurrent(window);
        if(!gladLoadGL(glfwGetProcAddress))
        {
            CRITICAL(Renderer, "[Init] failed to load OpenGL");
            return;
        }

        glfwSwapInterval(0);

        TRACE(Renderer, "[Init] load OpenGL return");

        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(MessageCallback, nullptr);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        glCreateBuffers(1, &g_service_buffer);
        glNamedBufferStorage(g_service_buffer, 1024 * 1024 * 128, NULL, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);

        TRACE(Renderer, "[Init] (g_service_buffer:{})", g_service_buffer);

        ChunkRenderer::Init();

        TRACE(Renderer, "[Init] return");
    }

    void Renderer::Destroy()
    {
        TRACE(Renderer, "[Destroy]");

        ChunkRenderer::Destroy();

        glDeleteBuffers(1, &g_service_buffer);
    
        TRACE(Renderer, "[Destroy] return");
    }

    void Renderer::Render(Game::World& world, const Game::Player& player)
    {
        glClearColor(0.1f, 0.2f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ChunkRenderer::SetViewMatrix(glm::inverse(player.GetTransform().GetMatrix()));
        ChunkRenderer::SetProjectionMatrix(player.GetProjection());

        {
            std::lock_guard<Game::World> guard(world);
            for(auto& chunk : world.GetChunks())
            {
                if(!chunk.IsGeometryBuilt())
                    chunk.BuildGeometry();
            }

            ChunkRenderer::Begin();
            ChunkRenderer::Render(world);
            ChunkRenderer::End();
        }

        GLFWwindow* window = Window::GetInternalWindow();
        glfwSwapBuffers(window);
    }

    TextureAtlas ChunkRenderer::atlas;
    VFShader ChunkRenderer::shader[3];
    int ChunkRenderer::model_location[3], ChunkRenderer::vp_location[3], ChunkRenderer::atlas_dimensions_location[3], ChunkRenderer::texture_size_location[3], ChunkRenderer::texture_location[3];
    glm::mat4 ChunkRenderer::view_matrix, ChunkRenderer::projection_matrix;
    std::unordered_map<std::string, int> ChunkRenderer::texture_IDs;
    std::vector<ChunkRenderer::BlockData> ChunkRenderer::blocks;

    void ChunkRenderer::Init()
    {
        TRACE(ChunkRenderer, "[Init]");

        unsigned int face_count = (CHUNK_SIZE + 1) * (CHUNK_SIZE + 1) * (CHUNK_SIZE + 1);
        std::vector<unsigned int> indices(face_count * 6);
        for(unsigned int i = 0; i < face_count; i++)
        {
            indices[6 * i + 0] = 4 * i + 0;
            indices[6 * i + 1] = 4 * i + 1;
            indices[6 * i + 2] = 4 * i + 2;
            indices[6 * i + 3] = 4 * i + 2;
            indices[6 * i + 4] = 4 * i + 3;
            indices[6 * i + 5] = 4 * i + 0;
        }

        glCreateBuffers(1, &g_ebo);
        TRACE(ChunkRenderer, "[Init] (face_count:{}) (g_ebo:{})", face_count, g_ebo);

        glNamedBufferStorage(g_ebo, sizeof(unsigned int) * face_count * 6, indices.data(), GL_DYNAMIC_STORAGE_BIT);

        shader[0].Init(Files::ReadFile(PATH("shaders/chunk_packed_64x64x64_xy.vert.glsl")), Files::ReadFile(PATH("shaders/chunk_packed_64x64x64.frag.glsl")));
        vp_location[0] = shader[0].GetUniformLocation("u_vp");
        model_location[0] = shader[0].GetUniformLocation("u_model");
        atlas_dimensions_location[0] = shader[0].GetUniformLocation("u_atlasDimensions");
        texture_size_location[0] = shader[0].GetUniformLocation("u_textureSize");
        texture_location[0] = shader[0].GetUniformLocation("u_texture");

        TRACE(ChunkRenderer, "[Init] (vp_location[0]:{}) (model_location[0]:{})", vp_location[0], model_location[0]);

        shader[1].Init(Files::ReadFile(PATH("shaders/chunk_packed_64x64x64_yz.vert.glsl")), Files::ReadFile(PATH("shaders/chunk_packed_64x64x64.frag.glsl")));
        vp_location[1] = shader[1].GetUniformLocation("u_vp");
        model_location[1] = shader[1].GetUniformLocation("u_model");
        atlas_dimensions_location[1] = shader[1].GetUniformLocation("u_atlasDimensions");
        texture_size_location[1] = shader[1].GetUniformLocation("u_textureSize");
        texture_location[1] = shader[1].GetUniformLocation("u_texture");

        TRACE(ChunkRenderer, "[Init] (vp_location[1]:{}) (model_location[1]:{})", vp_location[1], model_location[1]);

        shader[2].Init(Files::ReadFile(PATH("shaders/chunk_packed_64x64x64_zx.vert.glsl")), Files::ReadFile(PATH("shaders/chunk_packed_64x64x64.frag.glsl")));
        vp_location[2] = shader[2].GetUniformLocation("u_vp");
        model_location[2] = shader[2].GetUniformLocation("u_model");
        atlas_dimensions_location[2] = shader[2].GetUniformLocation("u_atlasDimensions");
        texture_size_location[2] = shader[2].GetUniformLocation("u_textureSize");
        texture_location[2] = shader[2].GetUniformLocation("u_texture");

        TRACE(ChunkRenderer, "[Init] (vp_location[2]:{}) (model_location[2]:{})", vp_location[2], model_location[2]);

        for(unsigned int i = 0; i < 3; i++)
        {
            if(vp_location[i] == -1)
            {
                CRITICAL(ChunkRenderer, "[Init] vp_location[{}] missing error", i);
                return;
            }

            if(model_location[i] == -1)
            {
                CRITICAL(ChunkRenderer, "[Init] model_location[{}] missing error", i);
                return;
            }

            if(atlas_dimensions_location[i] == -1)
            {
                CRITICAL(ChunkRenderer, "[Init] atlas_dimensions_location[{}] missing error", i);
                return;
            }

            if(texture_size_location[i] == -1)
            {
                CRITICAL(ChunkRenderer, "[Init] texture_size_location[{}] missing error", i);
                //return;
            }

            if(texture_location[i] == -1)
            {
                CRITICAL(ChunkRenderer, "[Init] texture_location[{}] missing error", i);
                //return;
            }
        }

        atlas.Init(4, 3, 32, 32);

        TRACE(ChunkRenderer, "[Init] return");
    }

    void ChunkRenderer::Destroy()
    {
        TRACE(ChunkRenderer, "[Destroy]");

        atlas.Destroy();

        for(unsigned int i = 0; i < 3; i++)
            shader[i].Destroy();

        glDeleteBuffers(1, &g_ebo);

        TRACE(ChunkRenderer, "[Destroy] return");
    }

    void ChunkRenderer::Begin()
    {
    }

    void ChunkRenderer::Render(const Game::World& world)
    {
        glm::mat4 vp_matrix = projection_matrix * view_matrix;

        atlas.Bind(0);

        for(unsigned int i = 0; i < 3; i++)
        {
            shader[i].Bind();

            glUniformMatrix4fv(vp_location[i], 1, GL_FALSE, glm::value_ptr(vp_matrix));
            glUniform2i(atlas_dimensions_location[i], atlas.GetDimensions().x, atlas.GetDimensions().y);
            glUniform2f(texture_size_location[i], atlas.GetTextureSize().x, atlas.GetTextureSize().y);
            glUniform1i(texture_location[i], 0);

            for(const Game::Chunk& chunk : world.GetChunks())
            {
                glm::mat4 model_matrix = chunk.GetTransform().GetMatrix();
                glUniformMatrix4fv(model_location[i], 1, GL_FALSE, glm::value_ptr(model_matrix));
                
                chunk.GetMesh().Bind(i);
                glDrawElements(GL_TRIANGLES, chunk.GetMesh().Size(i) * 6, GL_UNSIGNED_INT, NULL);
            }
        }
    }

    void ChunkRenderer::End()
    {
    }

    int ChunkRenderer::GetTextureID(const Game::Face& face)
    {
        Game::BlockID block = face.GetBlock();
        Game::Direction direction = face.GetDirection();

        if(blocks.size() <= block)
            blocks.resize(block + 1);

        switch(direction)
        {
        case Game::Direction::NORTH:
            if(blocks[block].north == -1)
                blocks[block].north = LoadTexture(Game::BlockBase::GetFaceTexture(block, direction));
            return blocks[block].north;
        
        case Game::Direction::SOUTH:
            if(blocks[block].south == -1)
                blocks[block].south = LoadTexture(Game::BlockBase::GetFaceTexture(block, direction));
            return blocks[block].south;
        
        case Game::Direction::EAST:
            if(blocks[block].east == -1)
                blocks[block].east = LoadTexture(Game::BlockBase::GetFaceTexture(block, direction));
            return blocks[block].east;
        
        case Game::Direction::WEST:
            if(blocks[block].west == -1)
                blocks[block].west = LoadTexture(Game::BlockBase::GetFaceTexture(block, direction));
            return blocks[block].west;
        
        case Game::Direction::UP:
            if(blocks[block].up == -1)
                blocks[block].up = LoadTexture(Game::BlockBase::GetFaceTexture(block, direction));
            return blocks[block].up;

        case Game::Direction::DOWN:
            if(blocks[block].down == -1)
                blocks[block].down = LoadTexture(Game::BlockBase::GetFaceTexture(block, direction));
            return blocks[block].down;
        }
    }

    int ChunkRenderer::LoadTexture(const std::string& path)
    {
        if(texture_IDs.find(path) == texture_IDs.end())
        {
            stbi_set_flip_vertically_on_load(true);

            int width, height, channels;
            unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);

            if(data == nullptr)
            {
                ERROR(ChunkRenderer, "[LoadTexture] (#path:{}) failed to load texture", path);
                return -1;
            }

            if(width != atlas.GetTextureSize().x)
            {
                ERROR(ChunkRenderer, "[LoadTexture] (#path:{}) (width:{}) (height:{}) (atlas.GetTextureSize().x:{}) width not valid", path, width, height, atlas.GetTextureSize().x);
                return -1;
            }

            if(height != atlas.GetTextureSize().y)
            {
                ERROR(ChunkRenderer, "[LoadTexture] (#path:{}) (width:{}) (height:{}) (atlas.GetTextureSize().y:{}) height not valid", path, width, height, atlas.GetTextureSize().y);
                return -1;
            }
            
            Texture2D texture;
            texture.Init(4, 3, width, height);
            texture.SetData(data, 0, channels);
            texture.GenerateMipmaps();

            stbi_image_free(data);

            texture_IDs[path] = atlas.Add(texture);
            texture.Destroy();
        }

        return texture_IDs[path];
    }

    RenderThread RenderThread::instance;
    std::thread RenderThread::thread;
    bool RenderThread::done;

    void RenderThread::Start()
    {
        thread = std::thread(RenderThread::Run);
    }

    void RenderThread::Stop()
    {
        done = true;
        thread.join();
    }

    void RenderThread::Run()
    {
        Renderer::Init();
        
        while(!done)
        {
            
        }

        Renderer::Destroy();
    }
};

