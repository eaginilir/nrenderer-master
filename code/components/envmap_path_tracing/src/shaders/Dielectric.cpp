#include "shaders/Dielectric.hpp"

namespace EnvMapPathTracer
{
    Dielectric::Dielectric(Material& material, vector<Texture>& textures)
        : Shader(material, textures)
    {
        auto iorProp = material.getProperty<Property::Wrapper::FloatType>("ior");
        if (iorProp) ior = (*iorProp).value;
        else ior = 1.5f;  // 默认玻璃折射率

        auto absorbedProp = material.getProperty<Property::Wrapper::RGBType>("absorbed");
        if (absorbedProp) absorbed = (*absorbedProp).value;
        else absorbed = {1, 1, 1};
    }

    Scattered Dielectric::shade(const Ray& ray, const Vec3& hitPoint, const Vec3& normal) const {
        Vec3 unitDir = glm::normalize(ray.direction);
        float cosI = -glm::dot(unitDir, normal);

        // 判断光线是从外部进入还是从内部射出
        Vec3 n;
        float eta;
        if (cosI > 0) {
            // 从外部进入
            n = normal;
            eta = 1.0f / ior;
        } else {
            // 从内部射出
            n = -normal;
            cosI = -cosI;
            eta = ior;
        }

        // 计算折射
        float k = 1.0f - eta * eta * (1.0f - cosI * cosI);

        Vec3 direction;
        bool isRefract = false;

        if (k >= 0) {
            // 可以折射，计算 Schlick 反射概率
            float r0 = (1.0f - ior) / (1.0f + ior);
            r0 = r0 * r0;
            float reflectProb = r0 + (1.0f - r0) * pow(1.0f - cosI, 5.0f);

            if (defaultSamplerInstance<UniformSampler>().sample1d() > reflectProb) {
                // 折射: eta * I + (eta * cosI - sqrt(k)) * N
                direction = eta * unitDir + (eta * cosI - sqrt(k)) * n;
                isRefract = true;
            } else {
                // 反射
                direction = unitDir + 2.0f * cosI * n;
            }
        } else {
            // 全内反射
            direction = unitDir + 2.0f * cosI * n;
        }

        direction = glm::normalize(direction);

        // 偏移避免自相交
        Vec3 offsetPoint = isRefract ? (hitPoint - n * 0.001f) : (hitPoint + n * 0.001f);

        return {
            Ray{offsetPoint, direction},
            absorbed,
            Vec3{0},
            1.0f
        };
    }
}
