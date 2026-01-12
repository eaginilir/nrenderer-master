// cpp d:\study\computer_graph\nrenderer-master\code\components\ray_tracing_KDTree\src\KDTree.cpp
#include "KDTree.hpp"

namespace RayCast
{
    void KDTree::setLeafSize(int s) { leafSize = std::max(1, s); }
    KDTree::AABB KDTree::triBox(const Tri& t) {
        Vec3 mn{
            std::min({t.v1.x, t.v2.x, t.v3.x}),
            std::min({t.v1.y, t.v2.y, t.v3.y}),
            std::min({t.v1.z, t.v2.z, t.v3.z})
        };
        Vec3 mx{
            std::max({t.v1.x, t.v2.x, t.v3.x}),
            std::max({t.v1.y, t.v2.y, t.v3.y}),
            std::max({t.v1.z, t.v2.z, t.v3.z})
        };
        return {mn, mx};
    }
    KDTree::AABB KDTree::merge(const AABB& a, const AABB& b) {
        Vec3 mn{std::min(a.min.x,b.min.x), std::min(a.min.y,b.min.y), std::min(a.min.z,b.min.z)};
        Vec3 mx{std::max(a.max.x,b.max.x), std::max(a.max.y,b.max.y), std::max(a.max.z,b.max.z)};
        return {mn, mx};
    }
    bool KDTree::hitAABB(const Ray& r, const AABB& box, float tMin, float tMax) {
        for (int i=0;i<3;i++) {
            float invD = 1.0f / (i==0?r.direction.x:(i==1?r.direction.y:r.direction.z));
            float o = (i==0?r.origin.x:(i==1?r.origin.y:r.origin.z));
            float t0 = ( (i==0?box.min.x:(i==1?box.min.y:box.min.z)) - o ) * invD;
            float t1 = ( (i==0?box.max.x:(i==1?box.max.y:box.max.z)) - o ) * invD;
            if (invD < 0.0f) std::swap(t0, t1);
            tMin = t0 > tMin ? t0 : tMin;
            tMax = t1 < tMax ? t1 : tMax;
            if (tMax <= tMin) return false;
        }
        return true;
    }
    bool KDTree::hitAABBWithT(const Ray& r, const AABB& box, float tMin, float tMax, float& tNear) {
        float tn = tMin;
        float tf = tMax;
        for (int i=0;i<3;i++) {
            float invD = 1.0f / (i==0?r.direction.x:(i==1?r.direction.y:r.direction.z));
            float o = (i==0?r.origin.x:(i==1?r.origin.y:r.origin.z));
            float t0 = ( (i==0?box.min.x:(i==1?box.min.y:box.min.z)) - o ) * invD;
            float t1 = ( (i==0?box.max.x:(i==1?box.max.y:box.max.z)) - o ) * invD;
            if (invD < 0.0f) std::swap(t0, t1);
            tn = t0 > tn ? t0 : tn;
            tf = t1 < tf ? t1 : tf;
            if (tf <= tn) return false;
        }
        tNear = tn;
        return true;
    }
    Vec3 KDTree::centroid(const Tri& t) {
        return (t.v1 + t.v2 + t.v3) / 3.0f;
    }

    std::unique_ptr<KDTree::Node> KDTree::build(const std::vector<int>& idx) {
        if (idx.empty()) return nullptr;
        auto node = std::make_unique<Node>();
        AABB box = triBox(tris[idx[0]]);
        for (size_t i=1;i<idx.size();i++) box = merge(box, triBox(tris[idx[i]]));
        node->box = box;
        if ((int)idx.size() <= leafSize) {
            node->indices = idx;
            return std::move(node);
        }
        Vec3 mn = box.min, mx = box.max;
        Vec3 extent = mx - mn;
        int axis = 0;
        if (extent.y > extent.x && extent.y >= extent.z) axis = 1;
        else if (extent.z > extent.x && extent.z >= extent.y) axis = 2;
        std::vector<int> leftIdx, rightIdx;
        std::vector<float> cs;
        cs.reserve(idx.size());
        for (int i : idx) {
            Vec3 c = centroid(tris[i]);
            cs.push_back(axis==0?c.x:(axis==1?c.y:c.z));
        }
        float median;
        {
            std::vector<float> tmp = cs;
            size_t mid = tmp.size()/2;
            std::nth_element(tmp.begin(), tmp.begin()+mid, tmp.end());
            median = tmp[mid];
        }
        for (size_t k=0;k<idx.size();k++) {
            int i = idx[k];
            float v = cs[k];
            if (v <= median) leftIdx.push_back(i);
            else rightIdx.push_back(i);
        }
        if (leftIdx.empty() || rightIdx.empty()) {
            node->indices = idx;
            return std::move(node);
        }
        node->left = build(leftIdx);
        node->right = build(rightIdx);
        return std::move(node);
    }

