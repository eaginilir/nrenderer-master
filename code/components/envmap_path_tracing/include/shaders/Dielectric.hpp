#pragma once
#ifndef __ENVMAP_DIELECTRIC_HPP__
#define __ENVMAP_DIELECTRIC_HPP__

#include "Shader.hpp"
#include "samplers/SamplerInstance.hpp"

namespace EnvMapPathTracer
{
    // 绝缘体材质 - 玻璃/水等透明材质
    class Dielectric : public Shader
    {
    private:
        float ior;      // 折射率
        Vec3 absorbed;  // 吸收颜色
    public:
        Dielectric(Material& material, vector<Texture>& textures);
        Scattered shade(const Ray& ray, const Vec3& hitPoint, const Vec3& normal) const;
    };
}

#endif
