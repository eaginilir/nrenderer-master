#include "server/Server.hpp"
#include "component/RenderComponent.hpp"

#include "RayTracingRenderer.hpp"

using namespace std;
using namespace NRenderer;

namespace RayTracing
{
    class Adapter : public RenderComponent
    {
    public:
        void render(SharedScene spScene) {
            RayTracingRenderer rayTracing{spScene};
            auto result = rayTracing.render();
            auto [ pixels, width, height ] = result;
            getServer().screen.set(pixels, width, height);
            rayTracing.release(result);
        }
    };
}

const static string description = 
    "Ray Cast Renderer.\n"
    "Supported:\n"
    " - Lambertian and Phong\n"
    " - One Point Light\n"
    " - Triangle, Sphere, Plane\n"
    " - Simple Pinhole Camera\n\n"
    "Please use ray_cast.scn"
    ;

REGISTER_RENDERER(RayTracing, description, RayTracing::Adapter);
