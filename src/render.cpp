#include "render.h"
#include "gui/render.h"
#include "format/render.h"

#include <glad/gl.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <implot.h>

#include <vector>
#include <cassert>

#include "format/glm.h"
#include "format/blocks.h"

#include "gui/aabb.h"
#include "gui/blocks.h"

#include "profiler.h"
#include "files.h"
#include "window.h"
#include "stb_image.h"

#include "mathematics.h"
#include "aabb.h"
#include "frustum.h"

#include "world.h"
#include "player.h"

#include "application.h"

DEFINE_LOG_CATEGORY(OpenGLInternal, FILE_LOGGER(trace, LOGFILE("Render/OpenGLInternal.txt")));
DEFINE_LOG_CATEGORY(Shaders, FILE_LOGGER(trace, LOGFILE("Render/Shaders.txt")));
DEFINE_LOG_CATEGORY(FaceBuffer, FILE_LOGGER(trace, LOGFILE("Render/FaceBuffer.txt")));
DEFINE_LOG_CATEGORY(ChunkMesh, FILE_LOGGER(trace, LOGFILE("Render/ChunkMesh.txt")));
DEFINE_LOG_CATEGORY(Renderer, FILE_LOGGER(trace, LOGFILE("Render/Renderer.txt")));
DEFINE_LOG_CATEGORY(ChunkRenderer, FILE_LOGGER(trace, LOGFILE("Render/ChunkRenderer.txt")));

namespace Render
{
    static std::optional<Buffer> g_ebo;

    Buffer::Buffer() :
        bytes(0), usage(GL_STATIC_DRAW)
    {
        glCreateBuffers(1, &buffer);
    }

    Buffer::Buffer(unsigned int usage) :
        bytes(0), usage(usage)
    {
        glCreateBuffers(1, &buffer);
    }

    Buffer::Buffer(Buffer&& other) :
        bytes(other.bytes), buffer(other.buffer), usage(other.usage)
    {
        other.buffer = 0;
    }

    Buffer::Buffer(const Buffer& other)
    {
        bytes = other.bytes;
        usage = other.usage;
        glCreateBuffers(1, &buffer);
        glNamedBufferData(buffer, bytes, NULL, usage);
        glCopyNamedBufferSubData(other.buffer, buffer, 0, 0, bytes);
    }

    Buffer::~Buffer()
    {
        if(buffer)
            glDeleteBuffers(1, &buffer);
    }

    Buffer& Buffer::operator=(Buffer&& other)
    {
        if(buffer)
            glDeleteBuffers(1, &buffer);

        bytes = other.bytes;
        buffer = other.buffer;
        usage = other.usage;
        other.buffer = 0;
    
        return *this;
    }

    Buffer& Buffer::operator=(const Buffer& other)
    {
        bytes = other.bytes;
        usage = other.usage;
        glNamedBufferData(buffer, bytes, NULL, usage);
        glCopyNamedBufferSubData(other.buffer, buffer, 0, 0, bytes);
        return *this;
    }

    void Buffer::SetData(unsigned int _bytes, const void* data)
    {
        bytes = _bytes;
        glNamedBufferData(buffer, bytes, data, usage);
    }

    void Buffer::Clear()
    {
        bytes = 0;
        glNamedBufferData(buffer, 0, NULL, usage);
    }

    void Buffer::Bind(unsigned int target) const
    {
        glBindBuffer(target, buffer);
    }

    void Buffer::Unbind(unsigned int target) const
    {
        glBindBuffer(target, 0);
    }

    void Buffer::BindBase(unsigned int target, unsigned int index) const
    {
        glBindBufferBase(target, index, buffer);
    }

    void Buffer::UnbindBase(unsigned int target, unsigned int index) const
    {
        glBindBufferBase(target, index, 0);
    }

    VertexArray::VertexArray() :
        vertex_buffers(0), attribs(0)
    {
        glCreateVertexArrays(1, &vao);
    }

    VertexArray::VertexArray(VertexArray&& other) :
        vao(other.vao), vertex_buffers(other.vertex_buffers), attribs(other.attribs)
    {
        other.vao = 0;
    }

    VertexArray::~VertexArray()
    {
        if(vao)
            glDeleteVertexArrays(1, &vao);
    }

    VertexArray& VertexArray::operator=(VertexArray&& other)
    {
        if(vao)
            glDeleteVertexArrays(1, &vao);

        vao = other.vao;
        vertex_buffers = other.vertex_buffers;
        attribs = other.attribs;
        other.vao = 0;

        return *this;
    }

    unsigned int VertexArray::AddVertexBuffer(const Buffer& vbo, unsigned int offset, unsigned int stride)
    {
        glVertexArrayVertexBuffer(vao, vertex_buffers, vbo.Get(), offset, stride);
        return vertex_buffers++;
    }

    void VertexArray::SetElementBuffer(const Buffer& ebo)
    {
        glVertexArrayElementBuffer(vao, ebo.Get());
    }

    void VertexArray::AddVertexArrayAttrib(unsigned int buffer, int size, unsigned int type, unsigned int offset)
    {
        glEnableVertexArrayAttrib(vao, attribs);
        switch(type)
        {
        case GL_INT:
            glVertexArrayAttribIFormat(vao, attribs, size, type, offset);
            break;
        default:
            glVertexArrayAttribFormat(vao, attribs, size, type, GL_FALSE, offset);
            break;
        }
        glVertexArrayAttribBinding(vao, attribs++, buffer);
    }

    void VertexArray::Clear()
    {
        for(unsigned int i = 0; i < attribs; i++)
            glDisableVertexArrayAttrib(vao, i);
        attribs = 0;

        glVertexArrayElementBuffer(vao, 0);

        for(unsigned int i = 0; i < vertex_buffers; i++)
            glVertexArrayVertexBuffer(vao, i, 0, 0, 0);
        vertex_buffers = 0;
    }

    void VertexArray::Bind() const
    {
        glBindVertexArray(vao);
    }

    void VertexArray::Unbind() const
    {
        glBindVertexArray(0);
    }
    
    FaceBuffer::FaceBuffer() :
        vao(), buffer(GL_STATIC_DRAW)
    {
        BuildVAO();
        size = 0;
    }

