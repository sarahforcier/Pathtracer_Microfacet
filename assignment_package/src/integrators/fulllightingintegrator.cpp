#include "fulllightingintegrator.h"

Color3f FullLightingIntegrator::Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const
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
            gPdf /= num;
            Color3f f2 = isect.bsdf->f(woW, wiWg);

            Intersection shad_Feel;
            wiWg = glm::normalize(wiWg);
            if (scene.Intersect(isect.SpawnRay(wiWg), &shad_Feel)) {
                if (shad_Feel.objectHit->GetAreaLight() && gPdf > 0.f) {
                    gColor = f2 * li2 * AbsDot(wiWg, isect.normalGeometric)/gPdf;
                }
            }
            float wg = PowerHeuristic(1, gPdf, 1, isect.bsdf->Pdf(woW, wiWg));

            // f (bsdf importance sampling)
            Vector3f wiWf; float fPdf;
            Point2f xi = sampler->Get2D();
            Color3f f1 = isect.bsdf->Sample_f(woW, &wiWf, xi, &fPdf);
            Color3f li1 = Li(isect.SpawnRay(glm::normalize(wiWf)), scene, sampler, depth -1);
            if (fPdf > 0.0000001) fColor = f1 * li1 * AbsDot(wiWf, isect.normalGeometric)/fPdf;
            float wf = PowerHeuristic(1, fPdf, 1, light->Pdf_Li(isect, wiWf));

            color = Le + wf * fColor + wg * gColor;
        }
    }
    return color;
}

float BalanceHeuristic(int nf, Float fPdf, int ng, Float gPdf)
{
    //TODO
    return (nf * fPdf) / (nf * fPdf + ng * gPdf);
}

float PowerHeuristic(int nf, Float fPdf, int ng, Float gPdf)
{
    //TODO
    float f = nf * fPdf;
    float g = ng * gPdf;
    return (f * f) / (f * f + g * g);
}

