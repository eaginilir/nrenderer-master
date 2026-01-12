#include "RTMaterials/Metal.hpp"

namespace RayTracing {

bool Metal::scatter(const Ray& in, const HitRecord& rec,
                    Vec3& attenuation, Ray& scattered) const {
    Vec3 inDir = glm::normalize(in.direction);
    Vec3 reflected = glm::reflect(inDir, glm::normalize(rec->normal));
    Vec3 origin = rec->hitPoint + 1e-4f * rec->normal;
    Vec3 dir = glm::normalize(reflected + fuzz_ * randomInUnitSphere());
    if (glm::dot(dir, rec->normal) <= 0.f) return false;
    scattered = Ray{ origin, dir };
    attenuation = albedo_;
    return true;
}

} // namespace RayTracing
