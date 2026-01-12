#include "server/Server.hpp"
#include "EnvMapPathTracer.hpp"
#include "VertexTransformer.hpp"
#include "intersections/intersections.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace EnvMapPathTracer
{
    RGB EnvMapPathTracerRenderer::gamma(const RGB& rgb) {
        return glm::sqrt(rgb);
    }

    void EnvMapPathTracerRenderer::renderTask(RGBA* pixels, int width, int height, int off, int step) {
        for (int i = off; i < height; i += step) {
            for (int j = 0; j < width; j++) {
                Vec3 color{0, 0, 0};
                for (int k = 0; k < samples; k++) {
                    auto r = defaultSamplerInstance<UniformInSquare>().sample2d();
                    float rx = r.x;
                    float ry = r.y;
                    float x = (float(j) + rx) / float(width);
                    float y = (float(i) + ry) / float(height);
                    auto ray = camera.shoot(x, y);
                    color += trace(ray, 0);
                }
                color /= samples;
                color = gamma(color);
                pixels[(height - i - 1) * width + j] = {color, 1};
            }
        }
    }

    auto EnvMapPathTracerRenderer::render() -> RenderResult {
        shaderPrograms.clear();
        ShaderCreator shaderCreator{};
        for (auto& m : scene.materials) {
            shaderPrograms.push_back(shaderCreator.create(m, scene.textures));
        }

        RGBA* pixels = new RGBA[width * height]{};

        VertexTransformer vertexTransformer{};
        vertexTransformer.exec(spScene);

        const auto taskNums = 8;
        thread t[taskNums];
        for (int i = 0; i < taskNums; i++) {
            t[i] = thread(&EnvMapPathTracerRenderer::renderTask,
                this, pixels, width, height, i, taskNums);
        }
        for (int i = 0; i < taskNums; i++) {
            t[i].join();
        }
        getServer().logger.log("Done...");
        return {pixels, width, height};
    }

    void EnvMapPathTracerRenderer::release(const RenderResult& r) {
        auto [p, w, h] = r;
        delete[] p;
    }

    HitRecord EnvMapPathTracerRenderer::closestHitObject(const Ray& r) {
        HitRecord closestHit = nullopt;
        float closest = FLOAT_INF;
        for (auto& s : scene.sphereBuffer) {
            auto hitRecord = Intersection::xSphere(r, s, 0.000001, closest);
            if (hitRecord && hitRecord->t < closest) {
                closest = hitRecord->t;
                closestHit = hitRecord;
            }
        }
        for (auto& t : scene.triangleBuffer) {
            auto hitRecord = Intersection::xTriangle(r, t, 0.000001, closest);
            if (hitRecord && hitRecord->t < closest) {
                closest = hitRecord->t;
                closestHit = hitRecord;
            }
        }
        for (auto& p : scene.planeBuffer) {
            auto hitRecord = Intersection::xPlane(r, p, 0.000001, closest);
            if (hitRecord && hitRecord->t < closest) {
                closest = hitRecord->t;
                closestHit = hitRecord;
            }
        }
        return closestHit;
    }

    tuple<float, Vec3> EnvMapPathTracerRenderer::closestHitLight(const Ray& r) {
        Vec3 v = {};
        HitRecord closest = getHitRecord(FLOAT_INF, {}, {}, {});
        for (auto& a : scene.areaLightBuffer) {
            auto hitRecord = Intersection::xAreaLight(r, a, 0.000001, closest->t);
            if (hitRecord && closest->t > hitRecord->t) {
                closest = hitRecord;
                v = a.radiance;
            }
        }
        return {closest->t, v};
    }

    RGB EnvMapPathTracerRenderer::trace(const Ray& r, int currDepth) {
        if (currDepth == depth) return Vec3{0};

        auto hitObject = closestHitObject(r);
        auto [t, emitted] = closestHitLight(r);

        // 击中物体
        if (hitObject && hitObject->t < t) {
            auto mtlHandle = hitObject->material;
            auto scattered = shaderPrograms[mtlHandle.index()]->shade(r, hitObject->hitPoint, hitObject->normal);
            auto scatteredRay = scattered.ray;
            auto attenuation = scattered.attenuation;
            auto emittedLight = scattered.emitted;
            auto next = trace(scatteredRay, currDepth + 1);
            float pdf = scattered.pdf;

            // delta分布材质(镜面/玻璃)直接返回 attenuation * next
            if (pdf >= 1.0f) {
                return emittedLight + attenuation * next;
            }
            // 漫反射材质需要乘以 cos(theta) / pdf
            float n_dot_in = glm::abs(glm::dot(hitObject->normal, scatteredRay.direction));
            return emittedLight + attenuation * next * n_dot_in / pdf;
        }
        // 击中面光源
        else if (t != FLOAT_INF) {
            return emitted;
        }
        // 未击中任何物体 - 采样环境贴图
        else {
            return getEnvironmentLight(r.direction);
        }
    }
}
