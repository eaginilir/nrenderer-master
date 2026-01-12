#pragma once
#ifndef __ENVMAP_CONDUCTOR_HPP__
#define __ENVMAP_CONDUCTOR_HPP__

#include "Shader.hpp"

namespace EnvMapPathTracer
{
    // 导体材质 - 镜面反射
    class Conductor : public Shader
    {
    private:
        Vec3 albedo;  // 反射颜色/金属色
    public:
        Conductor(Material& material, vector<Texture>& textures);
        Scattered shade(const Ray& ray, const Vec3& hitPoint, const Vec3& normal) const;
    };
}

#endif
