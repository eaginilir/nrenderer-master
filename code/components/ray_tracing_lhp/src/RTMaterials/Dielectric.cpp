#include "RTMaterials/Dielectric.hpp"

namespace RayTracing {

bool Dielectric::scatter(const Ray& in, const HitRecord& rec,
                         Vec3& attenuation, Ray& scattered) const {
    attenuation = albedo_;
    Vec3 inDir = glm::normalize(in.direction);
    Vec3 n = rec->normal;
    float cosIn = glm::dot(inDir, n);
    bool frontFace = cosIn < 0.f;
    float eta = frontFace ? (1.f / ior_) : ior_; // n1/n2
    if (!frontFace) n = -n;

    float cosTheta = std::min(-glm::dot(inDir, n), 1.0f);
    float sinTheta = std::sqrt(std::max(0.f, 1.0f - cosTheta * cosTheta));

    Vec3 origin = rec->hitPoint + 1e-4f * (frontFace ? n : -n);
    bool cannotRefract = eta * sinTheta > 1.0f;
    Vec3 outDir;
    if (cannotRefract || schlick(cosTheta, eta) > randomFloat()) { // 修正：用 eta
        outDir = glm::reflect(inDir, n);
    } else {
        Vec3 rPerp = eta * (inDir + cosTheta * n);
        Vec3 rPar = -std::sqrt(std::fabs(1.0f - glm::dot(rPerp, rPerp))) * n;
        outDir = rPerp + rPar;
    }
    scattered = Ray{ origin, glm::normalize(outDir) };
    return true;
}

} // namespace RayTracing
