#pragma once

#include "../logger.h"

#include "../aabb.h"

namespace fmt
{
    template<>
    struct formatter<::Math::AABB>
    {
        template<typename FormatParseContext>
        constexpr auto parse(FormatParseContext& ctx) -> decltype(ctx.begin())
        {
            return ctx.end();
        }

        template<typename FormatContext>
        auto format(const ::Math::AABB& aabb, FormatContext& ctx) -> decltype(ctx.out())
        {
            return ::fmt::format_to(ctx.out(), "{{min:{},max:{}}}", aabb.min, aabb.max);
        }
    };
};

