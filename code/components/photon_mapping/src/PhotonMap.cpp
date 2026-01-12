#include "PhotonMap.hpp"
#include "glm/glm.hpp"

namespace RayCast {
using namespace NRenderer;

void PhotonMap::clear() {
    photons.clear();
    root.reset();
}

void PhotonMap::reserve(size_t n) {
    photons.reserve(n);
}

void PhotonMap::add(const Vec3& pos, const Vec3& power) {
    photons.push_back(Photon{pos, power});
}

static float dist2(const Vec3& a, const Vec3& b) {
    auto d = a - b;
    return glm::dot(d, d);
}

std::unique_ptr<PhotonMap::Node> PhotonMap::buildRec(std::vector<int>& idx, int depth) {
    if (idx.empty()) return nullptr;
    int axis = depth % 3;
    auto midIt = idx.begin() + idx.size()/2;
    std::nth_element(idx.begin(), midIt, idx.end(), [&](int i, int j) {
        return (axis==0?photons[i].position.x:(axis==1?photons[i].position.y:photons[i].position.z))
             < (axis==0?photons[j].position.x:(axis==1?photons[j].position.y:photons[j].position.z));
    });
    int mid = *midIt;
    std::vector<int> left(idx.begin(), midIt);
    std::vector<int> right(midIt+1, idx.end());
    auto node = std::make_unique<Node>();
    node->index = mid;
    node->axis = axis;
    node->pos = photons[mid].position;
    node->left = buildRec(left, depth+1);
    node->right = buildRec(right, depth+1);
    return node;
}

void PhotonMap::build() {
    std::vector<int> idx(photons.size());
    for (int i=0; i<(int)photons.size(); ++i) idx[i] = i;
    root = buildRec(idx, 0);
}

void PhotonMap::knnRec(const Node* node, const Vec3& x, int k,
                       std::priority_queue<std::pair<float,int>>& heap) const {
    if (!node) return;
    float d2 = dist2(node->pos, x);
    if ((int)heap.size() < k) heap.emplace(d2, node->index);
    else if (d2 < heap.top().first) {
        heap.pop();
        heap.emplace(d2, node->index);
    }
    int axis = node->axis;
    float diff = (axis==0? x.x - node->pos.x : (axis==1? x.y - node->pos.y : x.z - node->pos.z));
    const Node* nearNode = diff < 0 ? node->left.get() : node->right.get();
    const Node* farNode  = diff < 0 ? node->right.get() : node->left.get();
    knnRec(nearNode, x, k, heap);
    if ((int)heap.size() < k || diff*diff < heap.top().first) {
        knnRec(farNode, x, k, heap);
    }
}

std::pair<Vec3, float> PhotonMap::estimateKNN(const Vec3& x, int k) const {
    std::priority_queue<std::pair<float,int>> heap;
    knnRec(root.get(), x, k, heap);
    Vec3 sum{0};
    float r2 = 0.f;
    while (!heap.empty()) {
        auto [d2, idx] = heap.top();
        heap.pop();
        sum += photons[idx].power;
        if (d2 > r2) r2 = d2;
    }
    return {sum, r2};
}
const std::vector<Photon>& PhotonMap::getPhotons() const {
    return photons;
}
}