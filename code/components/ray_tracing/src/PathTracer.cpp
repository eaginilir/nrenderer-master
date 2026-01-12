// d:\study\computer_graph\nrenderer-master\code\components\ray_tracing\src\PathTracer.cpp
#include "PathTracer.hpp"

#include <thread>
#include <random>
#include "glm/gtc/matrix_transform.hpp"

namespace RayCast
{
    RGB PathTracerRenderer::gamma(const RGB& rgb) {
        return glm::sqrt(rgb);
    }

    void PathTracerRenderer::release(const RenderResult& r) {
        auto [p, w, h] = r;
        delete[] p;
    }

    Vec3 PathTracerRenderer::sampleHemisphereUniform() const {
        thread_local static std::mt19937 rng{std::random_device{}()};
        thread_local static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        float z = dist(rng);
        float phi = 6.283185307179586f * dist(rng);
        float r = sqrt(glm::max(0.0f, 1.0f - z*z));
        return {r * cos(phi), r * sin(phi), z};
    }

    Vec3 PathTracerRenderer::toWorld(const Vec3& n, const Vec3& local) const {
        Vec3 w = glm::normalize(n);
        Vec3 a = (fabs(w.x) > 0.9f) ? Vec3{0, 1, 0} : Vec3{1, 0, 0};
        Vec3 v = glm::normalize(glm::cross(w, a));
        Vec3 u = glm::cross(v, w);
        return local.x * u + local.y * v + local.z * w;
    }

    void PathTracerRenderer::renderTask(RGBA* pixels, int width, int height, int off, int step) {
        thread_local static std::mt19937 rng{std::random_device{}()};
        thread_local static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        for(int i=off; i<height; i+=step) {
            for (int j=0; j<width; j++) {
                Vec3 color{0, 0, 0};
                for (int k=0; k<samples; k++) {
                    float rx = dist(rng);
                    float ry = dist(rng);
                    float x = (float(j)+rx)/float(width);
                    float y = (float(i)+ry)/float(height);
                    auto ray = camera.shoot(x, y);
                    color += trace(ray, 0);
                }
                color /= float(samples);
                color = gamma(color);
                pixels[(height-i-1)*width+j] = {color, 1};
            }
        }
    }

    auto PathTracerRenderer::render() -> RenderResult {
        VertexTransformer vertexTransformer{};
        vertexTransformer.exec(spScene);

        RGBA* pixels = new RGBA[width*height]{};

        const int taskNums = 8;
        std::thread t[taskNums];
        for (int i=0; i < taskNums; i++) {
            t[i] = std::thread(&PathTracerRenderer::renderTask, this, pixels, width, height, i, taskNums);
        }
        for (int i=0; i < taskNums; i++) {
            t[i].join();
        }
        return {pixels, width, height};
    }

    HitRecord PathTracerRenderer::closestHitObject(const Ray& r) {
        HitRecord closestHit = nullopt;
        float closest = FLOAT_INF;
        for (auto& s : scene.sphereBuffer) {
            auto hitRecord = Intersection::xSphere(r, s, 0.000001f, closest);
            if (hitRecord && hitRecord->t < closest) { closest = hitRecord->t; closestHit = hitRecord; }
        }
        for (auto& t : scene.triangleBuffer) {
            auto hitRecord = Intersection::xTriangle(r, t, 0.000001f, closest);
            if (hitRecord && hitRecord->t < closest) { closest = hitRecord->t; closestHit = hitRecord; }
        }
        for (auto& p : scene.planeBuffer) {
            auto hitRecord = Intersection::xPlane(r, p, 0.000001f, closest);
            if (hitRecord && hitRecord->t < closest) { closest = hitRecord->t; closestHit = hitRecord; }
        }
        for (auto& m : scene.meshBuffer) {
            auto hitRecord = Intersection::xMesh(r, m, 0.000001f, closest);
            if (hitRecord && hitRecord->t < closest) { closest = hitRecord->t; closestHit = hitRecord; }
        }
        return closestHit;
    }

    tuple<float, Vec3> PathTracerRenderer::closestHitLight(const Ray& r) {
        Vec3 v = {};
        HitRecord closest = getHitRecord(FLOAT_INF, {}, {}, {});
        for (auto& a : scene.areaLightBuffer) {
            auto hitRecord = Intersection::xAreaLight(r, a, 0.000001f, closest->t);
            if (hitRecord && closest->t > hitRecord->t) {
                closest = hitRecord;
                v = a.radiance;
            }
        }
        return { closest->t, v };
    }

    RGB PathTracerRenderer::trace(const Ray& r, int currDepth) {
        if (currDepth == depth) return scene.ambient.constant;
        auto hitObject = closestHitObject(r);
        auto [tLight, emitted] = closestHitLight(r);
        if (hitObject && hitObject->t < tLight) {
            auto& mtl = scene.materials[hitObject->material.index()];
            using PW = Property::Wrapper;
            Vec3 albedo{1, 1, 1};
            auto diffuseColor = mtl.getProperty<PW::RGBType>("diffuseColor");
            if (diffuseColor) albedo = (*diffuseColor).value;

            Vec3 origin = hitObject->hitPoint + 0.0001f * hitObject->normal;

            Vec3 direct{0, 0, 0};
            if (diffuseColor && !scene.areaLightBuffer.empty()) {
                int lightSamples = 8;
                for (auto& a : scene.areaLightBuffer) {
                    Vec3 nL = glm::normalize(glm::cross(a.u, a.v));
                    float area = glm::length(glm::cross(a.u, a.v));
                    for (int s=0; s<lightSamples; s++) {
                        float us = (s + 0.5f) / float(lightSamples);
                        float vs = ((s*73) % lightSamples + 0.5f) / float(lightSamples);
                        Vec3 y = a.position + us*a.u + vs*a.v;
                        Vec3 out = glm::normalize(y - origin);
                        float d = glm::length(y - origin);
                        if (glm::dot(out, hitObject->normal) <= 0) continue;
                        if (glm::dot(nL, -out) <= 0) continue;
                        auto shadowHit = closestHitObject(Ray{origin, out});
                        if (shadowHit && shadowHit->t <= d - 0.001f) continue;
                        float G = glm::max(0.0f, glm::dot(hitObject->normal, out)) * glm::max(0.0f, glm::dot(nL, -out)) / (d*d);
                        direct += (albedo / 3.1415926535898f) * a.radiance * G;
                    }
                    direct *= (area / float(lightSamples));
                }
            }

            Vec3 local = sampleHemisphereUniform();
            Vec3 direction = glm::normalize(toWorld(hitObject->normal, local));

            float pdf = 1.0f/(2.0f*3.1415926535898f);
            Vec3 attenuation = albedo / 3.1415926535898f;

            auto next = trace(Ray{origin, direction}, currDepth+1);
            float n_dot_in = glm::dot(hitObject->normal, direction);
            return direct + attenuation * next * n_dot_in / pdf;
        }
        else if (tLight != FLOAT_INF) {
            return emitted;
        }
        else {
            return Vec3{0};
        }
    }
}