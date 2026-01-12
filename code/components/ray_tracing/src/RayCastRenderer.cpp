#include "RayCastRenderer.hpp"

#include "VertexTransformer.hpp"
#include "intersections/intersections.hpp"
namespace RayCast
{
    void RayCastRenderer::release(const RenderResult& r) {
        auto [p, w, h] = r;
        delete[] p;
    }
    RGB RayCastRenderer::gamma(const RGB& rgb) {
        return glm::sqrt(rgb);
    }
    auto RayCastRenderer::render() -> RenderResult {
        auto width = scene.renderOption.width;
        auto height = scene.renderOption.height;
        auto pixels = new RGBA[width*height];

        VertexTransformer vertexTransformer{};
        vertexTransformer.exec(spScene);

        ShaderCreator shaderCreator{};
        for (auto& mtl : scene.materials) {
            shaderPrograms.push_back(shaderCreator.create(mtl, scene.textures));
        }

        for (int i=0; i<height; i++) {
            for (int j=0; j < width; j++) {
                auto ray = camera.shoot(float(j)/float(width), float(i)/float(height));
                auto color = trace(ray);
                color = clamp(color);
                color = gamma(color);
                pixels[(height-i-1)*width+j] = {color, 1};
            }
        }

        return {pixels, width, height};
    }
    
