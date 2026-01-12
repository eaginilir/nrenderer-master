#pragma once
#ifndef __ENVMAP_VERTEX_TRANSFORM_HPP__
#define __ENVMAP_VERTEX_TRANSFORM_HPP__

#include "scene/Scene.hpp"

namespace EnvMapPathTracer
{
    using namespace NRenderer;

    class VertexTransformer
    {
    public:
        void exec(SharedScene spScene);
    };
}

#endif
