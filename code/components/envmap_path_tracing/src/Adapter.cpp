#include "server/Server.hpp"
#include "scene/Scene.hpp"
#include "component/RenderComponent.hpp"
#include "Camera.hpp"
#include "EnvMapPathTracer.hpp"

using namespace std;
using namespace NRenderer;

namespace EnvMapPathTracer
{
    class Adapter : public RenderComponent
    {
        void render(SharedScene spScene) {
            EnvMapPathTracerRenderer renderer{spScene};
            auto renderResult = renderer.render();
            auto [pixels, width, height] = renderResult;
            getServer().screen.set(pixels, width, height);
            renderer.release(renderResult);
        }
    };
}

const static string description =
    "Path Tracer with Environment Map support. "
    "Supports HDR environment lighting for realistic outdoor/studio scenes. "
    "\nSet ambient type to ENVIRONMENT_MAP and provide a texture.";

REGISTER_RENDERER(EnvMapPathTracer, description, EnvMapPathTracer::Adapter);
