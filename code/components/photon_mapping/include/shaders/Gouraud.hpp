#pragma once
#ifndef __GOURAUD_HPP__
#define __GOURAUD_HPP__

#include "Shader.hpp"

namespace RayCast
{
    // Gouraud着色器 - 在顶点计算光照，然后插值
    class Gouraud : public Shader
    {
    private:
        Vec3 ambientColor;
        Vec3 diffuseColor;
        Vec3 specularColor;
        float specularEx;
        
    public:
        Gouraud(Material& material, vector<Texture>& textures);
        
        // 原有的shade接口（为了兼容性保留）
        virtual RGB shade(const Vec3& in, const Vec3& out, const Vec3& normal) const;

        // Gouraud专用接口：给定三角形三个顶点、三个法线、光源，直接返回插值后的颜色
        RGB shadeTriangle(
            const Vec3& hitPoint,
            const Vec3& v1, const Vec3& v2, const Vec3& v3,
            const Vec3& n1, const Vec3& n2, const Vec3& n3,
            const Vec3& viewDir,
            const Vec3& lightPos,
            const Vec3& lightIntensity
        ) const;
        
        // Gouraud专用：在顶点计算光照颜色
        RGB shadeVertex(const Vec3& vertexPos, const Vec3& vertexNormal, 
                       const Vec3& viewDir, const Vec3& lightPos, 
                       const Vec3& lightIntensity) const;

        // 计算重心坐标
        Vec3 computeBarycentricCoords(const Vec3& p, const Vec3& v1, const Vec3& v2, const Vec3& v3) const;
    };
}

#endif