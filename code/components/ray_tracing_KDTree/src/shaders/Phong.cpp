#include "shaders/Phong.hpp"

namespace RayCast
{
    Vec3 reflect(const Vec3& normal, const Vec3& dir) {
        return dir - 2*glm::dot(dir, normal)*normal;
    }
    Phong::Phong(Material& material, vector<Texture>& textures)
        : Shader                (material, textures)
    {
        using PW = Property::Wrapper;
        auto optAmbientColor = material.getProperty<PW::RGBType>("ambientColor");
        if (optAmbientColor) ambientColor = (*optAmbientColor).value;
        else 
        {
            ambientColor = {1, 1, 1};
            cout << "[Warning] Material has no property named 'ambientColor', using default value (1, 1, 1)." << endl;
        }

        auto optDiffuseColor = material.getProperty<PW::RGBType>("diffuseColor");
        if (optDiffuseColor) diffuseColor = (*optDiffuseColor).value;
        else 
        {
            diffuseColor = {1, 1, 1};
            cout << "[Warning] Material has no property named 'diffuseColor', using default value (1, 1, 1)." << endl;
        }
        
        auto optSpecularColor = material.getProperty<PW::RGBType>("specularColor");
        if (optSpecularColor) specularColor = (*optSpecularColor).value;
        else 
        {
            specularColor = {1, 1, 1};
            cout << "[Warning] Material has no property named 'specularColor', using default value (1, 1, 1)." << endl;
        }

        auto optSpecularEx = material.getProperty<PW::FloatType>("specularEx");
        if (optSpecularEx) specularEx = (*optSpecularEx).value;
        else specularEx = 1;

    }
    RGB Phong::shade(const Vec3& in, const Vec3& out, const Vec3& normal) const {
        Vec3 v = in;
        Vec3 r = reflect(normal, out);
        auto ambient = ambientColor;
        auto diffuse = diffuseColor * fabs(glm::dot(out, normal));
        auto specular = specularColor * fabs(glm::pow(glm::dot(v, r), specularEx));
        return ambient + diffuse + specular;
    }

    Vec3 Phong::computeBarycentricCoords(const Vec3& p, const Vec3& v1, const Vec3& v2, const Vec3& v3) const {
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

    // Phong Shading: 在像素级别使用插值法向量计算光照
    RGB Phong::shadeTriangle(
        const Vec3& hitPoint,
        const Vec3& v1, const Vec3& v2, const Vec3& v3,
        const Vec3& n1, const Vec3& n2, const Vec3& n3,
        const Vec3& viewDir,
        const Vec3& lightPos,
        const RGB& lightIntensity
    ) const {
        // 1. 计算重心坐标
        Vec3 bary = computeBarycentricCoords(hitPoint, v1, v2, v3);
        float alpha = bary.x;
        float beta = bary.y;
        float gamma = bary.z;

        // 2. 使用重心坐标插值法向量
        Vec3 interpolatedNormal = alpha * n1 + beta * n2 + gamma * n3;

        // 3. 归一化插值后的法向量
        interpolatedNormal = glm::normalize(interpolatedNormal);

        // 4. 计算光照方向（从命中点到光源）
        Vec3 L = glm::normalize(lightPos - hitPoint);

        // 5. 计算反射向量
        Vec3 R = reflect(interpolatedNormal, glm::normalize(lightPos - hitPoint));

        // 6. 使用 Phong 光照模型计算颜色
        // I(P) = Ia*ka + Id*kd*max(0, L·N(P)) + Is*ks*(max(0, R·V))^n
        
        // 环境光分量
        // RGB ambient = ambientColor;

        // // 漫反射分量
        // RGB diffuse = diffuseColor * glm::max(0.0f, glm::dot(L, interpolatedNormal));

        // // 镜面反射分量
        // RGB specular = specularColor * glm::pow(glm::max(0.0f, glm::dot(R, viewDir)), specularEx);

        // return ambient + diffuse + specular;
        // return ambientColor + diffuseColor * glm::max(0.0f, glm::dot(glm::normalize(lightPos - hitPoint), interpolatedNormal)) + specularColor * glm::pow(glm::max(0.0f, glm::dot(R, viewDir)), specularEx);
        return ambientColor + diffuseColor * fabs(glm::dot(glm::normalize(lightPos - hitPoint), interpolatedNormal)) + specularColor * fabs(glm::pow((glm::dot(R, viewDir)), specularEx));
    }
}