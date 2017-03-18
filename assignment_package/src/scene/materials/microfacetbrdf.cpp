#include "microfacetbrdf.h"

Color3f MicrofacetBRDF::f(const Vector3f &wo, const Vector3f &wi) const
{
    //TODO!
    float cosO = AbsCosTheta(wo);
    float cosI = AbsCosTheta(wi);
    if (cosO == 0.f || cosI == 0.f) return Color3f(0.f);
    Vector3f wh = glm::normalize(wo + wi);
    float D = distribution->D(wh);
    float G = distribution->G(wo,wi);
    Color3f F = fresnel->Evaluate(glm::dot(wi,wh));
    return R * D * G * F / (4.f * cosO * cosI);
}

Color3f MicrofacetBRDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &xi, Float *pdf, BxDFType *sampledType) const
{
    //TODO
    if (sampledType) *sampledType = type;
    Vector3f wh = distribution->Sample_wh(wo, xi);
    *wi = glm::reflect(-wo,wh);
    *pdf = Pdf(wo, *wi);
    return f(wo, *wi);
}

float MicrofacetBRDF::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
    //TODO
    Vector3f wh = glm::normalize(wo + wi);
    return distribution->Pdf(wo, wh) / (4.f * glm::dot(wo, wh));
}
