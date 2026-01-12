#pragma once
#ifndef __RT_OCTREE_HPP__
#define __RT_OCTREE_HPP__

#include <array>
#include <functional>
#include <limits>
#include <memory>
#include <stack>
#include <vector>

#include "geometry/vec.hpp"
#include "intersections/intersections.hpp" // 提供 Ray / HitRecord

namespace RayTracing {
using namespace NRenderer;

struct AABB {
    Vec3 min;
    Vec3 max;
    AABB()
        : min{+std::numeric_limits<float>::infinity()}
        , max{-std::numeric_limits<float>::infinity()} {}
    AABB(const Vec3 &mn, const Vec3 &mx) : min(mn), max(mx) {}
    void expand(const Vec3 &p) {
        min = glm::min(min, p);
        max = glm::max(max, p);
    }
    void expand(const AABB &b) {
        min = glm::min(min, b.min);
        max = glm::max(max, b.max);
    }
    Vec3 center() const { return 0.5f * (min + max); }
    bool valid() const { return min.x <= max.x && min.y <= max.y && min.z <= max.z; }

    static AABB fromSphere(const Vec3 &c, float r) {
        Vec3 rr{r, r, r};
        return AABB{c - rr, c + rr};
    }
    static AABB fromTriangle(const Vec3 &a, const Vec3 &b, const Vec3 &c) {
        AABB box;
        box.expand(a);
        box.expand(b);
        box.expand(c);
        return box;
    }
};

class Octree {
public:
    using IntersectPrim = std::function<HitRecord(size_t idx, const Ray &r, float tMin, float tMax)>;

    void build(const std::vector<AABB> &primBounds, int maxDepth = 12, int minPrims = 4);

    HitRecord intersectClosest(const Ray &r, float tMin, float tMax, const IntersectPrim &isect) const;

    const std::vector<AABB> &bounds() const { return primBounds_; }

    // 使用示例：
    // 1) 收集每个原始体的包围盒，构造 primBounds。
    // 2) 调用 build(primBounds)。
    // 3) 在光线求交时：
    //    auto result = octree.intersectClosest(ray, 0.01f, INF,
    //        [&](size_t packedIdx, const Ray& r, float tMin, float tMax){
    //            // 根据 packedIdx 查回实际原始体，调用几何求交并返回 HitRecord。
    //        });

private:
    struct Node {
        AABB box;
        int firstChild = -1;          // 子节点起始索引（连续 8 个）
        std::vector<size_t> prims;    // 叶子中的原始体索引
        bool leaf = true;
    };

    static std::array<AABB, 8> split8(const AABB &b);

    void buildNode(int nodeId, const std::vector<size_t> &prims, int depth, int maxDepth, int minPrims);

    static bool hitAABB(const AABB &b, const Ray &r, float tMin, float tMax);

    std::vector<Node> nodes_;
    std::vector<AABB> primBounds_;
};

} // namespace RayTracing

#endif
