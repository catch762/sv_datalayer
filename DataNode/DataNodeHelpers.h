#pragma once
#include "DataNode.h"
#include <functional>

//****************************************************************************************************************
//
// This is impl function which does all the work - i ll describe it in depth.
// But you really should use helper functions below. You ll figure them out
//
// Function takes variable number of trees. So, you pass N trees. They are supposed to be
// structurally equal. What this means is:
//     "Trees must have exact same structure - node types, node names, child count, leaf value types,
//      basically everything, except one thing: while leaf nodes must have same value type,
//      they may have different actual values"
//
// So, this function iterates all N trees one by one, checking if they are infact structurally equal.
// If they are not, function immediately returns false. If they are, and you supplied a visitor,
// on each step it takes all nodes on same place in the trees (siblings), and calls visitor on them.
//
// Return value: 'are all trees structurally equal or not' 
// 
// Arguments:
//
// 'siblingVisitor'         - optional visitor std::function (pass nullptr if dont need it).
//                            If no checks fail, called in the end like this:
//                            siblingVisitor(DataNode& sibling1, DataNode& sibling2, ..., DataNode& siblingN);
//
// 'outMismatchInfo'        - optional error string, if there was a mismatch, it contains text of
//                            what exactly mismatched. Pass nullptr if you dont need it.
//
// '_currentLevel'          - always pass 0. This is to track recursion level when filling 'outMismatchInfo'
//
// 'nodeFirst, nodesRest'   - just pass your DataNode& trees one by one, there must be at least two.
//
//****************************************************************************************************************
template <bool CompareLeafValues, class Visitor, class... OtherDataNodes>
bool visitStructurallyEqualTreesImpl(const Visitor&             siblingVisitor,
                                     std::string*               outMismatchInfo,
                                     int                        _currentLevel,
                                     const DataNode&            nodeFirst,
                                     const OtherDataNodes&...   nodesRest);

// 1) Const nodes version - just calls Impl
template <bool CompareLeafValues, class Visitor, class... OtherDataNodes>
bool visitStructurallyEqualConstTrees(const Visitor&             siblingVisitor,
                                 std::string*               outMismatchInfo,
                                 const DataNode&            nodeFirst,
                                 const OtherDataNodes&...   nodesRest);

// 2) Non-const nodes version - wraps visitor and calls Impl
template <bool CompareLeafValues, class Visitor, class... OtherDataNodes>
bool visitStructurallyEqualTrees(const Visitor&     siblingVisitor,
                                 std::string*       outMismatchInfo,
                                 DataNode&          nodeFirst,
                                 OtherDataNodes&... nodesRest);

//**************************************
// ACTUAL HELPER METHODS YOU SHOULD USE:
//**************************************

bool leafValuesEqual(const QVariant& a, const QVariant& b);

//When all you need is to check if trees are structurally equal.
//Two variants, one logs mismatch.
template <class... OtherDataNodes>
bool treesAreStructurallyEqual(const DataNode& nodeFirst,
                               const OtherDataNodes&... nodesRest)
{
    return visitStructurallyEqualConstTrees<false>(nullptr, nullptr, nodeFirst, nodesRest...);
}
template <class... OtherDataNodes>
bool treesAreStructurallyEqual_withMismatchLog(const DataNode& nodeFirst,
                                               const OtherDataNodes&... nodesRest)
{
    std::string mismatchError;
    bool result = visitStructurallyEqualConstTrees<false>(nullptr, &mismatchError, nodeFirst, nodesRest...);
    if (!result) SV_ERROR(mismatchError);
    return result;
}

template <class... OtherDataNodes>
bool treesAreCompletelyEqual(const DataNode& nodeFirst,
                             const OtherDataNodes&... nodesRest)
{
    return visitStructurallyEqualConstTrees<true>(nullptr, nullptr, nodeFirst, nodesRest...);
}

template <class... OtherDataNodes>
bool treesAreCompletelyEqual_withMismatchLog(const DataNode& nodeFirst,
                                               const OtherDataNodes&... nodesRest)
{
    std::string mismatchError;
    bool result = visitStructurallyEqualConstTrees<true>(nullptr, &mismatchError, nodeFirst, nodesRest...);
    if (!result) SV_ERROR(mismatchError);
    return result;
}

