#pragma once
#ifndef __LAMBERTIAN_HPP__
#define __LAMBERTIAN_HPP__

#include "RTMaterial.hpp"

namespace RayTracing {

class Lambertian final : public RTMaterial {
public:
    explicit Lambertian(const Vec3& albedo) : albedo_(albedo) {}
    bool scatter(const Ray& in, const HitRecord& rec,
                 Vec3& attenuation, Ray& scattered) const override;
private:
    Vec3 albedo_;   // 反射率
};

} // namespace RayTracing
#endif