    FaceBuffer::FaceBuffer(FaceBuffer&& other) :
        vao(std::move(other.vao)), buffer(std::move(other.buffer)), size(other.size)
    {
    }

    FaceBuffer::FaceBuffer(const FaceBuffer& other) :
        vao(), buffer(other.buffer)
    {
        BuildVAO();
        size = other.size;
    }

    FaceBuffer::~FaceBuffer()
    {
    }

    FaceBuffer& FaceBuffer::operator=(FaceBuffer&& other)
    {
        vao = std::move(other.vao);
        buffer = std::move(other.buffer);
        size = other.size;
        return *this;
    }

    FaceBuffer& FaceBuffer::operator=(const FaceBuffer& other)
    {
        vao.Clear();
        buffer = other.buffer;
        BuildVAO();
        size = other.size;
        return *this;
    }

    void FaceBuffer::SetData(unsigned int count, const FaceMesh* meshes)
    {
        size = count;
        if(size != 0)
            buffer.SetData(size * sizeof(FaceMesh), meshes);
        else buffer.Clear();
    }

    void FaceBuffer::Clear()
    {
        size = 0;
        buffer.Clear();
    }

    void FaceBuffer::BuildVAO()
    {
        unsigned int buffer_idx = vao.AddVertexBuffer(buffer, 0, sizeof(Vertex));
        vao.SetElementBuffer(g_ebo.value());

        vao.AddVertexArrayAttrib(buffer_idx, 1, GL_INT, offsetof(Vertex, encoded));
    }

    ChunkMesh::ChunkMesh()
    {
        TRACE(ChunkMesh, "[{}:constructor] <>", (unsigned long long) this);
    }

    ChunkMesh::ChunkMesh(ChunkMesh&& other) :
        buffers
        {
            FaceBuffer(std::move(other.buffers[0])),
            FaceBuffer(std::move(other.buffers[1])),
            FaceBuffer(std::move(other.buffers[2]))
        },
        model_matrix(std::move(other.model_matrix))
    {
        TRACE(ChunkMesh, "[{}:move_constructor] (other:{}) <>", (unsigned long long) this, (unsigned long long) &other);
    }

    ChunkMesh::ChunkMesh(const ChunkMesh& other) :
        buffers
        {
            other.buffers[0],
            other.buffers[1],
            other.buffers[2]
        },
        model_matrix(other.model_matrix)
    {
        TRACE(ChunkMesh, "[{}:copy_constructor] (other:{}) <>", (unsigned long long) this, (unsigned long long) &other);
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
        model_matrix = std::move(other.model_matrix);
        return *this;
    }

    ChunkMesh& ChunkMesh::operator=(const ChunkMesh& other)
    {
        buffers[0] = other.buffers[0];
        buffers[1] = other.buffers[1];
        buffers[2] = other.buffers[2];
        model_matrix = other.model_matrix;
        return *this;
    }

    static constexpr int encode(int x, int y, int z, int texture)
    {
        return (texture) | (z << 11) | (y << 18) | (x << 25);
    }

    static FaceMesh GenerateFaceMesh(const glm::ivec3& position, FaceOrientation orientation, const Game::Face& face)
    {
        FaceMesh mesh;

        int encoded = 0;//encode(position.x, position.y, position.z, ChunkRenderer::GetTextureID(face));
        switch(face.direction)
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

        switch(face.direction)
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

    void ChunkMesh::SetFaces(unsigned int count, const FaceMesh* meshes, FaceOrientation orientation)
    {
        buffers[(unsigned int) orientation].SetData(count, meshes);
    }

    void ChunkMesh::Clear(FaceOrientation orientation)
    {
        buffers[(unsigned int) orientation].Clear();
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

    Shader::Shader(unsigned int type)
    {
        shader = glCreateShader(type);
    }

    Shader::Shader(Shader&& other) :
        shader(other.shader)
    {
        other.shader = 0;
    }

    Shader::~Shader()
    {
        if(shader)
            glDeleteShader(shader);
    }

    Shader& Shader::operator=(Shader&& other)
    {
        if(shader)
            glDeleteShader(shader);

        shader = other.shader;
        other.shader = 0;

        return *this;
    }

    void Shader::SetSource(const char* src)
    {
        glShaderSource(shader, 1, &src, NULL);
    }

    bool Shader::Compile()
    {
        glCompileShader(shader);

        int status;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
        if(status != GL_TRUE)
        {
            int log_length;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);

            std::vector<char> message(log_length + 1, 0);
            glGetShaderInfoLog(shader, log_length, &log_length, message.data());
            message.back() = '\0';

            ERROR(Shaders, "Failed to compile shader! Error:\n{}\n", message.data());

            return false;
        }

        return true;
    }

    Program::Program()
    {
        program = glCreateProgram();
    }

    Program::Program(Program&& other) :
        program(other.program)
    {
        other.program = 0;
    }

    Program::~Program()
    {
        if(program)
            glDeleteProgram(program);
    }

    Program& Program::operator=(Program&& other)
    {
        if(program)
            glDeleteProgram(program);

        program = other.program;
        other.program = 0;

        return *this;
    }

    void Program::Attach(const Shader& shader)
    {
        glAttachShader(program, shader.Get());
    }

    void Program::Detach(const Shader& shader)
    {
        glDetachShader(program, shader.Get());
    }

    bool Program::Link()
    {
        glLinkProgram(program);

        int status;
        glGetProgramiv(program, GL_LINK_STATUS, &status);
        if(status != GL_TRUE)
        {
            int log_length;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);

            std::vector<char> message(log_length + 1, 0);
            glGetProgramInfoLog(program, log_length, &log_length, message.data());
            message.back() = '\0';

            ERROR(Shaders, "Failed to link program! Error:\n{}\n", message.data());
            return false;
        }

        return true;
    }

    int Program::GetUniformLocation(const char* name) const
    {
        return glGetUniformLocation(program, name);
    }

    unsigned int Program::GetUniformBlockIndex(const char* name) const
    {
        return glGetUniformBlockIndex(program, name);
    }

    unsigned int Program::GetShaderStorageBlockIndex(const char* name) const
    {
        return glGetProgramResourceIndex(program, GL_SHADER_STORAGE_BLOCK, name);
    }

