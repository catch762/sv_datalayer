#pragma once
#include "sv_qtcommon.h"
#include "InterpolationInterface.h"
class InterpolationSystem
{
public:
    using InterpolatorFunc = std::function<void(const QVariant &A, const QVariant &B, QVariant &Result, double ratioAToB01)>;

    // Returns success.
    // Will return 'false' in case of types mismatch or if no interpolator registered for this type.
    // The latter situation is perfectly fine, not all types need interpolation.
    static bool interpolate(const QVariant &A, const QVariant &B, QVariant &Result, double ratioAToB01);

    template<typename T>
    static void registerTypeInterpolator();

private:
    InterpolationSystem();
    DISABLE_COPY_AND_ASSIGNMENT(InterpolationSystem);

    static InterpolationSystem& instance();

    static const InterpolatorFunc* getInterpolator(QtTypeIndex typeIndex);

private:
    std::map<QtTypeIndex, InterpolatorFunc> interpolators;
};


template<typename T>
void InterpolationSystem::registerTypeInterpolator()
{
    const auto typeId = qtTypeId<T>();

    SV_ASSERT(!instance().interpolators.contains(typeId));

    InterpolatorFunc wrappedInterpolator = [typeId](const QVariant &A, const QVariant &B, QVariant &Result, double ratioAToB01)
    {
        SV_ASSERT(A.typeId()        == typeId);
        SV_ASSERT(B.typeId()        == typeId);
        SV_ASSERT(Result.typeId()   == typeId);

        // Todo: this is huge ass oversight. We still have to make bunch of expensive copies here.
        // Because cant get fkin pointer/ref from QVariant. I think i ll switch to std::any later, then fix this.

        T resultValue{};
        Interpolator<T>::interpolate(A.value<T>(), B.value<T>(), resultValue, ratioAToB01);
        Result = QVariant::fromValue(resultValue);
    };

    instance().interpolators[qtTypeId<T>()] = wrappedInterpolator;
}