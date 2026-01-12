#pragma once
#ifndef __RTMATERIALS_CREATOR_HPP__
#define __RTMATERIALS_CREATOR_HPP__

#include "RTMaterial.hpp"
#include "Lambertian.hpp"
#include "Metal.hpp"
#include "Dielectric.hpp"

#include "scene/Material.hpp"
#include "scene/Texture.hpp"
#include "geometry/vec.hpp"

namespace RayTracing {

class RTMaterialsCreator {
public:
    RTMaterialsCreator() = default;

    SRTMat create(NRenderer::Material& material, std::vector<NRenderer::Texture>& /*t*/) {
        using namespace NRenderer;

        switch (material.type) {
        // 0: Lambertian
        case 0: {
            Vec3 diffuseColor;
            auto optDiffuseColor = material.getProperty<Property::Wrapper::RGBType>("diffuseColor");
            if (optDiffuseColor) diffuseColor = (*optDiffuseColor).value;
            else diffuseColor = {1, 1, 1};
            Vec3 albedo = diffuseColor;
            return std::make_shared<Lambertian>(albedo);
        }
        // 1: Phong -> 退化为 Lambertian（使用 diffuseColor）
        case 1: {
            Vec3 diffuseColor;
            auto optDiffuseColor = material.getProperty<Property::Wrapper::RGBType>("diffuseColor");
            if (optDiffuseColor) diffuseColor = (*optDiffuseColor).value;
            else diffuseColor = {1, 1, 1};
            Vec3 albedo = diffuseColor;
            return std::make_shared<Lambertian>(albedo);
        }
        // 2: Dielectric（玻璃）
        case 2: {
            float ior;
            auto optIIor = material.getProperty<Property::Wrapper::FloatType>("ior");
            if (optIIor) ior = (*optIIor).value;
            else ior = 1.5f;

            Vec3 absorbed;
            auto optAbsorbed = material.getProperty<Property::Wrapper::RGBType>("absorbed");
            if (optAbsorbed) absorbed = (*optAbsorbed).value;
            else absorbed = {1,1,1};

            return std::make_shared<Dielectric>(ior, absorbed);
        }
        // 3: Conductor（金属）
        case 3: {
            Vec3 reflect;
            auto optReflect = material.getProperty<Property::Wrapper::RGBType>("reflect");
            if (optReflect) reflect = (*optReflect).value;
            else reflect = {1,1,1};
            float roughness;
            auto optRoughness = material.getProperty<Property::Wrapper::FloatType>("roughness");
            if (optRoughness) roughness = (*optRoughness).value;
            else roughness = 0.0f;
            return std::make_shared<Metal>(reflect, glm::clamp(roughness, 0.f, 1.f));
        }
        // 4: Plastic -> 退化为 Lambertian（使用 diffuseColor）
        case 4: {
            Vec3 diffuseColor;
            auto optDiffuseColor = material.getProperty<Property::Wrapper::RGBType>("diffuseColor");
            if (optDiffuseColor) diffuseColor = (*optDiffuseColor).value;
            else diffuseColor = {1, 1, 1};
            Vec3 albedo = diffuseColor;
            return std::make_shared<Lambertian>(albedo);
        }
        // 5: CookTorrance -> 近似为金属，使用 baseColor/roughness
        case 5: {
            Vec3 baseColor;
            auto optBaseColor = material.getProperty<Property::Wrapper::RGBType>("baseColor");
            if (optBaseColor) baseColor = (*optBaseColor).value;
            else baseColor = {1, 1, 1};
            float roughness;
            auto optRoughness = material.getProperty<Property::Wrapper::FloatType>("roughness");
            if (optRoughness) roughness = (*optRoughness).value;
            else roughness = 0.5f;
            return std::make_shared<Metal>(baseColor, glm::clamp(roughness, 0.f, 1.f));
        }
        // 6: Gouraud -> 退化为 Lambertian（使用 diffuseColor）
        case 6: {
            Vec3 diffuseColor;
            auto optDiffuseColor = material.getProperty<Property::Wrapper::RGBType>("diffuseColor");
            if (optDiffuseColor) diffuseColor = (*optDiffuseColor).value;
            else diffuseColor = {1, 1, 1};
            Vec3 albedo = diffuseColor;
            return std::make_shared<Lambertian>(albedo);
        }
        default: {
            Vec3 diffuseColor;
            auto optDiffuseColor = material.getProperty<Property::Wrapper::RGBType>("diffuseColor");
            if (optDiffuseColor) diffuseColor = (*optDiffuseColor).value;
            else diffuseColor = {1, 1, 1};
            Vec3 albedo = diffuseColor  ;
            return std::make_shared<Lambertian>(albedo);
        }
        }
    }
};

} // namespace RayTracing

#endif
