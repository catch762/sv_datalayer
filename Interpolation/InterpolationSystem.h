#pragma once
#include "sv_qtcommon.h"
#include "InterpolationInterface.h"
#include "DataNode/DataNodeHeader.h"
class InterpolationSystem
{
public:
    //used if no interpolator is registered
    enum class DefaultMixingStrategy
    {
        DoNothing,
        TakeA,
        TakeB
    };

    using InterpolatorFunc = std::function<void(const QVariant &A,
                                                const QVariant &B,
                                                QVariant &Result,
                                                double ratioAToB01)>;

    // Will return 'false' in case of types mismatch or if no interpolator registered for this type.
    // If no interpolator registered for this type, will execute 'defaultStrat'.
    // The latter situation is perfectly fine, not all types need interpolation.
    static bool interpolate(const QVariant &A,
                            const QVariant &B,
                            QVariant &Result,
                            double ratioAToB01,
                            DefaultMixingStrategy defaultStrat = DefaultMixingStrategy::DoNothing);

    template<typename T>
    static void registerTypeInterpolator();

    //returns success; will return false if all 3 trees are not structurally equal
    static bool interpolateTwoTreesToThird( const DataNode& treeA, 
                                            const DataNode& treeB, 
                                            DataNode& treeResult, 
                                            double ratioAToB01,
                                            DefaultMixingStrategy defaultStrat = DefaultMixingStrategy::DoNothing );

private:
    InterpolationSystem() = default;
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