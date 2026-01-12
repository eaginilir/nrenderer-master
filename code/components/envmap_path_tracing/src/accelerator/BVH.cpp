#include "accelerator/BVH.hpp"
#include "intersections/intersections.hpp"
#include <algorithm>

namespace EnvMapPathTracer
{
    AABB BVH::computeBounds(const Triangle& t) const {
        AABB box;
        box.expand(t.v1);
        box.expand(t.v2);
        box.expand(t.v3);
        return box;
    }

    AABB BVH::computeBounds(const Sphere& s) const {
        Vec3 r{s.radius, s.radius, s.radius};
        return AABB(s.position - r, s.position + r);
    }

    void BVH::build(const Scene& scn) {
        scene = &scn;
        primitives.clear();
        nodes.clear();

        // 收集所有图元
        for (size_t i = 0; i < scn.triangleBuffer.size(); i++) {
            Primitive p;
            p.type = PrimitiveType::TRIANGLE;
            p.index = i;
            p.bounds = computeBounds(scn.triangleBuffer[i]);
            primitives.push_back(p);
        }
        for (size_t i = 0; i < scn.sphereBuffer.size(); i++) {
            Primitive p;
            p.type = PrimitiveType::SPHERE;
            p.index = i;
            p.bounds = computeBounds(scn.sphereBuffer[i]);
            primitives.push_back(p);
        }

        if (primitives.empty()) return;

        nodes.reserve(primitives.size() * 2);
        buildRecursive(0, primitives.size());
    }

    int BVH::buildRecursive(int start, int end) {
        int nodeIdx = nodes.size();
        nodes.push_back(BVHNode{});
        BVHNode& node = nodes[nodeIdx];

        // 计算边界
        for (int i = start; i < end; i++) {
            node.bounds.expand(primitives[i].bounds);
        }

        int count = end - start;
        if (count <= 4) {
            // 叶子节点
            node.primStart = start;
            node.primCount = count;
            return nodeIdx;
        }

        // 选择最长轴分割
        int axis = node.bounds.longestAxis();
        int mid = (start + end) / 2;

        // 按质心排序
        std::nth_element(primitives.begin() + start, primitives.begin() + mid,
            primitives.begin() + end,
            [axis](const Primitive& a, const Primitive& b) {
                return a.bounds.centroid()[axis] < b.bounds.centroid()[axis];
            });

        node.left = buildRecursive(start, mid);
        node.right = buildRecursive(mid, end);
        return nodeIdx;
    }

    HitRecord BVH::intersectPrimitive(const Primitive& prim, const Ray& ray, float tMin, float tMax) const {
        if (prim.type == PrimitiveType::TRIANGLE) {
            return Intersection::xTriangle(ray, scene->triangleBuffer[prim.index], tMin, tMax);
        } else {
            return Intersection::xSphere(ray, scene->sphereBuffer[prim.index], tMin, tMax);
        }
    }

    HitRecord BVH::intersect(const Ray& ray, float tMin, float tMax) const {
        if (nodes.empty()) return nullopt;

        HitRecord closest = nullopt;
        float closestT = tMax;

        // 栈式遍历
        int stack[64];
        int stackPtr = 0;
        stack[stackPtr++] = 0;

        while (stackPtr > 0) {
            int idx = stack[--stackPtr];
            const BVHNode& node = nodes[idx];

            if (!node.bounds.hit(ray, tMin, closestT)) continue;

            if (node.isLeaf()) {
                for (int i = 0; i < node.primCount; i++) {
                    auto hit = intersectPrimitive(primitives[node.primStart + i], ray, tMin, closestT);
                    if (hit && hit->t < closestT) {
                        closestT = hit->t;
                        closest = hit;
                    }
                }
            } else {
                stack[stackPtr++] = node.left;
                stack[stackPtr++] = node.right;
            }
        }

        return closest;
    }
}
