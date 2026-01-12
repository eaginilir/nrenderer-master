#include "server/Server.hpp"
#include "component/RenderComponent.hpp"

#include "PathTracer.hpp"

using namespace std;
using namespace NRenderer;

namespace RayCast
{
    class Adapter : public RenderComponent
    {
    public:
        void render(SharedScene spScene) {
            PathTracerRenderer renderer{spScene};
            auto result = renderer.render();
            auto [ pixels, width, height ] = result;
            getServer().screen.set(pixels, width, height);
            renderer.release(result);
        }
    };
}

const static string description = 
    "Path Tracing Renderer (KDTree Accelerated).\n"
    "Supported:\n"
    " - Lambertian BRDF\n"
    " - Area Light\n"
    " - Triangle, Sphere, Plane\n"
    " - KDTree Acceleration for Triangle/Mesh\n"
    " - Simple Pinhole Camera\n\n"
    "Please use cornel_area_light.scn"
    ;

REGISTER_RENDERER(Ray_Tracing_KDTree, description, RayCast::Adapter);
