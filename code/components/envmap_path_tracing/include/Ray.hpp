#pragma once
#ifndef __ENVMAP_RAY_HPP__
#define __ENVMAP_RAY_HPP__

#include "geometry/vec.hpp"
#include <limits>

#define FLOAT_INF numeric_limits<float>::infinity()

namespace EnvMapPathTracer
{
    using namespace NRenderer;
    using namespace std;

    struct Ray
    {
        Vec3 origin;
        Vec3 direction;

        inline Vec3 at(float t) const {
            return origin + t * direction;
        }

        Ray(const Vec3& origin, const Vec3& direction)
            : origin(origin), direction(direction) {}
        Ray() : origin{}, direction{} {}
    };
}

#endif
