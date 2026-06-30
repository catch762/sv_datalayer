#pragma once
#include "DataNode/DataNode.h"

// Checks that node is a leaf containing fully registered type T,
// prints verbose logs if something's wrong
template<typename T>
bool nodeSuitableForWidgetOfType(DataNodeShared node)
{
    //todo: write about how exactly to register
    if (!typeIsNamed<T>())
    {
        SV_ERROR(std::format("Can not create widget: the type is not named: {}", mangledTypeName<T>()));
        return false;
    }

    auto errMsgHeader = std::format("Can not create widget of type[{}]: ", typeName<T>());

    if (!node)
    {
        SV_ERROR(errMsgHeader + "null node passed in");
        return false;
    }

    if (!node->isLeaf())
    {
        SV_ERROR(errMsgHeader + "node isnt even leaf, its: " + node->stdBasicInfo());
        return false;
    }

    if (!node->isLeafWithType<T>())
    {
        //todo better log
        SV_ERROR(errMsgHeader + "its a leaf but types mismatch: " + node->stdBasicInfo());
        return false;
    }

    return true;
}