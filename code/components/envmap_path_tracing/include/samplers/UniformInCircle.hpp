#pragma once
#ifndef __ENVMAP_UNIFORM_IN_CIRCLE_HPP__
#define __ENVMAP_UNIFORM_IN_CIRCLE_HPP__

#include "Sampler2d.hpp"
#include <ctime>

namespace EnvMapPathTracer
{
    using namespace std;

    class UniformInCircle : public Sampler2d
    {
    private:
        default_random_engine e;
        uniform_real_distribution<float> u;
    public:
        UniformInCircle()
            : e((unsigned int)time(0) + insideSeed())
            , u(-1, 1)
        {}

        Vec2 sample2d() override {
            float x{0}, y{0};
            do {
                x = u(e);
                y = u(e);
            } while ((x*x + y*y) > 1);
            return {x, y};
        }
    };
}

#endif
