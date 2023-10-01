#include "render.h"

#include <glad/gl.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "glm.fmt.h"
#include "blocks.fmt.h"

#include <vector>
#include <cassert>

#include "files.h"
#include "window.h"
#include "stb_image.h"

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
    static unsigned int g_ebo, g_service_buffer;
    
    FaceBuffer::FaceBuffer()
    {
        TRACE(FaceBuffer, "[{}:constructor]", (unsigned long long) this);

        size = 0;
        glCreateBuffers(1, &buffer);

        TRACE(FaceBuffer, "[{}:constructor] (buffer:{}) glCreateBuffer", (unsigned long long) this, buffer);

        glCreateVertexArrays(1, &vao);

        TRACE(FaceBuffer, "[{}:constructor] (vao:{}) glCreateVertexArrays", (unsigned long long) this, vao);

        glVertexArrayVertexBuffer(vao, 0, buffer, 0, sizeof(Vertex));
        glVertexArrayElementBuffer(vao, g_ebo);

        glEnableVertexArrayAttrib(vao, 0);
        glVertexArrayAttribIFormat(vao, 0, 1, GL_INT, offsetof(Vertex, encoded));
        glVertexArrayAttribBinding(vao, 0, 0);

        TRACE(FaceBuffer, "[{}:constructor] return", (unsigned long long) this);
    }

    FaceBuffer::FaceBuffer(FaceBuffer&& other)
    {
        TRACE(FaceBuffer, "[{}:move_constructor] (#other:{})", (unsigned long long) this, (unsigned long long) &other);

        std::memcpy(this, &other, sizeof(FaceBuffer));
        other.vao = other.buffer = 0;
    
        TRACE(FaceBuffer, "[{}:move_constructor] (buffer:{}) (vao:{}) return", (unsigned long long) this, buffer, vao);
    }

    FaceBuffer::FaceBuffer(const FaceBuffer& other) :
        FaceBuffer()
    {
        glNamedBufferData(buffer, other.size * sizeof(FaceMesh), NULL, GL_STATIC_DRAW);
        glCopyNamedBufferSubData(other.buffer, buffer, 0, 0, other.size * sizeof(FaceMesh));

        size = other.size;
    }

    FaceBuffer::~FaceBuffer()
    {
        TRACE(FaceBuffer, "[{}:destructor] (buffer:{}) (vao:{})", (unsigned long long) this, buffer, vao);

        if(vao != 0) glDeleteVertexArrays(1, &vao);
        if(buffer != 0) glDeleteBuffers(1, &buffer);

        TRACE(FaceBuffer, "[{}:destructor] return", (unsigned long long) this);
    }

    FaceBuffer& FaceBuffer::operator=(FaceBuffer&& other)
    {
        TRACE(FaceBuffer, "[{}:move_assignment] (#other:{}) (buffer:{}) (vao:{})", (unsigned long long) this, (unsigned long long) &other, buffer, vao);

        if(vao != 0) glDeleteVertexArrays(1, &vao);
        if(buffer != 0) glDeleteBuffers(1, &buffer);

        std::memcpy(this, &other, sizeof(FaceBuffer));
        other.vao = other.buffer = 0;

        TRACE(FaceBuffer, "[{}:move_assignment] return", (unsigned long long) this);
        return *this;
    }

    FaceBuffer& FaceBuffer::operator=(const FaceBuffer& other)
    {
        glNamedBufferData(buffer, other.size * sizeof(FaceMesh), NULL, GL_STATIC_DRAW);
        glCopyNamedBufferSubData(other.buffer, buffer, 0, 0, other.size * sizeof(FaceMesh));

        size = other.size;
        return *this;
    }

    void FaceBuffer::SetData(unsigned int count, const FaceMesh* meshes)
    {
        glNamedBufferData(buffer, count * sizeof(FaceMesh), meshes, GL_STATIC_DRAW);
        size = count;
    }

    void FaceBuffer::Bind() const
    {
        glBindVertexArray(vao);
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

    void ChunkMesh::Bind(unsigned int buffer) const
    {
        buffers[buffer].Bind();
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

    VFShader::VFShader(const char* vertex_src_file, const char* fragment_src_file)
    {
        TRACE(Shaders, "[{}:constructor] (#vertex_src:{}) (#fragment_src:{})", (unsigned long long) this, vertex_src_file, fragment_src_file);

        std::string vertex_src = Files::ReadFile(vertex_src_file);
        std::string fragment_src = Files::ReadFile(fragment_src_file);

        unsigned int vertex_shader = CreateShader(vertex_src, GL_VERTEX_SHADER);
        unsigned int fragment_shader = CreateShader(fragment_src, GL_FRAGMENT_SHADER);

        if(vertex_shader == 0)
        {
            ERROR(Shaders, "[{}:constructor] vertex shader create error", (unsigned long long) this);
            return;
        }

        if(fragment_shader == 0)
        {
            ERROR(Shaders, "[{}:constructor] fragment shader create error", (unsigned long long) this);
            return;
        }

        program = glCreateProgram();

        TRACE(Shaders, "[{}:constructor] (program:{})", (unsigned long long) this, program);

        glAttachShader(program, vertex_shader);
        glAttachShader(program, fragment_shader);

        TRACE(Shaders, "[{}:constructor] link", (unsigned long long) this);

        glLinkProgram(program);

        TRACE(Shaders, "[{}:constructor] link return", (unsigned long long) this);

        glDetachShader(program, vertex_shader);
        glDetachShader(program, fragment_shader);

        TRACE(Shaders, "[{}:constructor] (vertex_shader:{}) (fragment_shader:{}) vertex, fragment deleted");

        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);

        int status;
        glGetProgramiv(program, GL_LINK_STATUS, &status);
        if(status != GL_TRUE)
        {
            int log_length;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);

            std::vector<char> message(log_length + 1, 0);
            glGetProgramInfoLog(program, log_length, &log_length, message.data());

            ERROR(Shaders, "[{}:constructor] (message:{}) link error", (unsigned long long) this, message.data());

            glDeleteProgram(program);
            return;
        }

        TRACE(Shaders, "[{}:constructor] return", (unsigned long long) this);
    }

    VFShader::VFShader(VFShader&& other)
    {
        program = other.program;
        other.program = 0;
    }

    VFShader::~VFShader()
    {
        TRACE(Shaders, "[{}:destructor] (program:{}) <>", (unsigned long long) this, program);

        glDeleteProgram(program);
    }

    VFShader& VFShader::operator=(VFShader&& other)
    {
        if(program) glDeleteProgram(program);

        program = other.program;
        other.program = 0;
        
        return *this;
    }

    void VFShader::Bind() const
    {
        glUseProgram(program);
    }

    int VFShader::GetUniformLocation(const char* name)
    {
        int location = glGetUniformLocation(program, name);

        TRACE(Shaders, "[{}:GetUniformLocation] (#name:{}) (location: {}) <>", (unsigned long long) this, name, location);
        return location;
    }

    Texture2D::Texture2D(unsigned int _levels, unsigned int _channels, unsigned int _width, unsigned int _height)
    {
        levels = _levels; channels = _channels; width = _width; height = _height;
        format = (channels == 3 ? GL_RGB8 : GL_RGBA8);

        glCreateTextures(GL_TEXTURE_2D, 1, &texture);

        glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTextureStorage2D(texture, levels, format, width, height);
    }

    Texture2D::Texture2D(Texture2D&& other) :
        texture(other.texture), width(other.width), height(other.height), channels(other.channels), levels(other.levels), format(other.format), style(other.style)
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
        channels = other.channels; levels = other.levels; style = other.style;

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

        Texture2D newTexture(textures[0].GetLevels(), textures[0].GetChannelCount(), subwidth * width, subheight * height);
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

    void TextureAtlas::Bind(unsigned int slot) const
    {
        texture->Bind(slot);
    }

    glm::ivec2 TextureAtlas::CalculateDimensions(unsigned int count) const
    {
        glm::ivec2 dimensions;
        for(unsigned int i = 1; i * i <= count; i++)
            if(count % i == 0)
                dimensions = { i, count / i };
        return dimensions;
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

        glCreateBuffers(1, &g_service_buffer);
        glNamedBufferStorage(g_service_buffer, 1024 * 1024 * 128, NULL, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);

        TRACE(Renderer, "[{}:constructor] (g_service_buffer:{})", (unsigned long long) this, g_service_buffer);

        chunk_renderer.emplace();

        TRACE(Renderer, "[{}:constructor] return");
    }

    Renderer::~Renderer()
    {
        TRACE(Renderer, "[{}:destructor]", (unsigned long long) this);

        chunk_renderer.reset();

        glDeleteBuffers(1, &g_service_buffer);

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

        glCreateBuffers(1, &g_ebo);
        glNamedBufferStorage(g_ebo, sizeof(unsigned int) * face_count * 6, indices.data(), GL_DYNAMIC_STORAGE_BIT);

        chunk_shaders[0] = std::move(LoadShader(PATH("shaders/chunk_packed_64x64x64_xy.vert.glsl"), PATH("shaders/chunk_packed_64x64x64.frag.glsl")));
        
        chunk_shaders[1] = std::move(LoadShader(PATH("shaders/chunk_packed_64x64x64_yz.vert.glsl"), PATH("shaders/chunk_packed_64x64x64.frag.glsl")));

        chunk_shaders[2] = std::move(LoadShader(PATH("shaders/chunk_packed_64x64x64_zx.vert.glsl"), PATH("shaders/chunk_packed_64x64x64.frag.glsl")));

        if(!chunk_shaders[0].has_value() || !chunk_shaders[1].has_value() || !chunk_shaders[2].has_value())
        {
            ERROR(ChunkRenderer, "[{}:constructor] Failed to load some shaders!", (unsigned long long) this);
            return;
        }

        wireframe_shader.emplace(std::move(VFShader(PATH("shaders/chunk_packed_wireframe.vert.glsl"), PATH("shaders/chunk_packed_wireframe.frag.glsl"))));

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

        glDeleteBuffers(1, &g_ebo);
    }

    std::optional<ChunkRenderer::ChunkShader> ChunkRenderer::LoadShader(const char* vertex_path, const char* fragment_path)
    {
        ChunkShader shader { .shader = VFShader(vertex_path, fragment_path) };
        
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
    }

    void ChunkRenderer::End()
    {
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

    std::optional<Texture2D> ChunkRenderer::LoadTexture(const char* path)
    {
        int width, height, channels;
        unsigned char* data = stbi_load(path, &width, &height, &channels, 0);

        if(data == nullptr)
        {
            ERROR(ChunkRenderer, "[LoadTexture] (#path:{}) failed to load texture", path);
            return std::nullopt;
        }
        
        Texture2D texture(4, 3, width, height);
        texture.SetData(data, 0, channels);
        texture.GenerateMipmaps();

        stbi_image_free(data);
        return texture;
    }

    namespace __detail
    {
        ChunkMeshPool::ChunkMeshPool()
        {
        }

        ChunkMeshPool::ChunkMeshPool(ChunkMeshPool&& other) :
            meshes(std::move(other.meshes)), sparse(std::move(other.sparse)), available(std::move(other.available)), alloc_requests(std::move(other.alloc_requests)), set_faces_requests(std::move(other.set_faces_requests))
        {
        }

        ChunkMeshPool::ChunkMeshPool(const ChunkMeshPool& other) :
            meshes(other.meshes), sparse(other.sparse), available(other.available), alloc_requests(other.alloc_requests), set_faces_requests(other.set_faces_requests)
        {
        }

        ChunkMeshPool::~ChunkMeshPool()
        {
        }

        ChunkMeshPool& ChunkMeshPool::operator=(ChunkMeshPool&& other)
        {
            meshes = std::move(other.meshes);
            sparse = std::move(other.sparse);
            available = std::move(other.available);
            alloc_requests = std::move(other.alloc_requests);
            set_faces_requests = std::move(other.set_faces_requests);
            return *this;
        }

        ChunkMeshPool& ChunkMeshPool::operator=(const ChunkMeshPool& other)
        {
            meshes = other.meshes;
            sparse = other.sparse;
            available = other.available;
            alloc_requests = other.alloc_requests;
            set_faces_requests = other.set_faces_requests;
            return *this;
        }

        ChunkMeshPool::Handle ChunkMeshPool::New()
        {
            std::atomic<bool> fence = false;
            Handle handle;

            {
                std::lock_guard<std::mutex> guard(mutex);

                if(!available.empty())
                {
                    handle = available.front(); available.pop();
                    meshes[handle].SetHandle(handle);
                    
                    if(sparse.size() <= handle)
                        sparse.resize(handle + 1, -1);

                    sparse[handle] = handle;
                    fence = true;
                }
                else alloc_requests.emplace(&fence, &handle);
            }

            while(fence == false);
            return handle;
        }

        void ChunkMeshPool::Free(Handle handle)
        {
            std::lock_guard<std::mutex> guard(mutex);
            available.emplace(sparse[handle]);
            meshes[sparse[handle]].SetHandle(-1);
            sparse[handle] = -1;
        }

        void ChunkMeshPool::SetFaces(Handle handle, std::vector<FaceMesh>& faces, FaceOrientation orientation)
        {
            std::lock_guard<std::mutex> guard(mutex);
            set_faces_requests.emplace(handle, std::move(faces), orientation);
        }

        void ChunkMeshPool::SetTransform(Handle handle, const Game::Transform& transform)
        {
            std::lock_guard<std::mutex> guard(mutex);
            meshes[sparse[handle]].SetModelMatrix(transform.GetMatrix());
        }

        void ChunkMeshPool::HandleRequests()
        {
            std::lock_guard<std::mutex> guard(mutex);

            while(!alloc_requests.empty())
            {
                auto[fence, handle] = alloc_requests.front(); alloc_requests.pop();
                
                if(available.empty())
                {
                    *handle = meshes.size();
                    meshes.emplace_back().SetHandle(*handle);
                }
                else
                {
                    *handle = available.front(); available.pop();
                    meshes[*handle].SetHandle(*handle);
                }

                if(sparse.size() <= *handle)
                    sparse.resize(*handle + 1, -1);

                sparse[*handle] = *handle;
                *fence = true;
            }

            while(!set_faces_requests.empty())
            {
                auto& handle = set_faces_requests.front().handle;
                auto& faces = set_faces_requests.front().faces;
                auto& orientation = set_faces_requests.front().orientation;

                meshes[sparse[handle]].SetFaces(faces.size(), faces.data(), orientation);
            
                set_faces_requests.pop();
            }
        }

        void ChunkMeshPool::GarbageCollect()
        {
            std::lock_guard<std::mutex> guard(mutex);

            // Very basic (and inefficient) algorithm, but should be fine for our application.
            // TODO Could be improved in the future.
            while(available.size() >= CHUNK_SIZE * CHUNK_SIZE * 3)
            {
                int idx = available.front(); available.pop();

                if(idx != meshes.size() - 1)
                {
                    meshes[idx] = std::move(meshes.back());
                    int handle = meshes[idx].GetHandle();
                    if(handle != -1)
                        sparse[handle] = idx;
                }

                meshes.pop_back();
            }

            while(available.size() < CHUNK_SIZE * CHUNK_SIZE)
            {
                available.emplace(meshes.size());
                meshes.emplace_back().SetHandle(-1);
            }
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

    RenderThread::ChunkMeshHandle RenderThread::NewChunkMesh()
    {
        return chunk_mesh_pool.New();
    }

    void RenderThread::FreeChunkMesh(ChunkMeshHandle handle)
    {
        chunk_mesh_pool.Free(handle);
    }

    void RenderThread::SetChunkFaces(ChunkMeshHandle handle, std::vector<FaceMesh>& faces, FaceOrientation orientation)
    {
        chunk_mesh_pool.SetFaces(handle, faces, orientation);
    }

    void RenderThread::SetChunkTransform(ChunkMeshHandle handle, const Game::Transform& transform)
    {
        chunk_mesh_pool.SetTransform(handle, transform);
    }

    int RenderThread::GetBlockTextureID(const Game::Face& face)
    {
        return renderer->GetChunkRenderer().GetTextureID(face);
    }

    void RenderThread::AddChunkToDrawCall(ChunkMeshHandle handle)
    {
        std::lock_guard<std::mutex> guard(chunks_draw_set_mutex);
        chunks_draw_set.emplace(handle);
    }

    void RenderThread::RemoveChunkFromDrawCall(ChunkMeshHandle handle)
    {
        std::lock_guard<std::mutex> guard(chunks_draw_set_mutex);
        chunks_draw_set.erase(handle);
    }

    void RenderThread::SetChunkDrawMode(ChunkRenderer::DrawMode draw_mode)
    {
        std::lock_guard<std::mutex> guard(chunk_render_mutex);
        renderer->GetChunkRenderer().SetDrawMode(draw_mode);
    }

    void RenderThread::Run(GLFWwindow* window)
    {
        renderer.emplace(window);
        ChunkRenderer& chunk_renderer = renderer->GetChunkRenderer();

        {
            std::lock_guard<std::mutex> guard(viewport_mutex);

            viewport_changed = false;
            glfwGetFramebufferSize(window, &vp_width, &vp_height);
            vp_x = vp_y = 0;
        }

        initialized = true;

        float last_time = glfwGetTime();
        int count = 0;
        while(!exit)
        {
            float render_start_time = glfwGetTime();

            chunk_mesh_pool.HandleRequests();
            chunk_mesh_pool.GarbageCollect();

            {
                std::lock_guard<std::mutex> guard(viewport_mutex);

                if(viewport_changed)
                {
                    viewport_changed = false;
                    renderer->SetViewport(vp_x, vp_y, vp_width, vp_height);

                    std::lock_guard<std::mutex> player_guard(player_mutex);
                    if(player)
                        player->SetAspectRatio(vp_width * 1.0f / vp_height);
                }
            }

            static glm::mat4 vp_matrix = glm::mat4(1.0f);
            
            {
                std::lock_guard<std::mutex> player_guard(player_mutex);
                if(player)
                    vp_matrix = player->GetProjection() * glm::inverse(player->GetTransform().GetMatrix());
            }

            renderer->Begin();
                
            renderer->SetVPMatrix(vp_matrix);

            {
                std::lock_guard<std::mutex> chunk_render_guard(chunk_render_mutex);
                chunk_renderer.Begin();

                std::unordered_set<ChunkMeshHandle> chunks_draw_set_copy;

                {
                    std::lock_guard<std::mutex> chunks_draw_set_guard(chunks_draw_set_mutex);
                    chunks_draw_set_copy = chunks_draw_set;
                }

                for(unsigned int i = 0; i < 3; i++)
                {
                    chunk_renderer.Begin((FaceOrientation) i);
                    for(ChunkMeshHandle handle : chunks_draw_set_copy)
                    {
                        std::lock_guard<std::mutex> chunk_mesh_pool_guard(chunk_mesh_pool.GetMutex());
                        if(chunk_mesh_pool.Has(handle))
                            chunk_renderer.Render(chunk_mesh_pool.at(handle));
                    }
                }
 
                chunk_renderer.End();
            }

            renderer->End();

            float render_end_time = glfwGetTime();
            float render_time = render_end_time - render_start_time;

            float sleep_time = (render_time < 1.0f / 144.0f ? 1.0f / 144.0f - render_time : 0.0f);

            if(sleep_time != 0.0f)
                std::this_thread::sleep_for(std::chrono::microseconds((long long) (sleep_time * 1'000'000.0f)));

            count++;

            float time = glfwGetTime();
            if(time - last_time >= 5.0f)
            {
                WARN(LogTemp, "Render avg: {} fps", count / (time - last_time));
                last_time = time;
                count = 0;
            }
        }

        renderer.reset();
    }
};

