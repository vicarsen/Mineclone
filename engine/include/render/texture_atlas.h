#pragma once

#include "texture.h"

#include "../mathematics.h"

#include "../utils/optional.h"

namespace Render
{
    class TextureAtlas
    {
    public:
        TextureAtlas();
        TextureAtlas(TextureAtlas&& other);
        TextureAtlas(const TextureAtlas& other) = delete;
        
        ~TextureAtlas();

        TextureAtlas& operator=(TextureAtlas&& other);
        TextureAtlas& operator=(const TextureAtlas& other) = delete;

        void SetTextures(unsigned int count, const Texture2D* textures);

        inline void Bind(unsigned int slot) const { texture->Bind(slot); }
        inline void Unbind(unsigned int slot) const { texture->Unbind(slot); }

        inline ::Math::ivec2 GetTextureSize() const noexcept { return { subwidth, subheight }; }
        inline ::Math::ivec2 GetTableSize() const noexcept { return { width, height }; }
        inline ::Math::ivec2 GetAtlasSize() const noexcept { return { width * subwidth, height * subheight }; }

        inline Texture2D& GetTexture() { return texture.value(); }

    private:
        ::Math::ivec2 CalculateDimensions(unsigned int count) const;

    private:
        ::Utils::Optional<Texture2D> texture;
        unsigned int subwidth, subheight;
        unsigned int width, height, size;
    };
};

