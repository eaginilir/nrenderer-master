#pragma once
#ifndef __ENVMAP_PATH_TRACER_HPP__
#define __ENVMAP_PATH_TRACER_HPP__

#include "scene/Scene.hpp"
#include "Ray.hpp"
#include "Camera.hpp"
#include "intersections/HitRecord.hpp"
#include "shaders/ShaderCreator.hpp"
#include "EnvironmentMap.hpp"

#include <tuple>

namespace EnvMapPathTracer
{
    using namespace NRenderer;
    using namespace std;

    class EnvMapPathTracerRenderer
    {
    private:
        SharedScene spScene;
        Scene& scene;

        unsigned int width;
        unsigned int height;
        unsigned int depth;
        unsigned int samples;

        using SCam = EnvMapPathTracer::Camera;
        SCam camera;

        vector<SharedShader> shaderPrograms;
        EnvironmentMap envMap;

    public:
        EnvMapPathTracerRenderer(SharedScene spScene)
            : spScene(spScene)
            , scene(*spScene)
            , camera(spScene->camera)
        {
            width = scene.renderOption.width;
            height = scene.renderOption.height;
            depth = scene.renderOption.depth;
            samples = scene.renderOption.samplesPerPixel;

            // 初始化环境贴图
            if (scene.ambient.type == Ambient::Type::ENVIROMENT_MAP &&
                scene.ambient.environmentMap.valid()) {
                envMap = EnvironmentMap(&scene.textures[scene.ambient.environmentMap.index()]);
            }
        }

        ~EnvMapPathTracerRenderer() = default;

        using RenderResult = tuple<RGBA*, unsigned int, unsigned int>;
        RenderResult render();
        void release(const RenderResult& r);

    private:
        void renderTask(RGBA* pixels, int width, int height, int off, int step);
        RGB gamma(const RGB& rgb);
        RGB trace(const Ray& ray, int currDepth);
        HitRecord closestHitObject(const Ray& r);
        tuple<float, Vec3> closestHitLight(const Ray& r);

        // 获取环境光照
        RGB getEnvironmentLight(const Vec3& direction) const {
            if (envMap.isValid()) {
                return envMap.sample(direction);
            }
            return scene.ambient.constant;
        }
    };
}

#endif
