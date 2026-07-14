#include "../DataNodeEquality.h"
#include "doctest/doctest.h"

TEST_CASE("All DataNode tree equality functions")
{
    auto makeBaseTree = []()
    {
        return  DataNode::makeComposite("A", 
                {
                    DataNode::makeLeaf("B0", 5),
                    DataNode::makeLeaf("B1", 5),
                    DataNode::makeComposite("B2",
                    {
                        DataNode::makeLeaf("C0", 5.67574321)
                    })
                });
    };

    auto base = makeBaseTree();
    auto same_as_base = makeBaseTree();
    auto structurallyDifferent = makeBaseTree();
    {
        structurallyDifferent->tryGetChild(2)->addChild(DataNode::makeLeaf("C1", 777));
    }
    auto structurallyEqualButValueDifferent = makeBaseTree();
    {
        auto nodeC0 = structurallyEqualButValueDifferent->tryGetChild(2)->tryGetChild(0);
        auto val = nodeC0->tryGetLeafValueContent<double>();
        REQUIRE(val);

        (*val) += 0.0000001;

        if (auto leaf = nodeC0->tryGetLeafvalue())
        {
            *leaf = std::any(*val);
        }
    }

    CHECK(treesAreStructurallyEqual (*base, *same_as_base));
    CHECK(treesAreCompletelyEqual   (*base, *same_as_base));

    CHECK(!treesAreStructurallyEqual(*base, *structurallyDifferent));
    CHECK(!treesAreCompletelyEqual  (*base, *structurallyDifferent));

    CHECK(treesAreStructurallyEqual (*base, *structurallyEqualButValueDifferent));
    CHECK(!treesAreCompletelyEqual  (*base, *structurallyEqualButValueDifferent));
}