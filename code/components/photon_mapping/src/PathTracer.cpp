#include "PathTracer.hpp"

#include <thread>
#include <random>
#include <fstream>
#include <algorithm>
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
        thread_local static std::mt19937 rng{std::random_device{ }()};
        thread_local static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        float z = dist(rng);
        float phi = 6.283185307179586f * dist(rng);
        float r = sqrt(glm::max(0.0f, 1.0f - z*z));
        return {r * cos(phi), r * sin(phi), z};
    }

    Vec3 PathTracerRenderer::sampleHemisphereCosine() const {
        thread_local static std::mt19937 rng{std::random_device{ }()};
        thread_local static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        float r1 = dist(rng);
        float r2 = dist(rng);
        float phi = 6.283185307179586f * r1;
        float x = cos(phi) * sqrt(r2);
        float y = sin(phi) * sqrt(r2);
        float z = sqrt(glm::max(0.0f, 1.0f - r2));
        return {x, y, z};
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

        accel = std::make_unique<KDTree>();
        accel->buildFromScene(scene);

        buildPhotonMap();

        RGBA* pixels = new RGBA[width*height]{};

        const int taskNums = 8;
        std::thread t[taskNums];
        for (int i=0; i < taskNums; i++) {
            t[i] = std::thread(&PathTracerRenderer::renderTask, this, pixels, width, height, i, taskNums);
        }
        for (int i=0; i < taskNums; i++) {
            t[i].join();
        }
        if (photonMap) {
            const auto& pts = photonMap->getPhotons();
            for (const auto& ph : pts) {
                float sx, sy;
                if (camera.project(ph.position, sx, sy)) {
                    int xi = std::min(std::max(int(sx * width), 0), int(width) - 1);
                    int yi = std::min(std::max(int(sy * height), 0), int(height) - 1);
                    for (int oy=-1; oy<=1; ++oy) {
                        for (int ox=-1; ox<=1; ++ox) {
                            int X = xi + ox, Y = yi + oy;
                            if (X < 0 || X >= int(width) || Y < 0 || Y >= int(height)) continue;
                            auto& px = pixels[(height - Y - 1) * width + X];
                            px.x = glm::min(1.0f, px.x + 0.6f);
                            px.y = glm::min(1.0f, px.y + 0.6f);
                            px.z = glm::min(1.0f, px.z + 0.0f);
                            px.w = 1.0f;
                        }
                    }
                }
            }
        }
        if (photonMap) {
            const auto& pts = photonMap->getPhotons();
            for (const auto& ph : pts) {
                float sx, sy;
                if (camera.project(ph.position, sx, sy)) {
                    int xi = std::min(std::max(int(sx * width), 0), int(width) - 1);
                    int yi = std::min(std::max(int(sy * height), 0), int(height) - 1);
                    for (int oy=-1; oy<=1; ++oy) {
                        for (int ox=-1; ox<=1; ++ox) {
                            int X = xi + ox, Y = yi + oy;
                            if (X < 0 || X >= int(width) || Y < 0 || Y >= int(height)) continue;
                            auto& px = pixels[(height - Y - 1) * width + X];
                            px.x = glm::min(1.0f, px.x + 0.6f);
                            px.y = glm::min(1.0f, px.y + 0.6f);
                            px.z = glm::min(1.0f, px.z + 0.0f);
                            px.w = 1.0f;
                        }
                    }
                }
            }
        }
        Vec3 imageEnergyLinear{0, 0, 0};
        for (int i=0; i<height; ++i) {
            for (int j=0; j<width; ++j) {
                auto& p = pixels[(height-i-1)*width + j];
                imageEnergyLinear += Vec3{p.x*p.x, p.y*p.y, p.z*p.z};
            }
        }
        {
            std::ofstream ofs("photon_map.ppm", std::ios::binary);
            if (ofs.good()) {
                ofs << "P6\n" << width << " " << height << "\n255\n";
                for (unsigned int i=0; i<height; ++i) {
                    for (unsigned int j=0; j<width; ++j) {
                        auto& p = pixels[i*width + j];
                        unsigned char r = (unsigned char)(glm::clamp(p.x, 0.0f, 1.0f) * 255.0f);
                        unsigned char g = (unsigned char)(glm::clamp(p.y, 0.0f, 1.0f) * 255.0f);
                        unsigned char b = (unsigned char)(glm::clamp(p.z, 0.0f, 1.0f) * 255.0f);
                        ofs.write((char*)&r, 1);
                        ofs.write((char*)&g, 1);
                        ofs.write((char*)&b, 1);
                    }
                }
                ofs.close();
                std::cout << "[PhotonMap] Saved image to photon_map.ppm" << std::endl;
            }
        }
        std::cout << "[PhotonMap] ImageEnergy(linear_est) " << imageEnergyLinear << std::endl;
        return {pixels, width, height};
    }

    HitRecord PathTracerRenderer::closestHitObject(const Ray& r) {
        HitRecord closestHit = nullopt;
        float closest = FLOAT_INF;
        for (auto& s : scene.sphereBuffer) {
            auto hitRecord = Intersection::xSphere(r, s, 0.000001f, closest);
            if (hitRecord && hitRecord->t < closest) { closest = hitRecord->t; closestHit = hitRecord; }
        }
        if (accel) {
            auto hitRecord = accel->closestHit(r, 0.000001f, closest);
            if (hitRecord && hitRecord->t < closest) { closest = hitRecord->t; closestHit = hitRecord; }
        }
        for (auto& p : scene.planeBuffer) {
            auto hitRecord = Intersection::xPlane(r, p, 0.000001f, closest);
            if (hitRecord && hitRecord->t < closest) { closest = hitRecord->t; closestHit = hitRecord; }
        }

        return closestHit;
    }

    tuple<float, Vec3> PathTracerRenderer::closestHitLight(const Ray& r) {
        Vec3 v = {};
        HitRecord closest = getHitRecord(FLOAT_INF, {}, {}, {});
        for (auto& a : scene.areaLightBuffer) {
            auto hitRecord = Intersection::xAreaLight(r, a, 0.000001f, closest->t);
            if (hitRecord && hitRecord->t < closest->t) {
                closest = hitRecord;
                v = a.radiance;
            }
        }
        return { closest->t, v };
    }

    void PathTracerRenderer::buildPhotonMap() {
        photonMap = std::make_unique<PhotonMap>();
        std::mt19937 rng{std::random_device{ }()};
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        Vec3 emittedScene{0, 0, 0};
        Vec3 expectedScene{0, 0, 0};
        for (auto& a : scene.areaLightBuffer) {
            Vec3 nL = glm::normalize(glm::cross(a.u, a.v));
            float area = glm::length(glm::cross(a.u, a.v));
            Vec3 emittedLight{0, 0, 0};
            Vec3 expectedLight = a.radiance * area * 3.1415926535898f;
            for (int i=0; i<photonsPerLight; i++) {
                float us = dist(rng);
                float vs = dist(rng);
                Vec3 pos = a.position + us*a.u + vs*a.v;
                Vec3 dir = glm::normalize(toWorld(nL, sampleHemisphereCosine()));
                Vec3 power = a.radiance * area * 3.1415926535898f / float(photonsPerLight);
                emittedLight += power;
                Ray ray{pos + 0.0001f*nL, dir};
                for (int b=0; b<photonMaxDepth; b++) {
                    auto hit = closestHitObject(ray);
                    if (!hit) break;
                    auto& mtl = scene.materials[hit->material.index()];
                    using PW = Property::Wrapper;
                    Vec3 origin = hit->hitPoint + 0.0001f * hit->normal;
                    auto diffuseColor = mtl.getProperty<PW::RGBType>("diffuseColor");
                    auto reflectColor = mtl.getProperty<PW::RGBType>("reflect");
                    auto roughnessVal = mtl.getProperty<PW::FloatType>("roughness");
                    if (diffuseColor) {
                        Vec3 albedo = (*diffuseColor).value;
                        if (b > 0) photonMap->add(hit->hitPoint, power);
                        float p = glm::clamp(glm::max(albedo.x, glm::max(albedo.y, albedo.z)), 0.1f, 0.9f);
                        if (dist(rng) > p) break;
                        power *= albedo / p;
                        Vec3 d = toWorld(hit->normal, sampleHemisphereCosine());
                        ray = Ray{origin, glm::normalize(d)};
                    } else if (reflectColor) {
                        Vec3 reflect = (*reflectColor).value;
                        float rough = roughnessVal ? (*roughnessVal).value : 0.0f;
                        float p = glm::clamp(glm::max(reflect.x, glm::max(reflect.y, reflect.z)), 0.1f, 0.9f);
                        if (dist(rng) > p) break;
                        power *= reflect / p;
                        Vec3 rdir = glm::reflect(glm::normalize(ray.direction), glm::normalize(hit->normal));
                        if (rough > 0.0f) {
                            Vec3 jitter = toWorld(rdir, sampleHemisphereCosine());
                            rdir = glm::normalize(rdir + rough * jitter);
                        }
                        ray = Ray{origin, rdir};
                    } else {
                        break;
                    }
                }
            }
            emittedScene += emittedLight;
            expectedScene += expectedLight;
            std::cout << "[PhotonMap] Emitted(light) " << emittedLight << " | Expected " << expectedLight << std::endl;
        }
        photonMap->build();
        std::cout << "[PhotonMap] Emitted(scene) " << emittedScene << " | Expected(scene) " << expectedScene << std::endl;
    }

    RGB PathTracerRenderer::trace(const Ray& r, int currDepth) {
        if (currDepth >= depth) return Vec3{0};
        auto hitObject = closestHitObject(r);
        auto [tLight, emitted] = closestHitLight(r);
        if (hitObject && hitObject->t < tLight) {
            auto& mtl = scene.materials[hitObject->material.index()];
            using PW = Property::Wrapper;
            auto diffuseColor = mtl.getProperty<PW::RGBType>("diffuseColor");
            auto reflectColor = mtl.getProperty<PW::RGBType>("reflect");
            auto roughnessVal = mtl.getProperty<PW::FloatType>("roughness");

            Vec3 origin = hitObject->hitPoint + 0.0001f * hitObject->normal;

            if (reflectColor) {
                Vec3 reflect = (*reflectColor).value;
                Vec3 rdir = glm::reflect(glm::normalize(r.direction), glm::normalize(hitObject->normal));
                float rough = roughnessVal ? (*roughnessVal).value : 0.0f;
                if (rough > 0.0f) {
                    Vec3 jitter = toWorld(rdir, sampleHemisphereCosine());
                    rdir = glm::normalize(rdir + rough * jitter);
                }
                return reflect * trace(Ray{origin, rdir}, currDepth+1);
            }

            Vec3 albedo = diffuseColor ? (*diffuseColor).value : Vec3{1,1,1};

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

            Vec3 indirect{0, 0, 0};
            if (photonMap) {
                auto knn = photonMap->estimateKNN(origin, gatherK);
                Vec3 sumPower = knn.first;
                float r2 = knn.second;
                if (r2 > 0.0f) {
                    r2 = glm::max(r2, minGatherRadius2);
                    indirect = (albedo / 3.1415926535898f) * (sumPower / (3.1415926535898f * r2));
                }
            }
            return direct + indirect;
        } else if (tLight != FLOAT_INF) {
            return emitted;
        } else {
            return Vec3{0};
        }
    }
}