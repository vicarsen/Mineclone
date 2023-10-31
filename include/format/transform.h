#pragma once

#include "../logger.h"

#include "../transform.h"

namespace fmt
{
    template<>
    struct formatter<::Math::Transform>
    {
        template<typename FormatParseContext>
        constexpr auto parse(FormatParseContext& ctx) -> decltype(ctx.begin())
        {
            return ctx.end();
        }

        template<typename FormatContext>
        auto format(const ::Math::Transform& transform, FormatContext& ctx) -> decltype(ctx.out())
        {
            return ::fmt::format_to(ctx.out(), "{{position:{},rotation:{},scale:{}}}", transform.Position(), transform.Rotation(), transform.Scale());
        }
    };
};

