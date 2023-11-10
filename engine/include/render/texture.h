#pragma once

namespace Render
{
    enum class TextureStyle { PIXELATED, SMOOTH };

    enum class TextureFormatType { RED, RGB, RGBA };
    enum class TextureFormat { R8, R32, RGB8, RGB32, RGBA8, RGBA32 };

    class Texture2D
    {
    public:
        Texture2D(unsigned int levels, TextureFormat format, unsigned int width, unsigned int height);
        Texture2D(Texture2D&& other);
        Texture2D(const Texture2D& other) = delete;

        ~Texture2D();

        Texture2D& operator=(Texture2D&& other);
        Texture2D& operator=(const Texture2D& other) = delete;

        void SetStyle(TextureStyle style);

        void SetData(const unsigned char* data, unsigned int level, TextureFormatType format);
        void SetData(const unsigned char* data, unsigned int level, TextureFormatType format, unsigned int x, unsigned int y, unsigned int width, unsigned int height);
        
        void CopyData(const Texture2D& other, unsigned int src_level, unsigned int src_x, unsigned int src_y, unsigned int dst_level, unsigned int dst_x, unsigned int dst_y, unsigned int width, unsigned int height);
        void GetData(unsigned int level, TextureFormat format, unsigned int size, void* out);

        void GenerateMipmaps();

        void BindImage(unsigned int unit, unsigned int level, unsigned int access) const;
        void UnbindImage(unsigned int unit) const;

        void Bind(unsigned int slot) const;
        void Unbind(unsigned int slot) const;

        inline unsigned int GetLevels() const noexcept { return levels; }
        inline unsigned int GetWidth() const noexcept { return width; }
        inline unsigned int GetHeight() const noexcept { return height; }
        inline TextureFormat GetFormat() const noexcept { return format; }
        inline TextureStyle GetStyle() const noexcept { return style; }

        inline unsigned int GetInternalTexture() const noexcept { return texture; }

    private:
        unsigned int texture;
        TextureFormat format;
        unsigned int width, height, levels;
        TextureStyle style;
    };
};

