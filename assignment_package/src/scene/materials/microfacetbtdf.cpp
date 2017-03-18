#include "microfacetbtdf.h"

Color3f MicrofacetBTDF::f(const Vector3f &wo, const Vector3f &wi) const
{
    //TODO!
    float cosO = AbsCosTheta(wo);
    float cosI = AbsCosTheta(wi);

    // inside or out?
    bool in_out = wo.z > 0;
    float n = in_out ? etaA/etaB : etaB/etaA;
    Vector3f wh = glm::normalize(wo + n * wi);
    if (wh.z < 0.f) wh *= -1.f;

    float D = distribution->D(wh);
    float G = distribution->G(wo,wi);
    Color3f F = fresnel->Evaluate(glm::dot(wi,wh));
    float d = glm::dot(wo, wh) + n * glm::dot(wi, wh);
    return n * n * T * D * G * (Color3f(1.f) - F) * AbsDot(wi, wh) * AbsDot(wo, wh) /
            (d * d * cosO * cosI);
}

Color3f MicrofacetBTDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &xi, Float *pdf, BxDFType *sampledType) const
{
    //TODO
    if (sampledType) *sampledType = type;
    Vector3f wh = distribution->Sample_wh(wo, xi);

    // inside or out?
    bool in_out = wo.z > 0;
    float n = in_out ? etaA/etaB : etaB/etaA;
    if (wh.z < 0.f) wh *= -1.f;

    *wi = glm::refract(-wo, wh, n);
    *pdf = Pdf(wo, *wi);
    return f(wo, *wi);
}

float MicrofacetBTDF::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
    //TODO
    // inside or out?
    bool in_out = wo.z > 0;
    float n = in_out ? etaA/etaB : etaB/etaA;
    Vector3f wh = glm::normalize(wo + n * wi);
    if (wh.z < 0.f) wh *= -1.f;

    float d = glm::dot(wo, wh) + n * glm::dot(wi, wh);
    float dwh_dwi = glm::abs(n * n * glm::dot(wi,wh) / d / d);
    return distribution->Pdf(wo, wh) * dwh_dwi;
}
