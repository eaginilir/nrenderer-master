#pragma once
#ifndef __RTMATERIAL_HPP__
#define __RTMATERIAL_HPP__

#include <memory>
#include <vector>
#include <cmath>
#include "intersections/intersections.hpp"

#include "geometry/vec.hpp"
#include "common/macros.hpp"
#include "scene/Scene.hpp"

namespace RayTracing {
using namespace NRenderer;
using namespace std;

// 随机工具函数保留在基础头，供各材质实现使用
inline float randomFloat() {
    return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}
inline float randomFloatSigned() { return 2.f * randomFloat() - 1.f; }
inline Vec3 randomInUnitSphere() {
    while (true) {
        Vec3 p{ randomFloatSigned(), randomFloatSigned(), randomFloatSigned() };
        if (glm::dot(p, p) < 1.f) return p;
    }
}
inline Vec3 randomUnitVector() { return glm::normalize(randomInUnitSphere()); }
inline Vec3 randomInHemisphere(const Vec3& n) {
    Vec3 p = randomInUnitSphere();
    return glm::dot(p, n) < 0.f ? -p : p;
}
inline float schlick(float cosine, float refIdx) {
    float r0 = (1 - refIdx) / (1 + refIdx);
    r0 = r0 * r0;
    return r0 + (1 - r0) * std::pow(1 - cosine, 5);
}

// 基类：仅声明散射接口
class RTMaterial {
public:
    virtual ~RTMaterial() = default;
    virtual bool scatter(const Ray& in, const HitRecord& rec,
                         Vec3& attenuation, Ray& scattered) const = 0;
    // 折射接口
    // virtual bool refract(const Ray& in, const HitRecord& rec,
    //                      Vec3& attenuation, Ray& refracted) const {
    //     (void)in; (void)rec; (void)attenuation; (void)refracted;
    //     return false;
    // }
};

using SRTMat = std::shared_ptr<RTMaterial>;

} // namespace RayTracing
#endif