    void Program::BindUniformBlock(unsigned int target, unsigned int binding) const
    {
        glUniformBlockBinding(program, target, binding);
    }

    void Program::UnbindUniformBlock(unsigned int target) const
    {
        glUniformBlockBinding(program, target, 0);
    }

    void Program::BindShaderStorageBlock(unsigned int target, unsigned int binding) const
    {
        glShaderStorageBlockBinding(program, target, binding);
    }

    void Program::UnbindShaderStorageBlock(unsigned int target) const
    {
        glShaderStorageBlockBinding(program, target, 0);
    }

    void Program::Bind() const
    {
        glUseProgram(program);
    }

    void Program::Unbind() const
    {
        glUseProgram(0);
    }

    VFShader::VFShader(const char* vertex_src, const char* fragment_src) :
        Program()
    {
        Shader vertex_shader(GL_VERTEX_SHADER);
        vertex_shader.SetSource(vertex_src);
        if(!vertex_shader.Compile())
        {
            ERROR(Shaders, "Failed to compile vertex shader!");
            return;
        }

        Shader fragment_shader(GL_FRAGMENT_SHADER);
        fragment_shader.SetSource(fragment_src);
        if(!fragment_shader.Compile())
        {
            ERROR(Shaders, "Failed to compile fragment shader!");
            return;
        }

        Attach(vertex_shader);
        Attach(fragment_shader);

        if(!Link())
        {
            ERROR(Shaders, "Failed to link VF program!");
            return;
        }

        Detach(fragment_shader);
        Detach(vertex_shader);
    }

    ComputeShader::ComputeShader(const char* src) :
        Program()
    {
        Shader compute_shader(GL_COMPUTE_SHADER);
        compute_shader.SetSource(src);
        if(!compute_shader.Compile())
        {
            ERROR(Shaders, "Failed to create compute shader!");
            return;
        }

        Attach(compute_shader);
        
        if(!Link())
        {
            ERROR(Shaders, "Failed to link compute program!");
            return;
        }

        Detach(compute_shader);
    }

    static unsigned int TextureFormatToGLFormat(TextureFormat format)
    {
        switch(format)
        {
        case TextureFormat::R8: return GL_R8;
        case TextureFormat::R32: return GL_R32F;
        case TextureFormat::RGB8: return GL_RGB8;
        case TextureFormat::RGB32: return GL_RGB32F;
        case TextureFormat::RGBA8: return GL_RGBA8;
        case TextureFormat::RGBA32: return GL_RGBA32F;
        default: return GL_NONE;
        }
    }

    static unsigned int TextureFormatToGLFormatType(TextureFormat format)
    {
        switch(format)
        {
        case TextureFormat::R8:
        case TextureFormat::R32: return GL_RED;
        case TextureFormat::RGB8:
        case TextureFormat::RGB32: return GL_RGB;
        case TextureFormat::RGBA8:
        case TextureFormat::RGBA32: return GL_RGBA;
        default: return GL_NONE;
        }
    }

    static unsigned int TextureFormatToGLType(TextureFormat format)
    {
        switch(format)
        {
        case TextureFormat::R8:
        case TextureFormat::RGB8:
        case TextureFormat::RGBA8: return GL_UNSIGNED_BYTE;
        case TextureFormat::R32:
        case TextureFormat::RGB32:
        case TextureFormat::RGBA32: return GL_FLOAT;
        default: return GL_NONE;
        }
    }

    static unsigned int TextureFormatTypeToGLFormatType(TextureFormatType format)
    {
        switch(format)
        {
        case TextureFormatType::RED: return GL_RED;
        case TextureFormatType::RGB: return GL_RGB;
        case TextureFormatType::RGBA: return GL_RGBA;
        default: return GL_NONE;
        }
    }

    Texture2D::Texture2D(unsigned int _levels, TextureFormat _format, unsigned int _width, unsigned int _height)
    {
        format = _format;
        levels = _levels; width = _width; height = _height;

        glCreateTextures(GL_TEXTURE_2D, 1, &texture);

        glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTextureStorage2D(texture, levels, TextureFormatToGLFormat(format), width, height);
    }

    Texture2D::Texture2D(Texture2D&& other) :
        texture(other.texture), width(other.width), height(other.height), levels(other.levels), format(other.format), style(other.style)
    {
        other.texture = 0;
    }

    Texture2D::~Texture2D()
    {
        if(texture != 0) glDeleteTextures(1, &texture);
    }

