#pragma once
#include "sv_qtcommon.h"
#include "InterpolationInterface.h"

class InterpolationSystem
{
public:
    using InterpolatorFunc = std::function<QVariant(const QVariant &A, const QVariant &B, double ratioAToB01)>;

    //  - Will return nullopt in case of types mismatch or if no interpolator registered for this type
    QVariantOpt interpolate(const QVariant &A, const QVariant &B, double ratioAToB01);

private:


private:
    std::map<QtTypeIndex, InterpolatorFunc> interpolators;
};