#pragma once

#include "../logger.h"

DECLARE_LOG_CATEGORY(Shader);

namespace Render
{
    class Shader
    {
    public:
        Shader(unsigned int type);
        Shader(Shader&& other);
        Shader(const Shader& other) = delete;

        ~Shader();

        Shader& operator=(Shader&& other);
        Shader& operator=(const Shader& other) = delete;

        void SetSource(const char* src);
        bool Compile();

        inline unsigned int Get() const noexcept { return shader; }

    private:
        unsigned int shader;
    };

    class Program
    {
    public:
        Program();
        Program(Program&& other);
        Program(const Program& other) = delete;

        ~Program();

        Program& operator=(Program&& other);
        Program& operator=(const Program& other) = delete;

        void Attach(const Shader& shader);
        void Detach(const Shader& shader);

        bool Link();

        int GetUniformLocation(const char* name) const;
        unsigned int GetUniformBlockIndex(const char* name) const;
        unsigned int GetShaderStorageBlockIndex(const char* name) const;

        void BindUniformBlock(unsigned int target, unsigned int binding) const;
        void UnbindUniformBlock(unsigned int target) const;

        void BindShaderStorageBlock(unsigned int target, unsigned int binding) const;
        void UnbindShaderStorageBlock(unsigned int target) const;

        void Bind() const;
        void Unbind() const;

        inline unsigned int Get() const noexcept { return program; }

    private:
        unsigned int program;
    };

    class VFShader : public Program
    {
    public:
        VFShader(const char* vertex_src, const char* fragment_src);
        VFShader(VFShader&& other) = default;
        VFShader(const VFShader& other) = delete;

        ~VFShader() = default;

        VFShader& operator=(VFShader&& other) = default;
        VFShader& operator=(const VFShader& other) = delete;
    };

    class ComputeShader : public Program
    {
    public:
        ComputeShader(const char* src);
        ComputeShader(ComputeShader&& other) = default;
        ComputeShader(const ComputeShader& other) = delete;

        ~ComputeShader() = default;

        ComputeShader& operator=(ComputeShader&& other) = default;
        ComputeShader& operator=(const ComputeShader& other) = delete;
    };
};

