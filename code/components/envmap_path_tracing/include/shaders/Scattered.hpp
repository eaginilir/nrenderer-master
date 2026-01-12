#pragma once
#ifndef __ENVMAP_SCATTERED_HPP__
#define __ENVMAP_SCATTERED_HPP__

#include "Ray.hpp"

namespace EnvMapPathTracer
{
    struct Scattered
    {
        Ray ray = {};
        Vec3 attenuation = {};
        Vec3 emitted = {};
        float pdf = {0.f};
    };
}

#endif
