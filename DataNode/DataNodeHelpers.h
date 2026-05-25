#pragma once
#include "DataNode.h"
#include <functional>

//These are helper functions, wrapping call to visitStructurallyEqualTrees() function below.
//Read comments below to figure out details.

bool treesAreStructurallyEqual()

//****************************************************************************************************************
//
// Function takes variable number of trees. So, you pass N trees. They are supposed to be
// structurally equal. What this means is:
//     "Trees must have exact same structure - node types, node names, child count, leaf value types,
//      basically everything, except one thing: while leaf nodes must have same value type,
//      they may have different actual values"
//
// So, this function iterates all N trees one by one, checking if they are infact structurally equal.
// If they are not, function immediately returns false. If they are, and you supplied a visitor,
// on each step it takes all nodes on same place in the trees, and calls visitor on them.
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
template <class Visitor, class... OtherDataNodes>
bool visitStructurallyEqualTrees(const Visitor& siblingVisitor, std::string *outMismatchInfo, int _currentLevel, DataNode& nodeFirst, OtherDataNodes&... nodesRest)
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
                auto checkNullptrChild = [&](DataNode& nodeNext, int childIndex)->bool
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

            if (!doVisit(siblingVisitor, outMismatchInfo, _currentLevel + 1, *nodeFirst.tryGetChild(i), *nodesRest.tryGetChild(i)...))
            {
                //So, the call that decided that children were not equal did save
                //error to 'outMismatchInfo' already - now we just silently return.
                return false;
            }
        }
    }
   
    if constexpr (!std::is_null_pointer_v<std::remove_cvref_t<Visitor>>)
    {
        if (siblingVisitor) siblingVisitor(nodeFirst, nodesRest...);
    }

    return true;
}

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

    auto thevisit = [](DataNode& a, DataNode& b)
    {
        if (a.isLeaf())
        {
            SV_LOG(std::format("LEAVES {} AND {}", a, b));
        }
    };

    std::string e1;
    bool ok1 = doVisit(thevisit, &e1, 0, *a, *b);

    std::string e2;
    bool ok2 = doVisit(nullptr, &e2, 0, *a, *b, *c);

    SV_LOG(std::format("{}[{}] -- {}[{}]", ok1, e1, ok2, e2));
}