#pragma once

#include "../gui.h"

#include "../blocks.h"

namespace GUI
{
    namespace Game
    {
        void DirectionCombo(const char* name, ::Game::Direction& direction);
        void BlocksCombo(const char* name, ::Game::BlockID& current_block);

        class BlocksDemoWindow : public Window
        {
        public:
            BlocksDemoWindow();
            virtual ~BlocksDemoWindow() = default;

            virtual void Draw() override final;
        };
    };
};

