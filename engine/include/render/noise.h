#pragma once

#include "texture.h"
#include "shader.h"
#include "buffer.h"

#include "../mathematics.h"

#include "../utils/optional.h"

namespace Render
{
    namespace Noise
    {
        struct Perlin2DGradient
        {
            ::Math::vec2 gradient;
        };

        class Perlin2DGenerator
        {
        public:
            Perlin2DGenerator(unsigned int width, unsigned int height);
            Perlin2DGenerator(Perlin2DGenerator&& other);
            Perlin2DGenerator(const Perlin2DGenerator& other) = delete;

            ~Perlin2DGenerator();

            Perlin2DGenerator& operator=(Perlin2DGenerator&& other);
            Perlin2DGenerator& operator=(const Perlin2DGenerator& other) = delete;

            void Generate(Texture2D& texture, const ::Math::uvec2& resolution);
            
            inline void Generate() { Generate(texture, resolution); }

            inline void SetResolution(const ::Math::uvec2& resolution) noexcept { this->resolution = resolution; }

            inline const ::Math::uvec2& GetResolution() const noexcept { return resolution; }
            inline unsigned int GetWidth() const noexcept { return texture.GetWidth(); }
            inline unsigned int GetHeight() const noexcept { return texture.GetHeight(); }

            inline Texture2D& GetTexture() noexcept { return texture; }

        private:
            ::Math::uvec2 resolution;
            
            Buffer gradient_buffer;
            Texture2D texture;

            static unsigned int instance_count;

            static ::Utils::Optional<ComputeShader> shader;
            static unsigned int gradients_index;
            static int resolution_location, gradients_dimensions_location;
        };

        namespace __detail
        {
            class RenderContext
            {
            public:
                RenderContext();
                RenderContext(RenderContext&& other) = delete;
                RenderContext(const RenderContext& other) = delete;

                ~RenderContext();

                RenderContext& operator=(RenderContext&& other) = delete;
                RenderContext& operator=(const RenderContext& other) = delete;

            private:
            };
        };

        class Renderer
        {
        public:
            typedef __detail::RenderContext RenderContext;

            Renderer();
            Renderer(Renderer&& other) = delete;
            Renderer(const Renderer& other) = delete;

            ~Renderer();

            Renderer& operator=(Renderer&& other) = delete;
            Renderer& operator=(const Renderer& other) = delete;

            void Begin();
            void End();

        private:
            Perlin2DGenerator perlin2D_generator;
            RenderContext render_context;
        };
    };

    
};

