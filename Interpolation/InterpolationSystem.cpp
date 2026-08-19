#include "InterpolationSystem.h"

bool InterpolationSystem::interpolate(  const std::any& A, 
                                        const std::any& B,
                                        std::any& Result,
                                        double ratioAToB01,
                                        DefaultMixingStrategy defaultStrat )
{
    auto aType       = typeIndex(A);
    auto bType       = typeIndex(B);
    auto resultType  = typeIndex(Result);

    bool allSameType = aType == bType && aType == resultType;

    if (!allSameType)
    {
        SV_ERROR(std::format("Trying to interpolate mismatching values: {} {} to {}",
                                A, B, Result));
        return false;
    }

    if (auto* interpolator = getInterpolator(aType))
    {
        (*interpolator)(A, B, Result, ratioAToB01);
    }
    else
    {
        if (defaultStrat == DefaultMixingStrategy::DoNothing)
        {
        }
        else if(defaultStrat == DefaultMixingStrategy::TakeA)
        {
            Result = A;
        }
        else if(defaultStrat == DefaultMixingStrategy::TakeB)
        {
            Result = B;
        }
        else SV_UNREACHABLE();
    }

    return true;
}

bool InterpolationSystem::interpolateTwoTreesToThird(   const DataNode &treeA, 
                                                        const DataNode &treeB, 
                                                        DataNode &treeResult, 
                                                        double ratioAToB01,
                                                        DefaultMixingStrategy defaultStrat)
{
    auto mismatchErrOpt = visitThreeStructurallyEqualTrees_withMismatchInfo(treeA, treeB, treeResult, 
        [ratioAToB01, defaultStrat](const DataNode &nodeA, const DataNode &nodeB, DataNode &nodeResult)
        {
            // One check is enough, we only arrive here when all nodes same type etc,
            // all leaf or all composite.
            if (nodeA.isLeaf())
            {
                SV_ASSERT(nodeB.isLeaf() && nodeResult.isLeaf());

                //its fine if we didnt. not all leaf types should be interpolated.
                /*bool actuallyInterpolated =*/ InterpolationSystem::interpolate(*nodeA.tryGetLeafvalue(),
                                                                             *nodeB.tryGetLeafvalue(),
                                                                             *nodeResult.tryGetLeafvalue(),
                                                                             ratioAToB01,
                                                                             defaultStrat);
                //SV_LOG(std::format("[{}] interp for type {}", actuallyInterpolated, nodeA));
            }
        });

    if (mismatchErrOpt)
    {
        SV_ERROR(*mismatchErrOpt);
        return false;
    }
    else return true;
}

InterpolationSystem &InterpolationSystem::instance()
{
    static InterpolationSystem system;
    return system;
}

const InterpolationSystem::InterpolatorFunc *InterpolationSystem::getInterpolator(std::type_index typeIndex)
{
    return getValue(instance().interpolators, typeIndex);
}