    void KDTree::buildFromScene(const Scene& scene) {
        tris.clear();
        for (auto& t : scene.triangleBuffer) {
            Tri tr;
            tr.v1 = t.v1; tr.v2 = t.v2; tr.v3 = t.v3;
            tr.n1 = t.n1; tr.n2 = t.n2; tr.n3 = t.n3;
            tr.normal = glm::normalize(t.normal);
            tr.material = t.material;
            tris.push_back(tr);
        }
        for (auto& m : scene.meshBuffer) {
            for (size_t i = 0; i + 2 < m.positionIndices.size(); i += 3) {
                Tri tr;
                tr.v1 = m.positions[m.positionIndices[i]];
                tr.v2 = m.positions[m.positionIndices[i + 1]];
                tr.v3 = m.positions[m.positionIndices[i + 2]];
                Vec3 e1 = tr.v2 - tr.v1;
                Vec3 e2 = tr.v3 - tr.v1;
                tr.normal = glm::normalize(glm::cross(e1, e2));
                if (m.hasNormal() && (m.normalIndices.size() == m.positionIndices.size())) {
                    tr.n1 = m.normals[m.normalIndices[i]];
                    tr.n2 = m.normals[m.normalIndices[i + 1]];
                    tr.n3 = m.normals[m.normalIndices[i + 2]];
                } else {
                    tr.n1 = tr.n2 = tr.n3 = tr.normal;
                }
                tr.material = m.material;
                tris.push_back(tr);
            }
        }
        std::vector<int> idx(tris.size());
        for (size_t i=0;i<idx.size();i++) idx[i] = (int)i;
        root = build(idx);
    }

    HitRecord KDTree::traverse(const Node* node, const Ray& r, float tMin, float tMax) const {
        if (!node) return getMissRecord();
        if (!hitAABB(r, node->box, tMin, tMax)) return getMissRecord();
        HitRecord best = getMissRecord();
        float closest = tMax;
        if (node->isLeaf()) {
            for (int id : node->indices) {
                const Tri& tr = tris[id];
                Triangle t;
                t.v1 = tr.v1; t.v2 = tr.v2; t.v3 = tr.v3;
                t.normal = tr.normal;
                t.n1 = tr.n1; t.n2 = tr.n2; t.n3 = tr.n3;
                t.material = tr.material;
                auto hr = Intersection::xTriangle(r, t, tMin, closest);
                if (hr && hr->t < closest) { closest = hr->t; best = hr; }
            }
            return best;
        }
        float tNearL = 0.0f, tNearR = 0.0f;
        bool hitL = node->left && hitAABBWithT(r, node->left->box, tMin, closest, tNearL);
        bool hitR = node->right && hitAABBWithT(r, node->right->box, tMin, closest, tNearR);
        auto visit = [&](const Node* child){
            auto h = traverse(child, r, tMin, closest);
            if (h && h->t < closest) { closest = h->t; best = h; }
        };
        if (hitL && hitR) {
            if (tNearL <= tNearR) {
                visit(node->left.get());
                hitR = hitAABBWithT(r, node->right->box, tMin, closest, tNearR);
                if (hitR) visit(node->right.get());
            } else {
                visit(node->right.get());
                hitL = hitAABBWithT(r, node->left->box, tMin, closest, tNearL);
                if (hitL) visit(node->left.get());
            }
        } else if (hitL) {
            visit(node->left.get());
        } else if (hitR) {
            visit(node->right.get());
        }
        return best;
    }

    HitRecord KDTree::closestHit(const Ray& ray, float tMin, float tMax) const {
        return traverse(root.get(), ray, tMin, tMax);
    }
}