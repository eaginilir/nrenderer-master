#pragma once
#ifndef __ENVMAP_AABB_HPP__
#define __ENVMAP_AABB_HPP__

#include "geometry/vec.hpp"
#include "Ray.hpp"
#include <algorithm>

namespace EnvMapPathTracer
{
    using namespace NRenderer;

    struct AABB
    {
        Vec3 min{FLOAT_INF, FLOAT_INF, FLOAT_INF};
        Vec3 max{-FLOAT_INF, -FLOAT_INF, -FLOAT_INF};

        AABB() = default;
        AABB(const Vec3& min, const Vec3& max) : min(min), max(max) {}

        bool hit(const Ray& r, float tMin, float tMax) const {
            for (int i = 0; i < 3; i++) {
                float invD = 1.0f / r.direction[i];
                float t0 = (min[i] - r.origin[i]) * invD;
                float t1 = (max[i] - r.origin[i]) * invD;
                if (invD < 0.0f) std::swap(t0, t1);
                tMin = t0 > tMin ? t0 : tMin;
                tMax = t1 < tMax ? t1 : tMax;
                if (tMax <= tMin) return false;
            }
            return true;
        }

        void expand(const Vec3& p) {
            min = glm::min(min, p);
            max = glm::max(max, p);
        }

        void expand(const AABB& box) {
            min = glm::min(min, box.min);
            max = glm::max(max, box.max);
        }

        Vec3 centroid() const {
            return (min + max) * 0.5f;
        }

        int longestAxis() const {
            Vec3 d = max - min;
            if (d.x > d.y && d.x > d.z) return 0;
            if (d.y > d.z) return 1;
            return 2;
        }
    };
}

#endif
