#pragma once
#ifndef __ENVMAP_LAMBERTIAN_HPP__
#define __ENVMAP_LAMBERTIAN_HPP__

#include "Shader.hpp"

namespace EnvMapPathTracer
{
    class Lambertian : public Shader
    {
    private:
        Vec3 albedo;
    public:
        Lambertian(Material& material, vector<Texture>& textures);
        Scattered shade(const Ray& ray, const Vec3& hitPoint, const Vec3& normal) const;
    };
}

#endif
