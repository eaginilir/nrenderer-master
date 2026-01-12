#include "shaders/Gouraud.hpp"

namespace RayCast
{
    Vec3 reflectGouraud(const Vec3& normal, const Vec3& dir) {
        return dir - 2.0f * glm::dot(dir, normal) * normal;
    }
    
    Gouraud::Gouraud(Material& material, vector<Texture>& textures)
        : Shader(material, textures)
    {
        using PW = Property::Wrapper;
        
        auto optAmbientColor = material.getProperty<PW::RGBType>("ambientColor");
        if (optAmbientColor) ambientColor = (*optAmbientColor).value;
        else 
        {
            ambientColor = {1, 1, 1};
            std::cout << "[Warning] Material has no property named 'ambientColor', using default value (1, 1, 1)." << std::endl;
        }

        auto optDiffuseColor = material.getProperty<PW::RGBType>("diffuseColor");
        if (optDiffuseColor) diffuseColor = (*optDiffuseColor).value;
        else 
        {
            diffuseColor = {1, 1, 1};
            std::cout << "[Warning] Material has no property named 'diffuseColor', using default value (1, 1, 1)." << std::endl;
        }
        
        auto optSpecularColor = material.getProperty<PW::RGBType>("specularColor");
        if (optSpecularColor) specularColor = (*optSpecularColor).value;
        else 
        {
            specularColor = {1, 1, 1};
            std::cout << "[Warning] Material has no property named 'specularColor', using default value (1, 1, 1)." << std::endl;
        }

        auto optSpecularEx = material.getProperty<PW::FloatType>("specularEx");
        if (optSpecularEx) specularEx = (*optSpecularEx).value;
        else specularEx = 1;
    }
    
    RGB Gouraud::shade(const Vec3& in, const Vec3& out, const Vec3& normal) const {
        // 兼容接口 - 退化为Phong着色
        Vec3 v = in;
        Vec3 r = reflectGouraud(normal, out);
        auto ambient = ambientColor;
        auto diffuse = diffuseColor * fabs(glm::dot(out, normal));
        auto specular = specularColor * fabs(glm::pow(glm::dot(v, r), specularEx));
        return ambient + diffuse + specular;
    }
    
    RGB Gouraud::shadeVertex(const Vec3& vertexPos, const Vec3& vertexNormal, 
                            const Vec3& viewDir, const Vec3& lightPos, 
                            const Vec3& lightIntensity) const {
        // Blinn-Phong光照模型（在顶点处计算）
        Vec3 N = glm::normalize(vertexNormal);
        Vec3 L = glm::normalize(lightPos - vertexPos);  // 光源方向
        Vec3 V = glm::normalize(viewDir);                // 视线方向
        Vec3 H = glm::normalize(L + V);                  // 半程向量
        
        // 环境光
        Vec3 ambient = ambientColor;
        
        // 漫反射
        float NdotL = std::max(glm::dot(N, L), 0.0f);
        Vec3 diffuse = diffuseColor * NdotL;
        
        // 镜面反射（Blinn-Phong）
        float NdotH = std::max(glm::dot(N, H), 0.0f);
        Vec3 specular = specularColor * std::pow(NdotH, specularEx);
        
        // 最终颜色 = 环境光 + 漫反射 + 镜面反射
        return ambient + diffuse + specular;
    }

    Vec3 Gouraud::computeBarycentricCoords(const Vec3& p, const Vec3& v1, const Vec3& v2, const Vec3& v3) const {
        Vec3 v0 = v2 - v1;
        Vec3 v1_local = v3 - v1;
        Vec3 v2_local = p - v1;
        
        float d00 = glm::dot(v0, v0);
        float d01 = glm::dot(v0, v1_local);
        float d11 = glm::dot(v1_local, v1_local);
        float d20 = glm::dot(v2_local, v0);
        float d21 = glm::dot(v2_local, v1_local);
        
        float denom = d00 * d11 - d01 * d01;
        if (fabs(denom) < 1e-6f) {
            // 退化三角形，返回第一个顶点的坐标
            return Vec3(1.0f, 0.0f, 0.0f);
        }
        
        float v = (d11 * d20 - d01 * d21) / denom;
        float w = (d00 * d21 - d01 * d20) / denom;
        float u = 1.0f - v - w;
        
        return Vec3(u, v, w);
    }

    RGB Gouraud::shadeTriangle(
        const Vec3& hitPoint,
        const Vec3& v1, const Vec3& v2, const Vec3& v3,
        const Vec3& n1, const Vec3& n2, const Vec3& n3,
        const Vec3& viewDir,
        const Vec3& lightPos,
        const Vec3& lightIntensity
    ) const {
        // 计算重心坐标
        Vec3 bary = computeBarycentricCoords(hitPoint, v1, v2, v3);
        
        // 在三个顶点分别计算光照
        RGB c1 = shadeVertex(v1, n1, viewDir, lightPos, lightIntensity);
        RGB c2 = shadeVertex(v2, n2, viewDir, lightPos, lightIntensity);
        RGB c3 = shadeVertex(v3, n3, viewDir, lightPos, lightIntensity);
        
        // 使用重心坐标插值
        return bary.x * c1 + bary.y * c2 + bary.z * c3;
    }
}