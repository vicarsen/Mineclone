#pragma once

#include "../gui.h"

#include "../mathematics.h"

namespace GUI
{
    namespace Math
    {
        inline bool SliderVec2(const char* label, ::Math::vec2* v, float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0)
        {
            return SliderFloat2(label, &v->x, v_min, v_max, format, flags);
        }

        inline bool SliderVec3(const char* label, ::Math::vec3* v, float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0)
        {
            return SliderFloat3(label, &v->x, v_min, v_max, format, flags);
        }

        inline bool SliderVec4(const char* label, ::Math::vec4* v, float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0)
        {
            return SliderFloat4(label, &v->x, v_min, v_max, format, flags);
        }

        inline bool SliderIVec2(const char* label, ::Math::ivec2* v, int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0)
        {
            return SliderInt2(label, &v->x, v_min, v_max, format, flags);
        }

        inline bool SliderIVec3(const char* label, ::Math::ivec3* v, int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0)
        {
            return SliderInt3(label, &v->x, v_min, v_max, format, flags);
        }

        inline bool SliderIVec4(const char* label, ::Math::ivec4* v, int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0)
        {
            return SliderInt4(label, &v->x, v_min, v_max, format, flags);
        }

        inline bool DragVec2(const char* label, ::Math::vec2* v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0)
        {
            return DragFloat2(label, &v->x, v_speed, v_min, v_max, format, flags);
        }

        inline bool DragVec3(const char* label, ::Math::vec3* v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0)
        {
            return DragFloat3(label, &v->x, v_speed, v_min, v_max, format, flags);
        }

        inline bool DragVec4(const char* label, ::Math::vec4* v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0)
        {
            return DragFloat4(label, &v->x, v_speed, v_min, v_max, format, flags);
        }

        inline bool DragIVec2(const char* label, ::Math::ivec2* v, float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", ImGuiSliderFlags flags = 0)
        {
            return DragInt2(label, &v->x, v_speed, v_min, v_max, format, flags);
        }

        inline bool DragIVec3(const char* label, ::Math::ivec3* v, float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", ImGuiSliderFlags flags = 0)
        {
            return DragInt3(label, &v->x, v_speed, v_min, v_max, format, flags);
        }

        inline bool DragIVec4(const char* label, ::Math::ivec4* v, float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", ImGuiSliderFlags flags = 0)
        {
            return DragInt4(label, &v->x, v_speed, v_min, v_max, format, flags);
        }

        inline bool InputVec2(const char* label, ::Math::vec2* v, const char* format = "%.3f", ImGuiInputTextFlags flags = 0)
        {
            return InputFloat2(label, &v->x, format, flags);
        }

        inline bool InputVec3(const char* label, ::Math::vec3* v, const char* format = "%.3f", ImGuiInputTextFlags flags = 0)
        {
            return InputFloat3(label, &v->x, format, flags);
        }

        inline bool InputVec4(const char* label, ::Math::vec4* v, const char* format = "%.3f", ImGuiInputTextFlags flags = 0)
        {
            return InputFloat4(label, &v->x, format, flags);
        }

        inline bool InputIVec2(const char* label, ::Math::ivec2* v, ImGuiInputTextFlags flags = 0)
        {
            return InputInt2(label, &v->x, flags);
        }

        inline bool InputIVec3(const char* label, ::Math::ivec3* v, ImGuiInputTextFlags flags = 0)
        {
            return InputInt3(label, &v->x, flags);
        }

        inline bool InputIVec4(const char* label, ::Math::ivec4* v, ImGuiInputTextFlags flags = 0)
        {
            return InputInt4(label, &v->x, flags);
        }

        inline bool ColorEdit3(const char* label, ::Math::vec3* col, ImGuiColorEditFlags flags = 0)
        {
            return ::ImGui::ColorEdit3(label, &col->x, flags);
        }

        inline bool ColorEdit4(const char* label, ::Math::vec4* col, ImGuiColorEditFlags flags = 0)
        {
            return ::ImGui::ColorEdit4(label, &col->x, flags);
        }

        inline bool ColorPicker3(const char* label, ::Math::vec3* col, ImGuiColorEditFlags flags = 0)
        {
            return ::ImGui::ColorPicker3(label, &col->x, flags);
        }

        inline bool ColorPicker4(const char* label, ::Math::vec4* col, ImGuiColorEditFlags flags = 0)
        {
            return ::ImGui::ColorPicker4(label, &col->x, flags);
        }
    };
};

