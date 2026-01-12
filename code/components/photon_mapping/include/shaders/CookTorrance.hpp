#pragma once
#ifndef __COOK_TORRANCE_HPP__
#define __COOK_TORRANCE_HPP__

#include "Shader.hpp"

namespace RayCast
{
    class CookTorrance : public Shader
    {
    private:
        Vec3 baseColor;      // 基础颜色
        float metallic;      // 金属度 [0, 1]
        float roughness;     // 粗糙度 [0, 1]
        Vec3 F0;             // 菲涅尔反射率（非金属默认为 0.04）

        // 辅助函数：GGX/Trowbridge-Reitz 法线分布函数 (NDF)
        float DistributionGGX(const Vec3& N, const Vec3& H, float roughness) const;

        // 辅助函数：Schlick-GGX 几何遮蔽函数
        float GeometrySchlickGGX(float NdotV, float roughness) const;

        // 辅助函数：Smith's method 几何函数
        float GeometrySmith(const Vec3& N, const Vec3& V, const Vec3& L, float roughness) const;

        // 辅助函数：Fresnel-Schlick 近似
        Vec3 FresnelSchlick(float cosTheta, const Vec3& F0) const;

    public:
        CookTorrance(Material& material, vector<Texture>& textures);
        virtual RGB shade(const Vec3& in, const Vec3& out, const Vec3& normal) const override;
    };
}

#endif
