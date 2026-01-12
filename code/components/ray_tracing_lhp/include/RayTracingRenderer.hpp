#pragma once
#ifndef __RAY_TRACING_HPP__
#define __RAY_TRACING_HPP__

#include "scene/Scene.hpp"

#include "Camera.hpp"
#include "intersections/intersections.hpp"

#include "RTMaterials/RTMaterialsCreator.hpp"
// 可选：用于后续加速的八叉树与 AABB
#include "accel/Octree.hpp"

namespace RayTracing
{
    using namespace NRenderer;
    class RayTracingRenderer
    {
    private:
        SharedScene spScene;
        Scene& scene;
        RayTracing::Camera camera;

        vector<SRTMat> materialsPrograms;

        unsigned int maxDepth;
        // 八叉树与原始体索引映射
        std::unique_ptr<Octree> octree_;
        std::vector<std::pair<int, Index>> octPrimLUT_;
    public:
        RayTracingRenderer(SharedScene spScene)
            : spScene               (spScene)
            , scene                 (*spScene)
            , camera                (spScene->camera)
            , maxDepth               (spScene->renderOption.depth)
        {}
        ~RayTracingRenderer() = default;

        using RenderResult = tuple<RGBA*, unsigned int, unsigned int>;
        RenderResult render();
        void release(const RenderResult& r);

    private:
        RGB gamma(const RGB& rgb);
        RGB raytracing(const unsigned int x, const unsigned int y);
        RGB traceRay(const Ray& r, unsigned int depth);
        HitRecord closestHit(const Ray& r);
        bool scatter(const Ray &in, const HitRecord &rec, Vec3& attenuation, Ray &scattered);
        // 构建场景原始体的 AABB，并初始化八叉树（可选）
        void buildAABBScene();
    };
}

#endif