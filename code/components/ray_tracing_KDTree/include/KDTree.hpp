// cpp d:\study\computer_graph\nrenderer-master\code\components\ray_tracing_KDTree\include\KDTree.hpp
#pragma once
#ifndef __KDTREE_HPP__
#define __KDTREE_HPP__

#include <vector>
#include <memory>
#include <algorithm>

#include "scene/Scene.hpp"
#include "Ray.hpp"
#include "intersections/intersections.hpp"
#include "intersections/HitRecord.hpp"

namespace RayCast
{
    using namespace NRenderer;

    class KDTree
    {
    public:
        KDTree() = default;
        void setLeafSize(int s);
        void buildFromScene(const Scene& scene);
        HitRecord closestHit(const Ray& ray, float tMin, float tMax) const;

    private:
        struct Tri {
            Vec3 v1, v2, v3;
            Vec3 n1, n2, n3;
            Vec3 normal;
            Handle material;
        };
        struct AABB {
            Vec3 min, max;
        };
        struct Node {
            AABB box;
            std::unique_ptr<Node> left;
            std::unique_ptr<Node> right;
            std::vector<int> indices;
            bool isLeaf() const { return !left && !right; }
        };

        std::unique_ptr<Node> root;
        std::vector<Tri> tris;
        int leafSize = 8;

        static AABB triBox(const Tri& t);
        static AABB merge(const AABB& a, const AABB& b);
        static bool hitAABB(const Ray& r, const AABB& box, float tMin, float tMax);
        static bool hitAABBWithT(const Ray& r, const AABB& box, float tMin, float tMax, float& tNear);
        static Vec3 centroid(const Tri& t);
        std::unique_ptr<Node> build(const std::vector<int>& idx);
        HitRecord traverse(const Node* node, const Ray& r, float tMin, float tMax) const;
    };
}

#endif