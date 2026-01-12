#pragma once
#ifndef __NR_SCN_IMPORTER_HPP__
#define __NR_SCN_IMPORTER_HPP__

#include "Importer.hpp"
#include <map>

namespace NRenderer
{
    using namespace std;
    class ScnImporter: public Importer
    {
    private:
        bool parseMtl(Asset& asset, ifstream& file, map<string, size_t>& mtlMap);
        bool parseMdl(Asset& asset, ifstream& file, map<string, size_t>& mtlMap);
        bool parseLgt(Asset& asset, ifstream& file);
        void computeTriangleVertexNormals(Asset &asset, size_t beginTri);
        string convert(const Vec3& v) {
            const float epsilon = 1e-5f;
            int x = static_cast<int>(std::round(v.x / epsilon));
            int y = static_cast<int>(std::round(v.y / epsilon));
            int z = static_cast<int>(std::round(v.z / epsilon));
            return std::to_string(x) + "," + std::to_string(y) + "," + std::to_string(z);
        }

    public:
        virtual bool import(Asset& asset, const string& path) override;
    };
}

#endif