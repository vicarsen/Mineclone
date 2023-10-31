#pragma once

#include "../logger.h"

#include "../frustum.h"

namespace fmt
{
    template<>
    struct formatter<::Math::Frustum>
    {
        template<typename FormatParseContext>
        constexpr auto parse(FormatParseContext& ctx) -> decltype(ctx.begin())
        {
            return ctx.end();
        }

        template<typename FormatContext>
        auto format(const ::Math::Frustum& frustum, FormatContext& ctx) -> decltype(ctx.out())
        {
            return ::fmt::format_to(ctx.out(), "{{left:{},right:{},bottom:{},top:{},near:{},far:{}}}", frustum.GetPlane(::Math::Frustum::Plane::Left), frustum.GetPlane(::Math::Frustum::Plane::Right), frustum.GetPlane(::Math::Frustum::Plane::Bottom), frustum.GetPlane(::Math::Frustum::Plane::Top), frustum.GetPlane(::Math::Frustum::Plane::Near), frustum.GetPlane(::Math::Frustum::Plane::Far));
        }
    };
};

