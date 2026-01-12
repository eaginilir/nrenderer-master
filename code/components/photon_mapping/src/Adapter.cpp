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
    "Photon Mapping Renderer.\n"
    "Supported:\n"
    " - Lambertian BRDF\n"
    " - Area Light emission\n"
    " - Triangle, Sphere, Plane\n"
    " - KDTree geometry acceleration\n"
    " - k-NN radiance estimate for indirect\n\n"
    "Use cornel_area_light.scn" 
    ;

REGISTER_RENDERER(Photon_Mapping, description, RayCast::Adapter);
