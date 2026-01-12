#include "RayTracingRenderer.hpp"

#include "VertexTransformer.hpp"
#include "intersections/intersections.hpp"
// 加速结构：Octree（可按需在 renderer 中使用）
#include "accel/Octree.hpp"
namespace RayTracing
{
    // 释放渲染结果
    void RayTracingRenderer::release(const RenderResult& r) {
        auto [p, w, h] = r;
        delete[] p;
    }
    // 伽马校正
    RGB RayTracingRenderer::gamma(const RGB& rgb) {
        return glm::sqrt(rgb);
    }
    // 渲染主函数
    auto RayTracingRenderer::render() -> RenderResult {
        auto width = scene.renderOption.width;
        auto height = scene.renderOption.height;
        auto pixels = new RGBA[width*height];

        VertexTransformer vertexTransformer{};
        vertexTransformer.exec(spScene);

        // 构建场景 AABB（并可同时构建八叉树）
        buildAABBScene();

        RTMaterialsCreator materialCreator{};
        // 为每种材质创建f
        materialsPrograms.clear();
        for (auto& mtl : scene.materials) {
            materialsPrograms.push_back(materialCreator.create(mtl, scene.textures));
        }

        for (int i=0; i<height; i++) {
            for (int j=0; j < width; j++) {
                auto color = raytracing(j, i);
                color = clamp(color);
                color = gamma(color);
                pixels[(height-i-1)*width+j] = {color, 1};
            }
        }

        return {pixels, width, height};
    }

    // f返回一个0到1之间的随机浮点数
    // float randomFloat() {
    //     return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    // }

    // bool scatter(const Ray &in, const HitRecord &rec, Vector3D &attenuation, Ray &scattered){
        
    // }

    /**
     * 递归追踪射线
     * @param r 射线
     * @param depth 当前剩余递归深度，当depth为0时停止递归
     * @return 射线颜色
     */
    RGB RayTracingRenderer::traceRay(const Ray& r, unsigned int depth) {
        if (depth == 0) {
            return RGB{0};
        }
        // 寻找最近交点
        auto closestHitObj = closestHit(r);
        // r如果j击中的物体
        if (closestHitObj) {
            auto& hitRec = *closestHitObj;

            if(hitRec.material.valid() == false) {
                return RGB{0};
            }
            
            // 如果是光源，直接返回光源颜色
            for(auto& light: scene.pointLightBuffer) {
                if(hitRec.hitPoint == light.position) {
                    return light.intensity;
                }
            }

            // 计算镜面反射光线的方向
            Vec3 attenuation;
            Ray scattered;
            bool success = materialsPrograms[hitRec.material.index()]->scatter(r, hitRec, attenuation, scattered);
            if (!success) {
                return RGB{0};
            }
            // 递归追踪反射光线
            Ray scatteredRay{hitRec.hitPoint, glm::normalize(scattered.direction)};
            return attenuation * traceRay(scatteredRay, depth - 1);
        }
        // 这里不考虑没有击中的情况,因为scn文件里面是封闭的,并且没有提供背景色
        else {
            // 背景色渐变
            Vec3 unitDirection = glm::normalize(r.direction);
            float t = 0.5f * (unitDirection.y + 1.0f);
            return (1.0f - t) * RGB{1.0f, 1.0f, 1.0f} + t * RGB{0.5f, 0.7f, 1.0f};
        }
    }

    RGB RayTracingRenderer::raytracing(const unsigned int x, const unsigned int y) {
        Ray r = camera.shoot(float(x)/float(scene.renderOption.width), float(y)/float(scene.renderOption.height));
        RGB finalColor{0};
        for (unsigned int s = 0; s < scene.renderOption.samplesPerPixel; s++) {
            Ray sampledRay = r;
            // 进行随机采样以实现抗锯齿
            float u = (float(x) + randomFloat()) / float(scene.renderOption.width);
            float v = (float(y) + randomFloat()) / float(scene.renderOption.height);
            sampledRay = camera.shoot(u, v);
            finalColor += traceRay(sampledRay, scene.renderOption.depth);
        }
        // 对颜色进行平均,多次采样平滑
        finalColor /= float(scene.renderOption.samplesPerPixel);
        return finalColor;
    }

    // 寻找射线与场景中物体的最近交点
    HitRecord RayTracingRenderer::closestHit(const Ray& r) {
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

    // 构建场景原始体 AABB，并初始化八叉树（可选）
    void RayTracingRenderer::buildAABBScene() {
        std::vector<AABB> primBounds;
        octPrimLUT_.clear();

        // Spheres
        for (Index i = 0; i < scene.sphereBuffer.size(); ++i) {
            const auto& s = scene.sphereBuffer[i];
            primBounds.emplace_back(AABB::fromSphere(s.position, s.radius));
            octPrimLUT_.emplace_back(0, i);
        }
        // Triangles
        for (Index i = 0; i < scene.triangleBuffer.size(); ++i) {
            const auto& t = scene.triangleBuffer[i];
            primBounds.emplace_back(AABB::fromTriangle(t.v1, t.v2, t.v3));
            octPrimLUT_.emplace_back(1, i);
        }
        // Meshes（以整体 AABB 近似；若需更细，可拆为三角）
        for (Index i = 0; i < scene.meshBuffer.size(); ++i) {
            const auto& m = scene.meshBuffer[i];
            if (m.positions.empty()) continue;
            AABB box;
            for (const auto& p : m.positions) box.expand(p);
            primBounds.emplace_back(box);
            octPrimLUT_.emplace_back(2, i);
        }

        // Plane 为无限体，不加入树

        if (!primBounds.empty()) {
            if (!octree_) octree_ = std::make_unique<Octree>();
            octree_->build(primBounds, 12, 4);
        }
    }


}