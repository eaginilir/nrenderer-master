#pragma once
#ifndef __SHADER_CREATOR_HPP__
#define __SHADER_CREATOR_HPP__

#include "Shader.hpp"
#include "Lambertian.hpp"
#include "Phong.hpp"
#include "CookTorrance.hpp"
#include "Gouraud.hpp"

namespace RayCast
{
    class ShaderCreator
    {
    public:
        ShaderCreator() = default;
        SharedShader create(Material& material, vector<Texture>& t) {
            SharedShader shader{nullptr};
            switch (material.type)
            {
            case 0:
                shader = make_shared<Lambertian>(material, t);
                break;
            case 1:
                shader = make_shared<Phong>(material, t);
                break;
            case 5:
                shader = make_shared<CookTorrance>(material, t);
                break;
            case 6:
                shader = make_shared<Gouraud>(material, t);
                break;
            default:
                shader = make_shared<Lambertian>(material, t);
                break;
            }
            return shader;
        }
    };
}

#endif