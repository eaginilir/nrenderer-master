#pragma once
#ifndef __HIT_RECORD_HPP__
#define __HIT_RECORD_HPP__

#include <optional>
#include <array>

#include "geometry/vec.hpp"

namespace RayTracing
{
    using namespace NRenderer;
    using namespace std;
    struct HitRecordBase
    {
        float t;        // 射线参数t值
        Vec3 hitPoint;  // 交点坐标
        Vec3 normal;    // 法线
        Handle material;// 材质句柄

        bool hasVertexData = false;
        array<Vec3, 3> vertices = {};
        array<Vec3, 3> normals = {};
    };
    using HitRecord = optional<HitRecordBase>;
    inline
    HitRecord getMissRecord() {
        return nullopt;
    }

    inline
    HitRecord getHitRecord(float t, const Vec3& hitPoint, const Vec3& normal, Handle material) {
        return make_optional<HitRecordBase>(t, hitPoint, normal, material);
    }

    // 新增：带顶点数据的HitRecord（用于Gouraud着色）
    inline
    HitRecord getHitRecordWithVertices(float t, const Vec3& hitPoint, const Vec3& normal, Handle material,const Vec3& v1, const Vec3& v2, const Vec3& v3,const Vec3& n1, const Vec3& n2, const Vec3& n3) {
        HitRecordBase rec;
        rec.t = t;
        rec.hitPoint = hitPoint;
        rec.normal = normal;
        rec.material = material;
        rec.hasVertexData = true;
        rec.vertices = {v1, v2, v3};
        rec.normals = {n1, n2, n3};
        return make_optional<HitRecordBase>(rec);
    }
}

#endif