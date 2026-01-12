#pragma once
#ifndef __METAL_HPP__
#define __METAL_HPP__

#include "RTMaterial.hpp"

namespace RayTracing {

class Metal final : public RTMaterial {
public:
    Metal(const Vec3& albedo, float fuzz) : albedo_(albedo), fuzz_(glm::clamp(fuzz, 0.f, 1.f)) {}
    bool scatter(const Ray& in, const HitRecord& rec,
                 Vec3& attenuation, Ray& scattered) const override;
private:
    Vec3  albedo_;  // 反射率
    float fuzz_;    // 模糊系数
};

} // namespace RayTracing
#endif
