#include "RayCastRenderer.hpp"

#include "VertexTransformer.hpp"
#include "intersections/intersections.hpp"
namespace RayCast
{
    void RayCastRenderer::release(const RenderResult& r) {
        auto [p, w, h] = r;
        delete[] p;
    }
    RGB RayCastRenderer::gamma(const RGB& rgb) {
        return glm::sqrt(rgb);
    }
    auto RayCastRenderer::render() -> RenderResult {
        auto width = scene.renderOption.width;
        auto height = scene.renderOption.height;
        auto pixels = new RGBA[width*height];

        VertexTransformer vertexTransformer{};
        vertexTransformer.exec(spScene);

        ShaderCreator shaderCreator{};
        for (auto& mtl : scene.materials) {
            shaderPrograms.push_back(shaderCreator.create(mtl, scene.textures));
        }

        for (int i=0; i<height; i++) {
            for (int j=0; j < width; j++) {
                auto ray = camera.shoot(float(j)/float(width), float(i)/float(height));
                auto color = trace(ray);
                color = clamp(color);
                color = gamma(color);
                pixels[(height-i-1)*width+j] = {color, 1};
            }
        }

        return {pixels, width, height};
    }
    
    RGB RayCastRenderer::trace(const Ray& r) {
        if (scene.pointLightBuffer.size() < 1) return {0, 0, 0};
        auto& l = scene.pointLightBuffer[0];
        auto closestHitObj = closestHit(r);
        if (closestHitObj) {
            auto& hitRec = *closestHitObj;
            auto out = glm::normalize(l.position - hitRec.hitPoint);
            if (glm::dot(out, hitRec.normal) < 0) {
                return {0, 0, 0};
            }
            auto distance = glm::length(l.position - hitRec.hitPoint);
            auto shadowRay = Ray{hitRec.hitPoint, out};
            auto shadowHit = closestHit(shadowRay);
            auto c = shaderPrograms[hitRec.material.index()]->shade(-r.direction, out, hitRec.normal);
            if (dynamic_pointer_cast<Phong>(shaderPrograms[hitRec.material.index()]) && hitRec.hasVertexData) {
                cout<<"Using Phong Shader"<<endl;
                auto phongShader = dynamic_pointer_cast<Phong>(shaderPrograms[hitRec.material.index()]);
                c = phongShader->shadeTriangle(
                    hitRec.hitPoint,
                    hitRec.vertices[0], hitRec.vertices[1], hitRec.vertices[2],
                    hitRec.normals[0], hitRec.normals[1], hitRec.normals[2],
                    -r.direction,
                    l.position,
                    l.intensity
                );
            }
            if (dynamic_pointer_cast<Gouraud>(shaderPrograms[hitRec.material.index()]) && hitRec.hasVertexData) {
                // Gouraud着色器专用路径
                auto gouraudShader = dynamic_pointer_cast<Gouraud>(shaderPrograms[hitRec.material.index()]);
                cout<<"Using Gouraud Shader"<<endl;
                // cout<<hitRec.normals[0]<<", "<<hitRec.normals[1]<<", "<<hitRec.normals[2]<<endl;
                c = gouraudShader->shadeTriangle(
                    hitRec.hitPoint,
                    hitRec.vertices[0], hitRec.vertices[1], hitRec.vertices[2],
                    hitRec.normals[0], hitRec.normals[1], hitRec.normals[2],
                    -r.direction,
                    l.position,
                    l.intensity
                );
            }
            if ((!shadowHit) || (shadowHit && shadowHit->t > distance)) {
                return c * l.intensity;
            }
            else {
                return Vec3{0};
            }
        }
        else {
            return {0, 0, 0};
        }
    }

    HitRecord RayCastRenderer::closestHit(const Ray& r) {
        HitRecord closestHit = nullopt;
        float closest = FLOAT_INF;
        for (auto& s : scene.sphereBuffer) {
            auto hitRecord = Intersection::xSphere(r, s, 0.01, closest);
            if (hitRecord && hitRecord->t < closest) {
                closest = hitRecord->t;
                closestHit = hitRecord;
            }
        }
        for (auto& t : scene.triangleBuffer) {
            auto hitRecord = Intersection::xTriangle(r, t, 0.01, closest);
            if (hitRecord && hitRecord->t < closest) {
                closest = hitRecord->t;
                closestHit = hitRecord;
            }
        }
        for (auto& p : scene.planeBuffer) {
            auto hitRecord = Intersection::xPlane(r, p, 0.01, closest);
            if (hitRecord && hitRecord->t < closest) {
                closest = hitRecord->t;
                closestHit = hitRecord;
            }
        }
        for (auto& m : scene.meshBuffer) {
            auto hitRecord = Intersection::xMesh(r, m, 0.01, closest);
            if (hitRecord && hitRecord->t < closest) {
                closest = hitRecord->t;
                closestHit = hitRecord;
            }
        }
        return closestHit; 
    }
}