    Texture2D& Texture2D::operator=(Texture2D&& other)
    {
        if(texture != 0) glDeleteTextures(1, &texture);

        texture = other.texture;
        width = other.width; height = other.height;
        levels = other.levels; style = other.style;

        other.texture = 0;
        return *this;
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

    void Texture2D::SetData(const unsigned char* data, unsigned int level, TextureFormatType _format)
    {
        unsigned int format = TextureFormatTypeToGLFormatType(_format);
        glTextureSubImage2D(texture, level, 0, 0, width >> level, height >> level, format, GL_UNSIGNED_BYTE, data);
    }

    void Texture2D::SetData(const unsigned char* data, unsigned int level, TextureFormatType _format, unsigned int x, unsigned int y, unsigned int width, unsigned int height)
    {
        unsigned int format = TextureFormatTypeToGLFormatType(_format);
        glTextureSubImage2D(texture, level, x >> level, y >> level, width >> level, height >> level,format, GL_UNSIGNED_BYTE, data);
    }

    void Texture2D::CopyData(const Texture2D& other, unsigned int srcLevel, unsigned int srcX, unsigned int srcY, unsigned int dstLevel, unsigned int dstX, unsigned int dstY, unsigned int width, unsigned int height)
    {
        glCopyImageSubData(other.texture, GL_TEXTURE_2D, srcLevel, srcX >> srcLevel, srcY >> srcLevel, 0, texture, GL_TEXTURE_2D, dstLevel, dstX >> dstLevel, dstY >> dstLevel, 0, width >> srcLevel, height >> srcLevel, 1);
    }

    void Texture2D::GetData(unsigned int level, TextureFormat format, unsigned int size, void* out)
    {
        glGetTextureImage(texture, level, TextureFormatToGLFormatType(format), TextureFormatToGLType(format), size, out);
    }

    void Texture2D::GenerateMipmaps()
    {
        glGenerateTextureMipmap(texture);
    }

    void Texture2D::BindImage(unsigned int unit, unsigned int level, unsigned int access) const
    {
        glBindImageTexture(unit, texture, level, GL_FALSE, 0, access, TextureFormatToGLFormat(format));
    }

    void Texture2D::UnbindImage(unsigned int unit) const
    {
        glBindImageTexture(unit, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    }

    void Texture2D::Bind(unsigned int slot) const
    {
        glBindTextureUnit(slot, texture);
    }

    void Texture2D::Unbind(unsigned int slot) const
    {
        glBindTextureUnit(slot, 0);
    }

    TextureAtlas::TextureAtlas()
    {
    }

    TextureAtlas::TextureAtlas(TextureAtlas&& other) :
        texture(std::move(other.texture)), subwidth(other.subwidth), subheight(other.subheight), width(other.width), height(other.height), size(other.size)
    {
    }

    TextureAtlas::~TextureAtlas()
    {
    }

    TextureAtlas& TextureAtlas::operator=(TextureAtlas&& other)
    {
        texture = std::move(other.texture);
        subwidth = other.subwidth;
        subheight = other.subheight;
        width = other.width;
        height = other.height;
        size = other.size;
        return *this;
    }

    void TextureAtlas::SetTextures(unsigned int count, const Texture2D* textures)
    {
        glm::ivec2 dimensions = CalculateDimensions(count);

        width = dimensions.x; height = dimensions.y;
        subwidth = textures[0].GetWidth(); subheight = textures[0].GetHeight();
        size = count;

        Texture2D newTexture(textures[0].GetLevels(), textures[0].GetFormat(), subwidth * width, subheight * height);
        newTexture.SetStyle(TextureStyle::PIXELATED);

        texture = std::move(newTexture);
        for(unsigned int i = 0; i < count; i++)
        {
            unsigned int x = i / height;
            unsigned int y = i % height;

            for(unsigned int j = 0; j < texture->GetLevels(); j++)
                texture->CopyData(textures[i], j, 0, 0, j, x * subwidth, y * subheight, subwidth, subheight);
        }
    }

    glm::ivec2 TextureAtlas::CalculateDimensions(unsigned int count) const
    {
        glm::ivec2 dimensions;
        for(unsigned int i = 1; i * i <= count; i++)
            if(count % i == 0)
                dimensions = { i, count / i };
        return dimensions;
    }

    unsigned int Perlin2DGenerator::instance_count = 0;

    std::optional<ComputeShader> Perlin2DGenerator::shader = std::nullopt;
    unsigned int Perlin2DGenerator::gradients_index = 0;
    int Perlin2DGenerator::resolution_location = -1;
    int Perlin2DGenerator::gradients_dimensions_location = -1;

    Perlin2DGenerator::Perlin2DGenerator(unsigned int width, unsigned int height) :
        resolution{ 8, 8 }, gradient_buffer(GL_STATIC_DRAW), texture(1, TextureFormat::R32, width, height)
    {
        if(instance_count++ == 0)
        {
            std::string compute_src = Files::ReadFile(PATH("shaders/compute/perlin2D.glsl"));
            shader.emplace(compute_src.c_str());

            gradients_index = shader->GetShaderStorageBlockIndex("gradients");
            resolution_location = shader->GetUniformLocation("resolution");
            gradients_dimensions_location = shader->GetUniformLocation("gradients_dimensions");

            ERROR(LogTemp, "Gradients: {}", gradients_dimensions_location);
            ERROR(LogTemp, "Resolution: {}", resolution_location);
            ERROR(LogTemp, "Gradients index: {}", gradients_index);
        }
    }

    Perlin2DGenerator::Perlin2DGenerator(Perlin2DGenerator&& other) :
        resolution(other.resolution), gradient_buffer(std::move(other.gradient_buffer)), texture(std::move(other.texture))
    {
        instance_count++;
    }

    Perlin2DGenerator::~Perlin2DGenerator()
    {
        if(--instance_count == 0)
            shader.reset();
    }

    Perlin2DGenerator& Perlin2DGenerator::operator=(Perlin2DGenerator&& other)
    {
        resolution = other.resolution;
        gradient_buffer = std::move(other.gradient_buffer);
        texture = std::move(other.texture);
        return *this;
    }

    void Perlin2DGenerator::Generate(Texture2D& texture, const glm::uvec2& resolution)
    {
        unsigned int width = texture.GetWidth(), height = texture.GetHeight();
        unsigned int gradients_x = width / resolution.x + 1, gradients_y = height / resolution.y + 1;

        std::vector<Perlin2DGradient> gradients(gradients_x * gradients_y);
        for(int i = 0; i < gradients.size(); i++)
            gradients[i] = { .gradient = glm::circularRand(1.0f) };

        INFO(LogTemp, "Texture: {} x {}", width, height);
        INFO(LogTemp, "Gradients: {} x {}", gradients_x, gradients_y);
        INFO(LogTemp, "Resolution: {} x {}", resolution.x, resolution.y);

        gradient_buffer.SetData(gradients.size() * sizeof(Perlin2DGradient), gradients.data());

        shader->Bind();
        shader->BindShaderStorageBlock(gradients_index, 0);
        gradient_buffer.BindBase(GL_SHADER_STORAGE_BUFFER, 0);
        texture.BindImage(0, 0, GL_WRITE_ONLY);
        glUniform2i(resolution_location, resolution.x, resolution.y);
        glUniform2i(gradients_dimensions_location, gradients_x, gradients_y);

        glDispatchCompute((unsigned int) (width / 8), (unsigned int) (height / 8), 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        texture.UnbindImage(0);
        gradient_buffer.UnbindBase(GL_SHADER_STORAGE_BUFFER, 0);
        shader->UnbindShaderStorageBlock(gradients_index);
        shader->Unbind();
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

    static void GLFWFramebufferSizeCallback(GLFWwindow* window, int width, int height)
    {
        Application::Get()->DispatchEvent<FramebufferEvent>(0, 0, width, height);
    }

    Renderer::Renderer(GLFWwindow* _window) :
        window(_window)
    {
        stbi_set_flip_vertically_on_load(true);

        TRACE(Renderer, "[{}:constructor]", (unsigned long long) this);

        TRACE(Renderer, "[{}:constructor] load OpenGL", (unsigned long long) this);

        glfwMakeContextCurrent(window);
        if(!gladLoadGL(glfwGetProcAddress))
        {
            CRITICAL(Renderer, "[{}:constructor] failed to load OpenGL", (unsigned long long) this);
            return;
        }

        glfwSetFramebufferSizeCallback(window, GLFWFramebufferSizeCallback);

        glfwSwapInterval(0);

        TRACE(Renderer, "[{}:constructor] load OpenGL return", (unsigned long long) this);

        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(MessageCallback, nullptr);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        imgui_renderer.emplace(window);
        chunk_renderer.emplace();

        TRACE(Renderer, "[{}:constructor] return");
    }

    Renderer::~Renderer()
    {
        TRACE(Renderer, "[{}:destructor]", (unsigned long long) this);

        chunk_renderer.reset();
        imgui_renderer.reset();

        TRACE(Renderer, "[{}:destructor] return", (unsigned long long) this);
    }

    void Renderer::Begin()
    {
        glClearColor(0.1f, 0.2f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void Renderer::SetViewport(float x, float y, float width, float height)
    {
        glViewport(x, y, width, height);
    }

    void Renderer::SetVPMatrix(const glm::mat4& vp_matrix) noexcept
    {
        chunk_renderer->SetVPMatrix(vp_matrix);
    }

    void Renderer::End()
    {
        glfwSwapBuffers(window);
    }

    ChunkRenderer::ChunkRenderer()
    {
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

        g_ebo.emplace(GL_STATIC_DRAW);
        g_ebo->SetData(sizeof(unsigned int) * face_count * 6, indices.data());

        chunk_shaders[0] = std::move(LoadShader(PATH("shaders/chunk_packed_64x64x64_xy.vert.glsl"), PATH("shaders/chunk_packed_64x64x64.frag.glsl")));
        
        chunk_shaders[1] = std::move(LoadShader(PATH("shaders/chunk_packed_64x64x64_yz.vert.glsl"), PATH("shaders/chunk_packed_64x64x64.frag.glsl")));

        chunk_shaders[2] = std::move(LoadShader(PATH("shaders/chunk_packed_64x64x64_zx.vert.glsl"), PATH("shaders/chunk_packed_64x64x64.frag.glsl")));

        if(!chunk_shaders[0].has_value() || !chunk_shaders[1].has_value() || !chunk_shaders[2].has_value())
        {
            ERROR(ChunkRenderer, "[{}:constructor] Failed to load some shaders!", (unsigned long long) this);
            return;
        }

        std::string wireframe_vertex_src = Files::ReadFile(PATH("shaders/chunk_packed_wireframe.vert.glsl"));
        std::string wireframe_fragment_src = Files::ReadFile(PATH("shaders/chunk_packed_wireframe.frag.glsl"));
        wireframe_shader.emplace(std::move(VFShader(wireframe_vertex_src.c_str(), wireframe_fragment_src.c_str())));

        wireframe_shader->model_location = wireframe_shader->shader.GetUniformLocation("u_model");
        wireframe_shader->vp_location = wireframe_shader->shader.GetUniformLocation("u_vp");

        draw_mode = DrawMode::NORMAL;
        
        std::vector<Texture2D> textures;
        std::unordered_map<std::string, unsigned int> texture_IDs;

        auto get_texture = [&](const std::string& path)
        {
            if(path.empty())
                return 0u;

            if(texture_IDs.find(path) == texture_IDs.end())
            {
                std::optional<Texture2D> texture = LoadTexture(path.c_str());
                if(texture.has_value())
                {
                    texture_IDs[path] = textures.size();
                    textures.emplace_back(std::move(*texture));
                }
                else texture_IDs[path] = 0u;
            }
            
            return texture_IDs[path];
        };

        for(auto block_it = Game::BlockRegistry::Begin(); block_it != Game::BlockRegistry::End(); block_it++)
        {
            block_it->render_attachment = blocks.size();

            auto& block = blocks.emplace_back();
            block.north = get_texture(block_it->metadata.face_north);
            block.south = get_texture(block_it->metadata.face_south);
            block.east = get_texture(block_it->metadata.face_east);
            block.west = get_texture(block_it->metadata.face_west);
            block.up = get_texture(block_it->metadata.face_up);
            block.down = get_texture(block_it->metadata.face_down);
        }

        atlas.emplace();
        atlas->SetTextures(textures.size(), textures.data());
    
        textures.clear();
    }

    ChunkRenderer::~ChunkRenderer()
    {
        atlas.reset();

        wireframe_shader.reset();

        for(unsigned int i = 0; i < 3; i++)
            chunk_shaders[i].reset();

        g_ebo.reset();
    }

    std::optional<ChunkRenderer::ChunkShader> ChunkRenderer::LoadShader(const char* vertex_path, const char* fragment_path)
    {
        std::string vertex_src = Files::ReadFile(vertex_path);
        std::string fragment_src = Files::ReadFile(fragment_path);

        ChunkShader shader { .shader = VFShader(vertex_src.c_str(), fragment_src.c_str()) };
        
        shader.vp_location = shader.shader.GetUniformLocation("u_vp");
        if(shader.vp_location == -1)
        {
            ERROR(ChunkRenderer, "[{}:LoadShader] (vertex_path:{}) (fragment_path:{}) u_vp uniform doesn't exist", (unsigned long long) this, vertex_path, fragment_path);
            return std::nullopt;
        }

        shader.model_location = shader.shader.GetUniformLocation("u_model");
        if(shader.model_location == -1)
        {
            ERROR(ChunkRenderer, "[{}:LoadShader] (vertex_path:{}) (fragment_path:{}) u_model uniform doesn't exist", (unsigned long long) this, vertex_path, fragment_path);
            return std::nullopt;
        }

        shader.atlas_dimensions_location = shader.shader.GetUniformLocation("u_atlas_dimensions");
        if(shader.atlas_dimensions_location == -1)
        {
            ERROR(ChunkRenderer, "[{}:LoadShader] (vertex_path:{}) (fragment_path:{}) u_atlas_dimensions uniform doesn't exist", (unsigned long long) this, vertex_path, fragment_path);
            return std::nullopt;
        }

        shader.texture_size_location = shader.shader.GetUniformLocation("u_texture_size");
        if(shader.texture_size_location == -1)
        {
            ERROR(ChunkRenderer, "[{}:LoadShader] (vertex_path:{}) (fragment_path:{}) u_texture_size uniform doesn't exist", (unsigned long long) this, vertex_path, fragment_path);
            return std::nullopt;
        }

        shader.texture_location = shader.shader.GetUniformLocation("u_texture");
        if(shader.texture_location == -1)
        {
            ERROR(ChunkRenderer, "[{}:LoadShader] (vertex_path:{}) (fragment_path:{}) u_texture uniform doesn't exist", (unsigned long long) this, vertex_path, fragment_path);
            return std::nullopt;
        }

        return shader;
    }

    void ChunkRenderer::Begin()
    {
        atlas->Bind(0);
    }

    void ChunkRenderer::Begin(FaceOrientation orientation)
    {
        active_pipeline = (unsigned int) orientation;

        switch(draw_mode)
        {
        case DrawMode::NORMAL:
            {
                ChunkShader& shader = chunk_shaders[active_pipeline].value();
                
                shader.shader.Bind();
                glUniformMatrix4fv(shader.vp_location, 1, GL_FALSE, glm::value_ptr(vp_matrix));
                glUniform2i(shader.atlas_dimensions_location, atlas->GetTableSize().x, atlas->GetTableSize().y);
                glUniform2f(shader.texture_size_location, atlas->GetTextureSize().x, atlas->GetTextureSize().y);
                glUniform1i(shader.texture_location, 0);
                break;
            }
        case DrawMode::WIREFRAME:
            {
                ChunkShader& shader = wireframe_shader.value();

                shader.shader.Bind();
                glUniformMatrix4fv(shader.vp_location, 1, GL_FALSE, glm::value_ptr(vp_matrix));
                break;
            }
        }
    }

    void ChunkRenderer::Render(const ChunkMesh& mesh)
    {
        if(mesh.BufferSize(active_pipeline) != 0)
        {
            switch(draw_mode)
            {
            case DrawMode::NORMAL:
                {
                    ChunkShader& shader = chunk_shaders[active_pipeline].value();
                    glUniformMatrix4fv(shader.model_location, 1, GL_FALSE, glm::value_ptr(mesh.GetModelMatrix()));
                    break;
                }
            case DrawMode::WIREFRAME:
                {
                    ChunkShader& shader = wireframe_shader.value();
                    glUniformMatrix4fv(shader.model_location, 1, GL_FALSE, glm::value_ptr(mesh.GetModelMatrix()));
                    break;
                }
            }

            mesh.Bind(active_pipeline);
            glDrawElements(GL_TRIANGLES, mesh.BufferSize(active_pipeline) * 6, GL_UNSIGNED_INT, NULL);
            mesh.Unbind(active_pipeline);
        }
    }

    void ChunkRenderer::End()
    {
        switch(draw_mode)
        {
        case DrawMode::NORMAL:
            chunk_shaders[active_pipeline]->shader.Unbind();
            break;
        case DrawMode::WIREFRAME:
            wireframe_shader->shader.Unbind();
            break;
        }

        atlas->Unbind(0);
    }

    int ChunkRenderer::GetTextureID(const Game::Face& face)
    {
        auto block_cit = Game::BlockRegistry::CFind(face.block);
        unsigned int idx = block_cit->render_attachment;

        switch(face.direction)
        {
        case Game::Direction::NORTH: return blocks[idx].north;
        case Game::Direction::SOUTH: return blocks[idx].south;
        case Game::Direction::EAST: return blocks[idx].east;
        case Game::Direction::WEST: return blocks[idx].west;
        case Game::Direction::UP: return blocks[idx].up;
        case Game::Direction::DOWN: return blocks[idx].down;
        default: return 0;
        }
    }

    static TextureFormatType ChannelCountToTextureFormatType(int channels)
    {
        switch(channels)
        {
        case 1: return TextureFormatType::RED;
        case 3: return TextureFormatType::RGB;
        case 4: return TextureFormatType::RGBA;
        default: return TextureFormatType::RGBA;
        }
    }

    std::optional<Texture2D> ChunkRenderer::LoadTexture(const char* path)
    {
        int width, height, channels;
        unsigned char* data = stbi_load(path, &width, &height, &channels, 0);

        if(data == nullptr)
        {
            ERROR(ChunkRenderer, "[LoadTexture] (#path:{}) failed to load texture", path);
            return std::nullopt;
        }
        
        Texture2D texture(4, TextureFormat::RGB8, width, height);
        texture.SetData(data, 0, ChannelCountToTextureFormatType(channels));
        texture.GenerateMipmaps();

        stbi_image_free(data);
        return texture;
    }

    ImGuiRenderer::ImGuiRenderer(GLFWwindow* _window) :
        window(_window)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImPlot::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        font = io.Fonts->AddFontFromFileTTF(PATH("fonts/Roboto.ttf"), 24.0f);

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init();
    }

    ImGuiRenderer::~ImGuiRenderer()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImPlot::DestroyContext();
        ImGui::DestroyContext();
    }

    void ImGuiRenderer::Begin()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::PushFont(font);
    }
    
    void ImGuiRenderer::End()
    {
        ImGui::PopFont();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(window);
        }
    }

    namespace __detail
    {
        ChunkMeshPool::ChunkMeshPool()
        {
        }

        ChunkMeshPool::ChunkMeshPool(ChunkMeshPool&& other) :
            meshes(std::move(other.meshes))
        {
        }

        ChunkMeshPool::ChunkMeshPool(const ChunkMeshPool& other) :
            meshes(other.meshes)
        {
        }

        ChunkMeshPool::~ChunkMeshPool()
        {
        }

        ChunkMeshPool& ChunkMeshPool::operator=(ChunkMeshPool&& other)
        {
            meshes = std::move(other.meshes);
            return *this;
        }

        ChunkMeshPool& ChunkMeshPool::operator=(const ChunkMeshPool& other)
        {
            meshes = other.meshes;
            return *this;
        }

        ChunkMeshPool::Handle ChunkMeshPool::New()
        {
            static Handle handle = 0;
            meshes[handle] = std::make_shared<ChunkMesh>();
            meshes[handle]->SetHandle(handle);
            return handle++;
        }

        void ChunkMeshPool::Free(Handle handle)
        {
            meshes.erase(handle);
        }

        RenderContext::RenderContext()
        {
        }

        RenderContext::~RenderContext()
        {
        }

        ChunkMeshHandle RenderContext::NewChunkMesh()
        {
            return chunk_mesh_pool.New();
        }

        void RenderContext::FreeChunkMesh(ChunkMeshHandle handle)
        {
            chunk_mesh_pool.Free(handle);
        }

        void RenderContext::AddChunkToRenderQueue(ChunkMeshHandle handle)
        {
            std::shared_ptr<ChunkMesh> mesh = chunk_mesh_pool.At(handle);
            in_render_queue[handle] = render_queue.size();
            render_queue.emplace_back(handle, mesh);
        }
        
        void RenderContext::AddChunkToRenderQueue(const std::shared_ptr<ChunkMesh>& mesh)
        {
            ChunkMeshHandle handle = mesh->GetHandle();
            in_render_queue[handle] = render_queue.size();
            render_queue.emplace_back(handle, mesh);
        }

        void RenderContext::RemoveChunkFromRenderQueue(ChunkMeshHandle handle)
        {
            std::size_t idx = in_render_queue[handle];
            in_render_queue.erase(handle);
         
            InternalRemoveChunkFromRenderQueue(idx);
        }

        void RenderContext::RemoveChunkFromRenderQueue(const std::shared_ptr<ChunkMesh>& mesh)
        {
            ChunkMeshHandle handle = mesh->GetHandle();
            std::size_t idx = in_render_queue[handle];
            in_render_queue.erase(handle);

            InternalRemoveChunkFromRenderQueue(idx);
        }

        void RenderContext::InternalRemoveChunkFromRenderQueue(std::size_t idx)
        {
            if(idx != render_queue.size() - 1)
            {
                render_queue[idx] = render_queue.back();
                in_render_queue[render_queue[idx].first] = idx;
            }

            render_queue.pop_back();
        }

        void RenderContext::GeneratePerlin2DNoise(const glm::uvec2& resolution, const glm::uvec2& output_size, float* out)
        {
            Texture2D texture(1, TextureFormat::R32, output_size.x, output_size.y);
            perlin2D_generator->Generate(texture, resolution);
            texture.GetData(0, TextureFormat::R32, output_size.x * output_size.y * sizeof(float), out);
        }

        void RenderContext::GeneratePerlin2DNoise(const glm::uvec2& resolution, Texture2D& out)
        {
            perlin2D_generator->Generate(out, resolution);
        }
    };

    RenderThread::RenderThread(GLFWwindow* window)
    {
        exit = false;
        initialized = false;
        thread = std::move(std::thread([&]() { Run(window); }));
    }

    RenderThread::~RenderThread()
    {
        exit = true;
        thread.join();
    }

    void RenderThread::Handle(const FramebufferEvent& framebuffer_event)
    {
        SetViewport(framebuffer_event.x, framebuffer_event.y, framebuffer_event.width, framebuffer_event.height);
    }

    void RenderThread::Execute(Command&& command)
    {
        std::lock_guard<std::mutex> guard(command_queue_mutex);
        command_queue.emplace(command);
    }

    void RenderThread::SetViewport(float x, float y, float width, float height)
    {
        std::lock_guard<std::mutex> guard(viewport_mutex);
        vp_x = x; vp_y = y; vp_width = width; vp_height = height;
        viewport_changed = true;
    }

    void RenderThread::SetPlayer(const std::shared_ptr<Game::Player>& player)
    {
        std::lock_guard<std::mutex> guard(player_mutex);
        this->player = player;
    }

    int RenderThread::GetBlockTextureID(const Game::Face& face)
    {
        return renderer->GetChunkRenderer().GetTextureID(face);
    }

    void RenderThread::SetChunkDrawMode(ChunkRenderer::DrawMode draw_mode)
    {
        std::lock_guard<std::mutex> guard(chunk_render_mutex);
        renderer->GetChunkRenderer().SetDrawMode(draw_mode);
    }

    void RenderThread::Run(GLFWwindow* window)
    {
        renderer.emplace(window);
        render_context.perlin2D_generator.emplace(512, 512);

        InitViewport(window);
        InitGUIWindows();

        initialized = true;

        float last_time = glfwGetTime();
        int count = 0, chunks_rendered_total = 0;
        while(!exit)
        {
            PROFILE_THREAD(RenderThread);

            float render_start_time = glfwGetTime();

            ProcessCommands();

            if(viewport_changed)
                UpdateViewport();

            glm::mat4 vp_matrix = GetVPMatrix();
            Math::Frustum frustum(vp_matrix);

            renderer->Begin();
            renderer->SetVPMatrix(vp_matrix);

            chunks_rendered_total += RenderChunks(frustum);
            RenderGUI();

            renderer->End();

            float render_end_time = glfwGetTime();
            float render_time = render_end_time - render_start_time;

            float sleep_time = (render_time < 1.0f / 144.0f ? 1.0f / 144.0f - render_time : 0.0f);

            if(sleep_time != 0.0f)
                std::this_thread::sleep_for(std::chrono::microseconds((long long) (sleep_time * 1'000'000.0f)));

            count++;

            float time = glfwGetTime();
            //if(time - last_time >= 2.0f)
            {

                ::GUI::Render::RenderDebugInfoEvent event;
                event.fps = count / (time - last_time);
                event.chunks_rendered = chunks_rendered_total;

                Application::Get()->DispatchEvent(event);

                last_time = time;
                count = 0; 
                chunks_rendered_total = 0; 
            }
        }

        DestroyGUIWindows();

        renderer.reset();
    }

    void RenderThread::InitViewport(GLFWwindow* window)
    {
        std::lock_guard<std::mutex> guard(viewport_mutex);

        vp_x = vp_y = 0;
        glfwGetFramebufferSize(window, &vp_width, &vp_height);
        viewport_changed = false;
    }

    void RenderThread::InitGUIWindows()
    {
        ::GUI::WindowManager& window_manager = render_context.gui_window_manager.emplace();
        window_manager.emplace<::GUI::DemoWindow>()->Close();
        window_manager.emplace<::GUI::Plot::PlotDemoWindow>()->Close();
        window_manager.emplace<::GUI::Profiler::ProfilerWindow>()->Close();
        window_manager.emplace<::GUI::Render::RenderDemoWindow>(&render_context)->Close();
        window_manager.emplace<::GUI::Game::BlocksDemoWindow>()->Open();
        
        auto debug_info_overlay = window_manager.emplace<::GUI::Render::DebugInfoOverlayWindow>();
        debug_info_overlay->Open();
        ::Application::Get()->RegisterHandler(debug_info_overlay);
    }

    void RenderThread::ProcessCommands()
    {
        std::lock_guard<std::mutex> guard(command_queue_mutex);
        while(!command_queue.empty())
        {
            command_queue.front()(render_context);
            command_queue.pop();
        }
    }

    void RenderThread::UpdateViewport()
    {
        std::lock_guard<std::mutex> guard(viewport_mutex);

        viewport_changed = false;
        renderer->SetViewport(vp_x, vp_y, vp_width, vp_height);
    }

    glm::mat4 RenderThread::GetVPMatrix()
    {
        std::lock_guard<std::mutex> player_guard(player_mutex);
        if(player)
            return player->GetProjection() * glm::inverse(player->GetTransform().GetMatrix());
        else return glm::mat4(1.0f);
    }

    int RenderThread::RenderChunks(const Math::Frustum& frustum)
    {
        std::lock_guard<std::mutex> chunk_render_guard(chunk_render_mutex);

        ChunkRenderer& chunk_renderer = renderer->GetChunkRenderer();
        chunk_renderer.Begin();

        int total = 0;
        for(unsigned int i = 0; i < 3; i++)
        {
            chunk_renderer.Begin((FaceOrientation) i);
            for(std::size_t j = 0; j < render_context.render_queue.size(); )
                if(auto mesh = render_context.render_queue[j].second.lock())
                {
                    if(IsChunkOnScreen(mesh, frustum))
                    {
                        chunk_renderer.Render(*mesh);
                        total++;
                    }
                    j++;
                }
                else render_context.InternalRemoveChunkFromRenderQueue(j);
        }

        chunk_renderer.End();
        return total;
    }

    bool RenderThread::IsChunkOnScreen(const std::shared_ptr<ChunkMesh>& mesh, const Math::Frustum& frustum)
    {
        Math::AABB aabb = { mesh->GetModelMatrix() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
                            mesh->GetModelMatrix() * glm::vec4(CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE, 1.0f) };

        return frustum.IsAABBVisible(aabb);                    
    }

    void RenderThread::RenderGUI()
    {
        std::lock_guard<std::mutex> imgui_render_guard(imgui_render_mutex);

        ImGuiRenderer& imgui_renderer = renderer->GetImGuiRenderer();
        imgui_renderer.Begin();
        render_context.gui_window_manager->Draw();
        imgui_renderer.End();
    }

    void RenderThread::DestroyGUIWindows()
    {
        render_context.gui_window_manager.reset();
    }
};

namespace GUI
{
    namespace Render
    {
        RenderDemoWindow::RenderDemoWindow(::Render::RenderThread::RenderContext* render_context) :
            Window("Render Demo Window"), render_context(render_context)
        {}

        void RenderDemoWindow::Draw()
        {
            static ::Math::AABB example_aabb;
            static ::Render::Texture2D example_noise(1, ::Render::TextureFormat::R32, 512, 512);
            static glm::ivec2 example_noise_resolution = { 4, 4 };

            if(Begin())
            {
                InputAABB("AABB", example_aabb);

                if(ImGui::TreeNode("Perlin 2D"))
                {
                    SliderInt2("Resolution", (int*) &example_noise_resolution, 1, 512);
                    
                    if(ImGui::Button("Generate"))
                        render_context->GeneratePerlin2DNoise(example_noise_resolution, example_noise);

                    Text("Noise");
                    Image((void*) (intptr_t) example_noise.GetInternalTexture(), ImVec2(example_noise.GetWidth(), example_noise.GetHeight()));
                
                    ImGui::TreePop();
                }

                End();
            }
        }

        DebugInfoOverlayWindow::DebugInfoOverlayWindow() :
            Window("Debug Info Overlay")
        {
        }

        void DebugInfoOverlayWindow::Draw()
        {
            static const float PADDING = 10.0f;
            static const ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;

            const ImGuiViewport* viewport = GetMainViewport();
            ImVec2 work_position = viewport->WorkPos;

            ImVec2 window_position, window_position_pivot;
            window_position.x = work_position.x + PADDING;
            window_position.y = work_position.y + PADDING;
            window_position_pivot.x = 0;
            window_position_pivot.y = 0;
            SetNextWindowPos(window_position, ImGuiCond_Always, window_position_pivot);
            SetNextWindowViewport(viewport->ID);
            SetNextWindowBgAlpha(0.35f);
            if(Begin(window_flags))
            {
                if(TreeNode("Game Info"))
                {
                    Text("Game Ticks Per Second: %.0f", game_fps);
                    Text("Player Position: %.2f %.2f %.2f", player_position.x, player_position.y, player_position.z);
                    TreePop();
                }
                if(TreeNode("World Generation Info"))
                {
                    Text("World Generation Ticks Per Second: %.0f", world_generation_fps);
                    Text("Chunks Generated: %d", chunks_generated);
                    TreePop();
                }
                if(TreeNode("Render Info"))
                {
                    Text("Render Ticks Per Second: %.0f", render_fps);
                    Text("Chunks Rendered: %d", chunks_rendered);
                    TreePop();
                }
            }
            End();
        }

        void DebugInfoOverlayWindow::Handle(const GameDebugInfoEvent& event)
        {
            game_fps = event.fps;
            player_position = event.player_position;
        }

        void DebugInfoOverlayWindow::Handle(const WorldGenerationInfoEvent& event)
        {
            world_generation_fps = event.fps;
            chunks_generated = event.chunks_generated;
        }

        void DebugInfoOverlayWindow::Handle(const RenderDebugInfoEvent& event)
        {
            render_fps = event.fps;
            chunks_rendered = event.chunks_rendered;
        }
    };
};