    RGB RayCastRenderer::trace(const Ray& r) {
        int maxDepth = scene.renderOption.depth;
        struct Node { Ray ray; RGB weight; int depth; };
        std::vector<Node> stack;
        stack.push_back({r, Vec3{1,1,1}, 0});
        RGB total{0,0,0};
        while (!stack.empty()) {
            auto node = stack.back();
            stack.pop_back();
            if (node.depth > maxDepth) continue;
            auto closestHitObj = closestHit(node.ray);
            if (!closestHitObj) continue;
            auto& hitRec = *closestHitObj;
            auto& mat = scene.materials[hitRec.material.index()];
            // Point light direct illumination
            if (!scene.pointLightBuffer.empty()) {
                auto& l = scene.pointLightBuffer[0];
                auto out = glm::normalize(l.position - hitRec.hitPoint);
                float distance = glm::length(l.position - hitRec.hitPoint);
                auto shadowRay = Ray{hitRec.hitPoint, out};
                auto shadowHit = closestHit(shadowRay);
                RGB c = shaderPrograms[hitRec.material.index()]->shade(-node.ray.direction, out, hitRec.normal);
                if (dynamic_pointer_cast<Phong>(shaderPrograms[hitRec.material.index()]) && hitRec.hasVertexData) {
                    auto phongShader = dynamic_pointer_cast<Phong>(shaderPrograms[hitRec.material.index()]);
                    c = phongShader->shadeTriangle(
                        hitRec.hitPoint,
                        hitRec.vertices[0], hitRec.vertices[1], hitRec.vertices[2],
                        hitRec.normals[0], hitRec.normals[1], hitRec.normals[2],
                        -node.ray.direction,
                        l.position,
                        l.intensity
                    );
                }
                if (dynamic_pointer_cast<Gouraud>(shaderPrograms[hitRec.material.index()]) && hitRec.hasVertexData) {
                    auto gouraudShader = dynamic_pointer_cast<Gouraud>(shaderPrograms[hitRec.material.index()]);
                    c = gouraudShader->shadeTriangle(
                        hitRec.hitPoint,
                        hitRec.vertices[0], hitRec.vertices[1], hitRec.vertices[2],
                        hitRec.normals[0], hitRec.normals[1], hitRec.normals[2],
                        -node.ray.direction,
                        l.position,
                        l.intensity
                    );
                }
                if (glm::dot(out, hitRec.normal) >= 0 && ((!shadowHit) || (shadowHit && shadowHit->t > distance))) {
                    total += node.weight * c * l.intensity;
                }
            }
            // Area light direct illumination with geometry term and shadowing
            if (!scene.areaLightBuffer.empty()) {
                int lightSamples = 8;
                for (auto& a : scene.areaLightBuffer) {
                    Vec3 nL = glm::normalize(glm::cross(a.u, a.v));
                    float area = glm::length(glm::cross(a.u, a.v));
                    RGB sum{0,0,0};
                    for (int s=0; s<lightSamples; s++) {
                        float us = (s + 0.5f) / float(lightSamples);
                        float vs = ((s*73) % lightSamples + 0.5f) / float(lightSamples);
                        Vec3 y = a.position + us*a.u + vs*a.v;
                        Vec3 out = glm::normalize(y - hitRec.hitPoint);
                        float d = glm::length(y - hitRec.hitPoint);
                        if (glm::dot(out, hitRec.normal) <= 0) continue;
                        if (glm::dot(nL, -out) <= 0) continue;
                        auto shadowRay = Ray{hitRec.hitPoint, out};
                        auto shadowHit = closestHit(shadowRay);
                        if (shadowHit && shadowHit->t <= d - 0.001f) continue;
                        RGB c = shaderPrograms[hitRec.material.index()]->shade(-node.ray.direction, out, hitRec.normal);
                        if (dynamic_pointer_cast<Phong>(shaderPrograms[hitRec.material.index()]) && hitRec.hasVertexData) {
                            auto phongShader = dynamic_pointer_cast<Phong>(shaderPrograms[hitRec.material.index()]);
                            c = phongShader->shadeTriangle(
                                hitRec.hitPoint,
                                hitRec.vertices[0], hitRec.vertices[1], hitRec.vertices[2],
                                hitRec.normals[0], hitRec.normals[1], hitRec.normals[2],
                                -node.ray.direction,
                                y,
                                a.radiance
                            );
                        }
                        if (dynamic_pointer_cast<Gouraud>(shaderPrograms[hitRec.material.index()]) && hitRec.hasVertexData) {
                            auto gouraudShader = dynamic_pointer_cast<Gouraud>(shaderPrograms[hitRec.material.index()]);
                            c = gouraudShader->shadeTriangle(
                                hitRec.hitPoint,
                                hitRec.vertices[0], hitRec.vertices[1], hitRec.vertices[2],
                                hitRec.normals[0], hitRec.normals[1], hitRec.normals[2],
                                -node.ray.direction,
                                y,
                                a.radiance
                            );
                        }
                        sum += c * a.radiance * (glm::max(0.0f, glm::dot(nL, -out)) / (d*d));
                    }
                    total += node.weight * (sum * (area / float(lightSamples)));
                }
            }
            using PW = Property::Wrapper;
            auto reflectProp = mat.getProperty<PW::RGBType>("reflect");
            if (reflectProp) {
                Vec3 dir = glm::normalize(glm::reflect(node.ray.direction, hitRec.normal));
                stack.push_back({Ray{hitRec.hitPoint, dir}, node.weight * (*reflectProp).value, node.depth + 1});
            }
            auto iorProp = mat.getProperty<PW::FloatType>("ior");
            if (iorProp) {
                float ior = (*iorProp).value;
                Vec3 n = hitRec.normal;
                bool front = glm::dot(node.ray.direction, n) < 0;
                float eta = front ? (1.0f / ior) : ior;
                n = front ? n : -n;
                float cosTheta = glm::min(1.0f, glm::dot(-node.ray.direction, n));
                float sinTheta = sqrt(glm::max(0.0f, 1.0f - cosTheta*cosTheta));
                bool tir = eta * sinTheta > 1.0f;
                float R0 = (ior - 1.0f) / (ior + 1.0f); R0 *= R0;
                float Fr = tir ? 1.0f : (R0 + (1.0f - R0) * pow(1.0f - cosTheta, 5.0f));
                Vec3 reflDir = glm::normalize(glm::reflect(node.ray.direction, n));
                stack.push_back({Ray{hitRec.hitPoint, reflDir}, node.weight * Fr, node.depth + 1});
                if (!tir) {
                    Vec3 refrDir = glm::normalize(glm::refract(node.ray.direction, n, eta));
                    RGB w = node.weight * (1.0f - Fr);
                    auto absorbedProp = mat.getProperty<PW::RGBType>("absorbed");
                    if (absorbedProp) w *= (*absorbedProp).value;
                    stack.push_back({Ray{hitRec.hitPoint, refrDir}, w, node.depth + 1});
                }
            }
        }
        return total;
    }

    HitRecord RayCastRenderer::closestHit(const Ray& r) {
        HitRecord closestHit = nullopt;
        float closest = FLOAT_INF;
        for (auto& s : scene.sphereBuffer) {
            auto hitRecord = Intersection::xSphere(r, s, 0.01, closest);
            if (hitRecord && hitRecord->t < closest) {
                closest = hitRecord->t;
                closestHit = hitRecord;
            }
        }
        for (auto& t : scene.triangleBuffer) {
            auto hitRecord = Intersection::xTriangle(r, t, 0.01, closest);
            if (hitRecord && hitRecord->t < closest) {
                closest = hitRecord->t;
                closestHit = hitRecord;
            }
        }
        for (auto& p : scene.planeBuffer) {
            auto hitRecord = Intersection::xPlane(r, p, 0.01, closest);
            if (hitRecord && hitRecord->t < closest) {
                closest = hitRecord->t;
                closestHit = hitRecord;
            }
        }
        for (auto& m : scene.meshBuffer) {
            auto hitRecord = Intersection::xMesh(r, m, 0.01, closest);
            if (hitRecord && hitRecord->t < closest) {
                closest = hitRecord->t;
                closestHit = hitRecord;
            }
        }
        return closestHit; 
    }
}