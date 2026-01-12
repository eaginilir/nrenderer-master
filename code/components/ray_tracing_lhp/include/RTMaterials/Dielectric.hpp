#pragma once
#ifndef __DIELECTRIC_HPP__
#define __DIELECTRIC_HPP__

#include "RTMaterial.hpp"

namespace RayTracing {

class Dielectric final : public RTMaterial {
public:
    Dielectric(float ior, const Vec3& albedo = Vec3{1,1,1}) : ior_(ior), albedo_(albedo) {}
    bool scatter(const Ray& in, const HitRecord& rec,
                 Vec3& attenuation, Ray& scattered) const override;
private:
    float ior_; // 折射率
    Vec3  albedo_;  // 反射率
};

} // namespace RayTracing
#endif
