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
        DoNothing, //will not change 'Result' value at all
        TakeA,
        TakeB
    };

    using InterpolatorFunc = std::function<void(const std::any& A,
                                                const std::any& B,
                                                std::any& Result,
                                                double ratioAToB01)>;

    // All QVariant's must be same type, ofcourse.
    // Will only return 'false' in case of types mismatch.
    //
    // If no interpolator registered for this type, will execute 'defaultStrat'.
    // The situation is perfectly fine, not all types need interpolation.
    static bool interpolate(const std::any &A,
                            const std::any &B,
                            std::any &Result,
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

    static const InterpolatorFunc* getInterpolator(std::type_index typeIndex);

private:
    std::map<std::type_index, InterpolatorFunc> interpolators;
};


template<typename T>
void InterpolationSystem::registerTypeInterpolator()
{
    const auto typeId = qtTypeId<T>();

    SV_ASSERT(!instance().interpolators.contains(typeId));

    InterpolatorFunc wrappedInterpolator = [typeId](const std::any &A, const std::any &B, std::any &Result, double ratioAToB01)
    {
        SV_ASSERT(anyHoldsType<T>(A));
        SV_ASSERT(anyHoldsType<T>(B));
        SV_ASSERT(anyHoldsType<T>(Result));

        Interpolator<T>::interpolate(*anyGet<T>(A),
                                     *anyGet<T>(B),
                                     *anyGet<T>(Result),
                                     ratioAToB01);
    };

    instance().interpolators[typeIndex<T>()] = wrappedInterpolator;
}