inline bool visitThreeStructurallyEqualTrees_withMismatchLog(   const DataNode& nodeFirst,
                                                                const DataNode& nodeSecond,
                                                                      DataNode& nodeThird,
                                                                const std::function<void(   const DataNode&,
                                                                                            const DataNode&,
                                                                                                  DataNode&)> &visitor )
{
    //one tree is non-const, so we have to cast all to non-const

    std::string mismatchError;
    bool result = visitStructurallyEqualTrees<false>(  visitor,
                                                &mismatchError,
                                                const_cast<DataNode&>(nodeFirst),
                                                const_cast<DataNode&>(nodeSecond),
                                                nodeThird );
    if (!result) SV_ERROR(mismatchError);
    return result;
};



//********************************
// IMPLEMENTATION FUNCTIONS:
//********************************

template <bool CompareLeafValues, class Visitor, class... OtherDataNodes>
bool visitStructurallyEqualTreesImpl(const Visitor& siblingVisitor, std::string *outMismatchInfo, int _currentLevel, const DataNode& nodeFirst, const OtherDataNodes&... nodesRest)
{
    static_assert((std::is_same_v<OtherDataNodes, DataNode> && ...),
                  "All node arguments must be DataNode&");

    auto setMismatchError = [&](const DataNode& mismatchedNode, int mismatchedNodeIndex, std::string&& errorText)
    {
        if (outMismatchInfo)
        {
            *outMismatchInfo = std::format("[On level {}] NOT STRUCTURALLY EQUAL nodes found: [idx 0] {} and [idx {}] {}: {}",
                                        _currentLevel, nodeFirst, mismatchedNodeIndex, mismatchedNode, std::move(errorText));
        }
    };

    // Nodes must have same name
    {
        int argIndex = 1;
        auto checkName = [&](const DataNode& nodeNext)->bool
        {
            if (nodeFirst.getName() != nodeNext.getName())
            {
                if (outMismatchInfo) setMismatchError(nodeNext, argIndex, "different names");
                return false;
            }

            argIndex++;
            return true;
        };

        if (!(checkName(nodesRest) && ...))
        {
            return false;
        }
    }

    // Nodes must have same type
    {
        int argIndex = 1;
        auto checkType = [&](const DataNode& nodeNext)->bool
        {
            if (nodeFirst.isLeaf() != nodeNext.isLeaf())
            {
                if (outMismatchInfo) setMismatchError(nodeNext, argIndex, "different node type");
                return false;
            }

            argIndex++;
            return true;
        };

        if (!(checkType(nodesRest) && ...))
        {
            return false;
        }
    }

    // For leaf nodes: nodes must have same value type id, and thats all we check
    if (nodeFirst.isLeaf())
    {
        int argIndex = 1;
        auto checkLeafTypeId = [&](const DataNode& nodeNext)->bool
        {
            if (nodeFirst.tryGetLeafvalue()->typeId() != nodeNext.tryGetLeafvalue()->typeId())
            {
                if (outMismatchInfo) setMismatchError(nodeNext, argIndex, "different leaf value type");
                return false;
            }

            argIndex++;
            return true;
        };

        if (!(checkLeafTypeId(nodesRest) && ...))
        {
            return false;
        }

        if constexpr (CompareLeafValues)
        {
            int argIndex = 1;
            auto checkLeafValue = [&](const DataNode& nodeNext)->bool
            {
                if (!leafValuesEqual(*nodeFirst.tryGetLeafvalue(), *nodeNext.tryGetLeafvalue()))
                {
                    if (outMismatchInfo) setMismatchError(nodeNext, argIndex, "same type, but different leaf VALUE");
                    return false;
                }

                argIndex++;
                return true;
            };

            if (!(checkLeafValue(nodesRest) && ...))
            {
                return false;
            }
        }
    }
    else // For composite nodes:
    {
        // Nodes must have same number of children
        {
            int argIndex = 1;
            auto checkNumberOfChildren = [&](const DataNode& nodeNext)->bool
            {
                if (nodeFirst.childrenCount() != nodeNext.childrenCount())
                {
                    if (outMismatchInfo) setMismatchError(nodeNext, argIndex, "different children count");
                    return false;
                }

                argIndex++;
                return true;
            };

            if (!(checkNumberOfChildren(nodesRest) && ...))
            {
                return false;
            }
        }

        //Ok, we can compare sets of children now, and its responsibility of nested 'structurallyEqual' call.
        //Last thing we check here is if any child is nullptr. Just in case.
        for (int i = 0; i < nodeFirst.childrenCount(); ++i)
        {
            //Nullptr check. Any nullptr will cause function to return false.
            {
                auto logNullptrChild = [&](const DataNode& nodeWithNullChild, int treeIndex, int nullChildIndex)
                {
                    if (outMismatchInfo)
                    {
                        *outMismatchInfo = std::format("[On level {}] nodes will be considered NOT STRUCTURALLY EQUAL because [idx {}] {} has nullptr child at [{}]",
                                        _currentLevel, treeIndex, nodeWithNullChild, nullChildIndex);
                    }
                };

                if (!nodeFirst.tryGetChild(i))
                {
                    if (outMismatchInfo) logNullptrChild(nodeFirst, 0, i);
                    return false;
                }

                int argIndex = 1;
                auto checkNullptrChild = [&](const DataNode& nodeNext, int childIndex)->bool
                {
                    if (!nodeNext.tryGetChild(childIndex))
                    {
                        if (outMismatchInfo) logNullptrChild(nodeNext, argIndex, childIndex);
                        return false;
                    }

                    argIndex++;
                    return true;
                };

                if (!(checkNullptrChild(nodesRest, i) && ...))
                {
                    return false;
                }
            }

            if (!visitStructurallyEqualTreesImpl<CompareLeafValues>(siblingVisitor, outMismatchInfo, _currentLevel + 1, *nodeFirst.tryGetChild(i), *nodesRest.tryGetChild(i)...))
            {
                //So, the call that decided that children were not equal did save
                //error to 'outMismatchInfo' already - now we just silently return.
                return false;
            }
        }
    }
   
    if constexpr (!std::is_null_pointer_v<std::remove_cvref_t<Visitor>>)
    {
        siblingVisitor(nodeFirst, nodesRest...);
    }

    return true;
}

