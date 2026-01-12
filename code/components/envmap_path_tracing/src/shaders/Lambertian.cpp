#include "shaders/Lambertian.hpp"
#include "samplers/SamplerInstance.hpp"
#include "samplers/CosineWeighted.hpp"
#include "Onb.hpp"

namespace EnvMapPathTracer
{
    Lambertian::Lambertian(Material& material, vector<Texture>& textures)
        : Shader(material, textures)
    {
        auto diffuseColor = material.getProperty<Property::Wrapper::RGBType>("diffuseColor");
        if (diffuseColor) albedo = (*diffuseColor).value;
        else albedo = {1, 1, 1};
    }

    Scattered Lambertian::shade(const Ray& ray, const Vec3& hitPoint, const Vec3& normal) const {
        Vec3 origin = hitPoint;
        // 使用 Cosine-weighted 采样
        Vec3 random = defaultSamplerInstance<CosineWeighted>().sample3d();
        Onb onb{normal};
        Vec3 direction = glm::normalize(onb.local(random));

        // Cosine-weighted PDF = cos(theta) / PI
        float cosTheta = glm::dot(direction, normal);
        float pdf = cosTheta / PI;

        // BRDF = albedo / PI
        auto attenuation = albedo / PI;

        return {
            Ray{origin, direction},
            attenuation,
            Vec3{0},
            pdf
        };
    }
}
