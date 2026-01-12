#pragma once
#ifndef __ENVIRONMENT_MAP_HPP__
#define __ENVIRONMENT_MAP_HPP__

#include "geometry/vec.hpp"
#include "scene/Texture.hpp"
#include <cmath>

namespace EnvMapPathTracer
{
    using namespace NRenderer;

    constexpr float ENV_PI = 3.14159265358979323846f;

    // 环境贴图类 - 支持球面映射的HDR环境光照
    class EnvironmentMap
    {
    private:
        const Texture* texture;
        bool valid;

    public:
        EnvironmentMap() : texture(nullptr), valid(false) {}

        EnvironmentMap(const Texture* tex) : texture(tex), valid(tex != nullptr && tex->rgba != nullptr) {}

        bool isValid() const { return valid; }

        // 根据方向向量采样环境贴图
        // 使用球面坐标映射: direction -> (theta, phi) -> (u, v)
        RGB sample(const Vec3& direction) const {
            if (!valid) return Vec3{0};

            // 归一化方向
            Vec3 d = glm::normalize(direction);

            // 球面坐标转换
            // theta: 与Y轴的夹角 [0, PI]
            // phi: 在XZ平面上的角度 [-PI, PI]
            float theta = acos(clamp(d.y, 1.0f, -1.0f));
            float phi = atan2(d.z, d.x);

            // 转换到UV坐标 [0, 1]
            float u = (phi + ENV_PI) / (2.0f * ENV_PI);
            float v = theta / ENV_PI;

            // 双线性插值采样
            float fx = u * (texture->width - 1);
            float fy = v * (texture->height - 1);

            int x0 = (int)fx;
            int y0 = (int)fy;
            int x1 = min(x0 + 1, (int)texture->width - 1);
            int y1 = min(y0 + 1, (int)texture->height - 1);

            float dx = fx - x0;
            float dy = fy - y0;

            // 获取四个相邻像素
            RGBA c00 = texture->rgba[y0 * texture->width + x0];
            RGBA c10 = texture->rgba[y0 * texture->width + x1];
            RGBA c01 = texture->rgba[y1 * texture->width + x0];
            RGBA c11 = texture->rgba[y1 * texture->width + x1];

            // 双线性插值
            Vec3 color = Vec3(c00) * (1 - dx) * (1 - dy)
                       + Vec3(c10) * dx * (1 - dy)
                       + Vec3(c01) * (1 - dx) * dy
                       + Vec3(c11) * dx * dy;

            return color;
        }
    };
}

#endif