template <bool CompareLeafValues, class Visitor, class... OtherDataNodes>
bool visitStructurallyEqualConstTrees(const Visitor&             siblingVisitor,
                                 std::string*               outMismatchInfo,
                                 const DataNode&            nodeFirst,
                                 const OtherDataNodes&...   nodesRest)
{
    static_assert((std::is_same_v<std::remove_cvref_t<OtherDataNodes>, DataNode> && ...),
                  "All node arguments must be: const DataNode&");

    return visitStructurallyEqualTreesImpl<CompareLeafValues>(
        siblingVisitor, outMismatchInfo, 0,
        nodeFirst, nodesRest...);
}

template <bool CompareLeafValues, class Visitor, class... OtherDataNodes>
bool visitStructurallyEqualTrees(const Visitor&     siblingVisitor,
                                 std::string*       outMismatchInfo,
                                 DataNode&          nodeFirst,
                                 OtherDataNodes&... nodesRest)
{
    static_assert((std::is_same_v<std::remove_cvref_t<OtherDataNodes>, DataNode> && ...),
                  "All node arguments must be: DataNode&");

    if constexpr (std::is_null_pointer_v<std::remove_cvref_t<Visitor>>)
    {
        return visitStructurallyEqualTreesImpl<CompareLeafValues>(
        nullptr, outMismatchInfo, 0,
        nodeFirst, nodesRest...);
    }
    else
    {
        // Visitor must take const args, so we wrap in const version, then cast const away inside
        auto constVisitor = [&](const DataNode& f, const OtherDataNodes&... r)
        {
            siblingVisitor(
                const_cast<DataNode&>(f),
                const_cast<OtherDataNodes&>(r)...
            );
        };

        return visitStructurallyEqualTreesImpl<CompareLeafValues>(
            constVisitor, outMismatchInfo, 0,
            nodeFirst, nodesRest...);
    }
}

/*
inline void testvisitor()
{

    auto makeBaseTree = []()
    {
        return DataNode::makeComposite("a", {
            DataNode::makeLeaf("a2-0", 5),
            DataNode::makeLeaf("a2-1", 5),
            DataNode::makeComposite("a2-2", {
                DataNode::makeLeaf("a3-0", 5)
            })
        });
    };

    auto a = makeBaseTree();
    auto b = makeBaseTree();
    auto c = makeBaseTree();

    c->tryGetChild(2)->tryGetCompositeData()->children[0].reset();

    auto thevisit = [](const DataNode& a, const DataNode& b)
    {
        if (a.isLeaf())
        {
            SV_LOG(std::format("LEAVES {} AND {}", a, b));
        }
    };

    treesAreStructurallyEqual_withMismatchLog(*a, *b, *c);
}
*/