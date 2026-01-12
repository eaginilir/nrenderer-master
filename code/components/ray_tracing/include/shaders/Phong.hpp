#pragma once
#ifndef __PHONG_HPP__
#define __PHONG_HPP__

#include "Shader.hpp"

namespace RayCast
{
    class Phong : public Shader
    {
    private:
        Vec3 ambientColor;
        Vec3 diffuseColor;
        Vec3 specularColor;
        float specularEx;
    public:
        Phong(Material& material, vector<Texture>& textures);
        virtual RGB shade(const Vec3& in, const Vec3& out, const Vec3& normal) const;
        RGB shadeTriangle(
            const Vec3& hitPoint,
            const Vec3& v1, const Vec3& v2, const Vec3& v3,
            const Vec3& n1, const Vec3& n2, const Vec3& n3,
            const Vec3& viewDir,
            const Vec3& lightPos,
            const RGB& lightIntensity
        ) const;

        Vec3 computeBarycentricCoords(const Vec3& p, const Vec3& v1, const Vec3& v2, const Vec3& v3) const;
    };
}

#endif