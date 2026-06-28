#pragma once
#include "DataNode/DataNode.h"

// Checks that node is a leaf containing fully registered type T,
// prints verbose logs if something's wrong
template<typename T>
bool nodeSuitableForWidgetOfType(DataNodeShared node)
{
    //todo: write about how exactly to register

    int typeId = qtTypeId<T>();
    if (typeId == 0)
    {
        SV_ERROR("Can not create widget: the type has typeid=0, so its not registered");
        return false;
    }

    QString typeName = qtTypeName<T>();
    if (typeName.isEmpty())
    {
        SV_ERROR(std::format("Can not create widget: type has typeid=[{}], but empty typeName, so is not FULLY registered", typeId));
        return false;
    }

    auto errMsgHeader = std::format("Can not create widget of type[{}]: ", typeName.toStdString());

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