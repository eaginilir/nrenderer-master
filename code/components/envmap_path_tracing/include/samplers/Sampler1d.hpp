#pragma once
#ifndef __ENVMAP_SAMPLER_1D_HPP__
#define __ENVMAP_SAMPLER_1D_HPP__

#include "Sampler.hpp"
#include <random>

namespace EnvMapPathTracer
{
    class Sampler1d : protected Sampler
    {
    public:
        Sampler1d() = default;
        virtual float sample1d() = 0;
    };
}

#endif
