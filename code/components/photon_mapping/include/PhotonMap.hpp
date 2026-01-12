#pragma once
#ifndef __PHOTON_MAP_HPP__
#define __PHOTON_MAP_HPP__

#include <vector>
#include <memory>
#include <queue>
#include <algorithm>
#include "geometry/vec.hpp"

namespace RayCast {
using namespace NRenderer;

struct Photon {
    Vec3 position;
    Vec3 power;
};

class PhotonMap {
public:
    PhotonMap() = default;
    void clear();
    void reserve(size_t n);
    void add(const Vec3& pos, const Vec3& power);
    void build();
    std::pair<Vec3, float> estimateKNN(const Vec3& x, int k) const;
    const std::vector<Photon>& getPhotons() const;

private:
    struct Node {
        int index;
        int axis;
        std::unique_ptr<Node> left;
        std::unique_ptr<Node> right;
        Vec3 pos;
    };
    std::unique_ptr<Node> root;
    std::vector<Photon> photons;

    std::unique_ptr<Node> buildRec(std::vector<int>& idx, int depth);
    void knnRec(const Node* node, const Vec3& x, int k,
                std::priority_queue<std::pair<float,int>>& heap) const;
};
}

#endif