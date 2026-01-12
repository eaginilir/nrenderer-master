#pragma once
#ifndef __ENVMAP_SAMPLER_HPP__
#define __ENVMAP_SAMPLER_HPP__

#include <mutex>

namespace EnvMapPathTracer
{
    using std::mutex;

    class Sampler
    {
    protected:
        static int insideSeed() {
            static mutex m;
            static int seed = 0;
            m.lock();
            seed++;
            m.unlock();
            return seed;
        }
    public:
        virtual ~Sampler() = default;
        Sampler() = default;
    };
}

#endif
