#pragma once
#ifndef __ENVMAP_BVH_HPP__
#define __ENVMAP_BVH_HPP__

#include "AABB.hpp"
#include "intersections/HitRecord.hpp"
#include "scene/Scene.hpp"
#include <vector>
#include <memory>

namespace EnvMapPathTracer
{
    using namespace NRenderer;
    using namespace std;

    // 图元类型
    enum class PrimitiveType { TRIANGLE, SPHERE };

    // 图元引用
    struct Primitive {
        PrimitiveType type;
        size_t index;
        AABB bounds;
    };

    // BVH 节点
    struct BVHNode {
        AABB bounds;
        int left = -1;   // 左子节点索引，-1 表示叶子
        int right = -1;  // 右子节点索引
        int primStart = 0;
        int primCount = 0;

        bool isLeaf() const { return left == -1; }
    };

    class BVH {
    private:
        vector<BVHNode> nodes;
        vector<Primitive> primitives;
        const Scene* scene = nullptr;

    public:
        BVH() = default;

        void build(const Scene& scn);
        HitRecord intersect(const Ray& ray, float tMin, float tMax) const;

    private:
        int buildRecursive(int start, int end);
        AABB computeBounds(const Triangle& t) const;
        AABB computeBounds(const Sphere& s) const;
        HitRecord intersectPrimitive(const Primitive& prim, const Ray& ray, float tMin, float tMax) const;
    };
}

#endif
