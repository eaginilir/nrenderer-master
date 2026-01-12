#include "accel/Octree.hpp"

namespace RayTracing {
using namespace NRenderer;

std::array<AABB, 8> Octree::split8(const AABB &b) {
    std::array<AABB, 8> out;
    Vec3 c = b.center();
    const Vec3 &mn = b.min;
    const Vec3 &mx = b.max;
    out[0] = AABB({mn.x, mn.y, mn.z}, {c.x, c.y, c.z});
    out[1] = AABB({c.x, mn.y, mn.z}, {mx.x, c.y, c.z});
    out[2] = AABB({mn.x, c.y, mn.z}, {c.x, mx.y, c.z});
    out[3] = AABB({c.x, c.y, mn.z}, {mx.x, mx.y, c.z});
    out[4] = AABB({mn.x, mn.y, c.z}, {c.x, c.y, mx.z});
    out[5] = AABB({c.x, mn.y, c.z}, {mx.x, c.y, mx.z});
    out[6] = AABB({mn.x, c.y, c.z}, {c.x, mx.y, mx.z});
    out[7] = AABB({c.x, c.y, c.z}, {mx.x, mx.y, mx.z});
    return out;
}

void Octree::build(const std::vector<AABB> &primBounds, int maxDepth, int minPrims) {
    primBounds_ = primBounds;
    nodes_.clear();
    nodes_.reserve(1024);

    AABB root;
    for (const auto &b : primBounds_) root.expand(b);

    nodes_.push_back(Node{root, -1, {}, true});
    std::vector<size_t> all(primBounds_.size());
    for (size_t i = 0; i < all.size(); ++i) all[i] = i;

    buildNode(0, all, 0, maxDepth, minPrims);
}

void Octree::buildNode(int nodeId, const std::vector<size_t> &prims, int depth, int maxDepth, int minPrims) {
    Node &node = nodes_[nodeId];
    if (depth >= maxDepth || prims.size() <= static_cast<size_t>(minPrims)) {
        node.leaf = true;
        node.prims = prims;
        return;
    }

    auto children = split8(node.box);
    std::array<std::vector<size_t>, 8> childPrims;
    for (size_t pi : prims) {
        const AABB &pb = primBounds_[pi];
        for (int ci = 0; ci < 8; ++ci) {
            const AABB &cb = children[ci];
            if (!(pb.max.x < cb.min.x || pb.min.x > cb.max.x ||
                  pb.max.y < cb.min.y || pb.min.y > cb.max.y ||
                  pb.max.z < cb.min.z || pb.min.z > cb.max.z)) {
                childPrims[ci].push_back(pi);
            }
        }
    }

    size_t nonEmpty = 0;
    for (int i = 0; i < 8; ++i)
        if (!childPrims[i].empty()) ++nonEmpty;
    if (nonEmpty == 0 || nonEmpty == 1) {
        node.leaf = true;
        node.prims = prims;
        return;
    }

    node.leaf = false;
    node.firstChild = static_cast<int>(nodes_.size());
    nodes_.resize(nodes_.size() + 8);
    for (int i = 0; i < 8; ++i) {
        Node &cnode = nodes_[node.firstChild + i];
        cnode.box = children[i];
        cnode.firstChild = -1;
        cnode.leaf = true;
        if (!childPrims[i].empty()) {
            buildNode(node.firstChild + i, childPrims[i], depth + 1, maxDepth, minPrims);
        } else {
            cnode.leaf = true;
            cnode.prims.clear();
        }
    }
}

bool Octree::hitAABB(const AABB &b, const Ray &r, float tMin, float tMax) {
    for (int a = 0; a < 3; ++a) {
        float invD = 1.0f / r.direction[a];
        float t0 = (b.min[a] - r.origin[a]) * invD;
        float t1 = (b.max[a] - r.origin[a]) * invD;
        if (invD < 0.0f) std::swap(t0, t1);
        tMin = t0 > tMin ? t0 : tMin;
        tMax = t1 < tMax ? t1 : tMax;
        if (tMax <= tMin) return false;
    }
    return true;
}

HitRecord Octree::intersectClosest(const Ray &r, float tMin, float tMax, const IntersectPrim &isect) const {
    HitRecord best = std::nullopt;
    float closest = tMax;

    if (nodes_.empty()) return best;

    std::stack<int> st;
    st.push(0);

    while (!st.empty()) {
        int id = st.top();
        st.pop();
        const Node &n = nodes_[id];
        if (!hitAABB(n.box, r, tMin, closest)) continue;

        if (n.leaf) {
            for (size_t pi : n.prims) {
                auto h = isect(pi, r, tMin, closest);
                if (h && h->t < closest) {
                    closest = h->t;
                    best = h;
                }
            }
        } else {
            for (int i = 0; i < 8; ++i) st.push(n.firstChild + i);
        }
    }
    return best;
}

} // namespace RayTracing
