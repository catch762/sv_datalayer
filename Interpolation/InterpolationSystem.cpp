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
        return true;
    }
    else
    {
        //its fine, dont even need to log error.
        return false;
    }
}

bool InterpolationSystem::interpolateTwoTreesToThird(const DataNode &treeA, const DataNode &treeB, DataNode &treeResult, double ratioAToB01)
{
    return visitThreeStructurallyEqualTrees_withMismatchLog(treeA, treeB, treeResult, 
        [ratioAToB01](const DataNode &nodeA, const DataNode &nodeB, DataNode &nodeResult)
        {
            // One check is enough, we only arrive here when all nodes same type etc,
            // all leaf or all composite.
            if (nodeA.isLeaf())
            {
                //its fine if we didnt. not all leaf types should be interpolated.
                bool actuallyInterpolated = InterpolationSystem::interpolate(*nodeA.tryGetLeafvalue(),
                                                                             *nodeB.tryGetLeafvalue(),
                                                                             *nodeResult.tryGetLeafvalue(),
                                                                             ratioAToB01);
            }
        });
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
