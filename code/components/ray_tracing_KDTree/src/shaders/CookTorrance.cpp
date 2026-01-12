#include "shaders/CookTorrance.hpp"
#include <algorithm>

namespace RayCast
{
    CookTorrance::CookTorrance(Material& material, vector<Texture>& textures)
        : Shader(material, textures)
    {
        using PW = Property::Wrapper;

        // 从材质属性中读取参数
        auto baseColorProp = material.getProperty<PW::RGBType>("baseColor");
        if (baseColorProp) {
            baseColor = (*baseColorProp).value;
        }
        else {
            baseColor = {1, 1, 1};
            cout << "[Warning] Material has no property named 'baseColor', using default value (1, 1, 1)." << endl;
        }

        auto metallicProp = material.getProperty<PW::FloatType>("metallic");
        if (metallicProp) {
            metallic = glm::clamp((*metallicProp).value, 0.0f, 1.0f);
        }
        else {
            metallic = 0.5f;
            cout << "[Warning] Material has no property named 'metallic', using default value 0.5." << endl;
        }

        auto roughnessProp = material.getProperty<PW::FloatType>("roughness");
        if (roughnessProp) {
            roughness = glm::clamp((*roughnessProp).value, 0.01f, 1.0f); // 避免除零
        }
        else {
            roughness = 0.5f;
            cout << "[Warning] Material has no property named 'roughness', using default value 0.5." << endl;
        }

        auto F0Prop = material.getProperty<PW::RGBType>("F0");
        if (F0Prop) {
            F0 = (*F0Prop).value;
        }
        else {
            F0 = {0.04, 0.04, 0.04}; // 非金属默认值
        }

        // 根据金属度插值 F0
        // 金属使用 baseColor 作为 F0，非金属使用默认的 0.04
        F0 = glm::mix(F0, baseColor, metallic);
    }

    // GGX/Trowbridge-Reitz 法线分布函数
    float CookTorrance::DistributionGGX(const Vec3& N, const Vec3& H, float roughness) const
    {
        float a = roughness * roughness;
        float a2 = a * a;
        float NdotH = glm::max(glm::dot(N, H), 0.0f);
        float NdotH2 = NdotH * NdotH;

        float nom = a2;
        float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
        denom = PI * denom * denom;

        return nom / glm::max(denom, 0.0001f); // 防止除零
    }

    // Schlick-GGX 几何遮蔽函数（单向）
    float CookTorrance::GeometrySchlickGGX(float NdotV, float roughness) const
    {
        float r = (roughness + 1.0f);
        float k = (r * r) / 8.0f;  // 用于直接光照的 k

        float nom = NdotV;
        float denom = NdotV * (1.0f - k) + k;

        return nom / glm::max(denom, 0.0001f); // 防止除零
    }

    // Smith's method 几何函数
    float CookTorrance::GeometrySmith(const Vec3& N, const Vec3& V, const Vec3& L, float roughness) const
    {
        float NdotV = glm::max(glm::dot(N, V), 0.0f);
        float NdotL = glm::max(glm::dot(N, L), 0.0f);
        float ggx2 = GeometrySchlickGGX(NdotV, roughness);
        float ggx1 = GeometrySchlickGGX(NdotL, roughness);

        return ggx1 * ggx2;
    }

    // Fresnel-Schlick 近似
    Vec3 CookTorrance::FresnelSchlick(float cosTheta, const Vec3& F0) const
    {
        return F0 + (Vec3(1.0f) - F0) * glm::pow(glm::clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
    }

    RGB CookTorrance::shade(const Vec3& in, const Vec3& out, const Vec3& normal) const
    {
        // in: 从击中点到光源的方向（光线方向 L）
        // out: 从光源到击中点的方向（与传统相反）
        // normal: 表面法线

        Vec3 V = glm::normalize(in);      // 视线方向（从表面指向观察者）
        Vec3 L = glm::normalize(out);     // 光线方向（从表面指向光源）
        Vec3 N = glm::normalize(normal);  // 法线
        Vec3 H = glm::normalize(V + L);   // 半程向量

        // 计算各种点积
        float NdotL = glm::max(glm::dot(N, L), 0.0f);
        float NdotV = glm::max(glm::dot(N, V), 0.0f);
        float HdotV = glm::max(glm::dot(H, V), 0.0f);

        // 如果光线在表面下方，返回黑色
        if (NdotL <= 0.0f) {
            return RGB{0, 0, 0};
        }

        // Cook-Torrance BRDF 计算
        // D: 法线分布函数
        float D = DistributionGGX(N, H, roughness);

        // G: 几何函数
        float G = GeometrySmith(N, V, L, roughness);

        // F: 菲涅尔项
        Vec3 F = FresnelSchlick(HdotV, F0);

        // 镜面反射项（Cook-Torrance）
        Vec3 numerator = D * G * F;
        float denominator = 4.0f * NdotV * NdotL + 0.0001f; // 防止除零
        Vec3 specular = numerator / denominator;

        // 计算能量守恒
        // kS: 镜面反射比例（等于菲涅尔项）
        Vec3 kS = F;
        // kD: 漫反射比例
        Vec3 kD = Vec3(1.0f) - kS;
        // 金属没有漫反射
        kD *= (1.0f - metallic);

        // 漫反射项（Lambertian）
        Vec3 diffuse = kD * baseColor / PI;

        // 组合 BRDF，并乘以入射光的余弦项
        Vec3 result = (diffuse + specular) * NdotL;

        return result;
    }
}
