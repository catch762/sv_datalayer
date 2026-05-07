#include "InterpolationSystem.h"

QVariantOpt InterpolationSystem::interpolate(const QVariant &A, const QVariant &B, double ratioAToB01)
{
    QtTypeIndex aType = A.typeId();
    QtTypeIndex bType = B.typeId();

    if (aType != bType)
    {
        SV_ERROR(std::format("Trying to interpolate mismatching values: {} {}", qVariantInfo(A), qVariantInfo(B)));
        return {};
    }

    if (auto* interpolator = getInterpolator(aType))
    {
        return (*interpolator)(A, B, ratioAToB01);
    }
    else
    {
        //its fine, dont even need to log error.
        return {};
    }
}

InterpolationSystem &InterpolationSystem::instance()
{
    static InterpolationSystem system;
    return system;
}

const InterpolationSystem::InterpolatorFunc *InterpolationSystem::getInterpolator(QtTypeIndex typeIndex)
{
    return getValue(instance().interpolators, typeIndex);
}
