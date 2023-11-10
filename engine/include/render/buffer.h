#pragma once

#include "../logger.h"

DECLARE_LOG_CATEGORY(Buffer);

namespace Render
{
    class Buffer
    {
    public:
        Buffer();
        Buffer(unsigned int usage);
        Buffer(Buffer&& other);
        Buffer(const Buffer& other);

        ~Buffer();

        Buffer& operator=(Buffer&& other);
        Buffer& operator=(const Buffer& other);

        void SetData(unsigned int bytes, const void* data);
        void Clear();

        void Bind(unsigned int target) const;
        void Unbind(unsigned int target) const;

        void BindBase(unsigned int target, unsigned int index) const;
        void UnbindBase(unsigned int target, unsigned int index) const;

        inline unsigned int GetUsage() const noexcept { return usage; }
        inline void SetUsage(unsigned int usage) noexcept { this->usage = usage; }

        inline unsigned int Get() const noexcept { return buffer; }
    
    private:
        unsigned int bytes, buffer, usage;
    };

    class VertexArray
    {
    public:
        VertexArray();
        VertexArray(VertexArray&& other);
        VertexArray(const VertexArray& other) = delete;

        ~VertexArray();

        VertexArray& operator=(VertexArray&& other);
        VertexArray& operator=(const VertexArray& other) = delete;

        unsigned int AddVertexBuffer(const Buffer& vbo, unsigned int offset, unsigned int stride);
        void SetElementBuffer(const Buffer& ebo);

        void AddVertexArrayAttrib(unsigned int buffer, int size, unsigned int type, unsigned int offset);

        void Clear();

        void Bind() const;
        void Unbind() const;

        inline unsigned int Get() const noexcept { return vao; }

    private:
        unsigned int vao, vertex_buffers, attribs;
    };
};

