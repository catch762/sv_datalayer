#pragma once
#include "sv_qtcommon.h"
#include "InterpolationInterface.h"

class InterpolationSystem
{
public:
    using InterpolatorFunc = std::function<QVariant(const QVariant &A, const QVariant &B, double ratioAToB01)>;

    //  - Will return nullopt in case of types mismatch or if no interpolator registered for this type
    static QVariantOpt interpolate(const QVariant &A, const QVariant &B, double ratioAToB01);

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

    InterpolatorFunc wrappedInterpolator = [typeId](const QVariant &A, const QVariant &B, double ratioAToB01)->QVariant
    {
        SV_ASSERT(A.typeId() == typeId);
        SV_ASSERT(B.typeId() == typeId);

        return QVariant::fromValue( Interpolator<T>::interpolate(A.value<T>(), B.value<T>(), ratioAToB01) );
    };

    instance().interpolators[qtTypeId<T>()] = wrappedInterpolator;
}