#include "shaders/Conductor.hpp"

namespace EnvMapPathTracer
{
    Conductor::Conductor(Material& material, vector<Texture>& textures)
        : Shader(material, textures)
    {
        auto color = material.getProperty<Property::Wrapper::RGBType>("albedo");
        if (color) albedo = (*color).value;
        else albedo = {1, 1, 1};
    }

    Scattered Conductor::shade(const Ray& ray, const Vec3& hitPoint, const Vec3& normal) const {
        // 镜面反射: r = d - 2(d·n)n
        Vec3 reflected = ray.direction - 2.0f * glm::dot(ray.direction, normal) * normal;
        reflected = glm::normalize(reflected);

        return {
            Ray{hitPoint, reflected},
            albedo,      // 金属反射颜色
            Vec3{0},     // 无自发光
            1.0f         // delta分布，pdf=1
        };
    }
}
