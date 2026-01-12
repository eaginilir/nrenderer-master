#include "RTMaterials/Lambertian.hpp"

namespace RayTracing {

bool Lambertian::scatter(const Ray& in, const HitRecord& rec,
                         Vec3& attenuation, Ray& scattered) const {
    (void)in;
    Vec3 origin = rec->hitPoint + 1e-4f * rec->normal;
    Vec3 dir = glm::normalize(rec->normal + randomUnitVector());
    scattered = Ray{ origin, dir };
    attenuation = albedo_;
    return true;
}

} // namespace RayTracing
