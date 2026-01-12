// d:\study\computer_graph\nrenderer-master\code\components\ray_tracing\include\PathTracer.hpp
#pragma once
#ifndef __PATH_TRACER_HPP__
#define __PATH_TRACER_HPP__

#include "scene/Scene.hpp"
#include "Ray.hpp"
#include "Camera.hpp"
#include "intersections/intersections.hpp"
#include "VertexTransformer.hpp"
#include "KDTree.hpp"

#include <tuple>

namespace RayCast
{
    using namespace NRenderer;
    using namespace std;

    class PathTracerRenderer
    {
    private:
        SharedScene spScene;
        Scene& scene;

        unsigned int width;
        unsigned int height;
        unsigned int depth;
        unsigned int samples;

        Camera camera;
        unique_ptr<KDTree> accel;
    public:
        PathTracerRenderer(SharedScene spScene)
            : spScene               (spScene)
            , scene                 (*spScene)
            , camera                (spScene->camera)
        {
            width = scene.renderOption.width;
            height = scene.renderOption.height;
            depth = scene.renderOption.depth;
            samples = scene.renderOption.samplesPerPixel;
        }
        ~PathTracerRenderer() = default;

        using RenderResult = tuple<RGBA*, unsigned int, unsigned int>;
        RenderResult render();
        void release(const RenderResult& r);

    private:
        void renderTask(RGBA* pixels, int width, int height, int off, int step);
        RGB gamma(const RGB& rgb);
        RGB trace(const Ray& ray, int currDepth);
        HitRecord closestHitObject(const Ray& r);
        tuple<float, Vec3> closestHitLight(const Ray& r);

        Vec3 sampleHemisphereUniform() const;
        Vec3 toWorld(const Vec3& n, const Vec3& local) const;
    };
}

#endif