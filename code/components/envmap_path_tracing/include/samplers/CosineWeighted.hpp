#pragma once
#ifndef __ENVMAP_COSINE_WEIGHTED_HPP__
#define __ENVMAP_COSINE_WEIGHTED_HPP__

#include "Sampler3d.hpp"
#include <ctime>
#include <cmath>

namespace EnvMapPathTracer
{
    using namespace std;

    // Cosine-weighted 半球采样
    // PDF = cos(theta) / PI
    class CosineWeighted : public Sampler3d
    {
    private:
        constexpr static float C_PI = 3.14159265358979323846264338327950288f;
        default_random_engine e;
        uniform_real_distribution<float> u;
    public:
        CosineWeighted()
            : e((unsigned int)time(0) + insideSeed())
            , u(0, 1)
        {}

        Vec3 sample3d() override {
            float u1 = u(e);
            float u2 = u(e);
            // Malley's method: 在单位圆上均匀采样，然后投影到半球
            float r = sqrt(u1);
            float theta = 2 * C_PI * u2;
            float x = r * cos(theta);
            float y = r * sin(theta);
            float z = sqrt(1.0f - u1);  // cos(theta) = sqrt(1 - r^2)
            return {x, y, z};
        }

        // PDF = cos(theta) / PI
        static float pdf(float cosTheta) {
            return cosTheta / C_PI;
        }
    };
}

#endif
