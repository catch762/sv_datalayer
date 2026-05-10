#include "InterpolationSystem.h"

bool InterpolationSystem::interpolate(const QVariant &A, const QVariant &B, QVariant &Result, double ratioAToB01)
{
    QtTypeIndex aType       = A.typeId();
    QtTypeIndex bType       = B.typeId();
    QtTypeIndex resultType  = Result.typeId();

    bool allSameType = aType == bType && aType == resultType;

    if (!allSameType)
    {
        SV_ERROR(std::format("Trying to interpolate mismatching values: {} {} to {}",
                                qVariantInfo(A), qVariantInfo(B), qVariantInfo(Result)));
        return false;
    }

    if (auto* interpolator = getInterpolator(aType))
    {
        (*interpolator)(A, B, Result, ratioAToB01);
    }
    else
    {
        //its fine, dont even need to log error.
        return false;
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
