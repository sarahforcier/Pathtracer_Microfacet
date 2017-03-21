#include "fulllightingintegrator.h"

Color3f FullLightingIntegrator::Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth, Color3f energy) const
{
    //TODO
    Intersection isect;
    Color3f Le;
    Color3f fColor;
    Color3f gColor;
    Color3f color;
    if (scene.Intersect(ray, &isect)) {
        Vector3f woW = - ray.direction;
        Le = isect.Le(woW);
        if (depth < 1 || !isect.objectHit->GetMaterial()) {
            color = Le;
        } else {
            isect.ProduceBSDF();

            // g (lighting importance sampling)
            int num= scene.lights.length();
            if (num == 0) return color;
            int index = std::min((int)(sampler->Get1D() * num), num - 1);

            float gPdf; Vector3f wiWg;
            const std::shared_ptr<Light> &light = scene.lights[index];
            Color3f li2 = light->Sample_Li(isect, sampler->Get2D(), &wiWg, &gPdf);
            Color3f f2 = isect.bsdf->f(woW, wiWg);
            float wg = BalanceHeuristic(1, gPdf, 1, isect.bsdf->Pdf(woW, wiWg));
            gPdf /= num;

            Intersection shad_Feel;
            if (scene.Intersect(isect.SpawnRay(wiWg), &shad_Feel)) {
                if (shad_Feel.objectHit->areaLight == scene.lights[index] && gPdf > 0.f) {
                    gColor = f2 * li2 * AbsDot(wiWg, isect.normalGeometric)/gPdf;
                }
            }

            // russian roulette
            bool term = false;
            float q = sampler->Get1D();
            if (depth < 3) {

                float E = glm::max(energy.x, glm::max(energy.y, energy.z));
                if (E < q) term = true;
            }

            // f (bsdf importance sampling)
            Vector3f wiWf; float fPdf;
            Point2f xi = sampler->Get2D();
            Color3f f1 = isect.bsdf->Sample_f(woW, &wiWf, xi, &fPdf);
            if (!term && fPdf > 0.f) {
                Color3f f = f1 * AbsDot(wiWf, isect.normalGeometric)/fPdf;
                Color3f li1 = Li(isect.SpawnRay(glm::normalize(wiWf)), scene, sampler, depth -1, (energy * f)/(1.f - q + RayEpsilon));
                fColor = f * li1;
            }
            float wf = BalanceHeuristic(1, fPdf, 1, light->Pdf_Li(isect, wiWf));

            color = Le + wf * fColor + wg * gColor;
        }
    }
    return color;
}

float BalanceHeuristic(int nf, Float fPdf, int ng, Float gPdf)
{
    //TODO
    if (fPdf == 0.f && gPdf == 0.f) return 0.f;
    return (nf * fPdf) / (nf * fPdf + ng * gPdf);
}

float PowerHeuristic(int nf, Float fPdf, int ng, Float gPdf)
{
    //TODO
    if (fPdf == 0.f && gPdf == 0.f) return 0.f;
    float f = nf * fPdf;
    float g = ng * gPdf;
    return (f * f) / (f * f + g * g);
}

