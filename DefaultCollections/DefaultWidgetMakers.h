#pragma once
#include "sv_qtcommon.h"
#include "DataNode.h"
#include "TypesAndWidgets/TypesAndWidgets.h"

class WidgetMakerSystem;

class DefaultWidgetMakers
{
public:
    static void RegisterEverything(WidgetMakerSystem *system);

    static QWidget* widgetMakerForQString(DataNodeShared leafWithQString);

    static QWidget* widgetMakerForLimitedDouble(DataNodeShared leafWithLimitedDouble);

    //e.g Leaf node must hold "std::vector<LimitedValue< T_OfLimitedValue >>"
    /*template<typename T_OfLimitedValue>
    static QWidget* widgetMakerForLimitedValueVec(DataNodeShared leafWithQString)
    {
        if (!WidgetMakerSystem::checkIsProperLeafNodeForCreatingWidgetOfType<QString>(leafWithQString))
        {
            return nullptr;
        }
    }*/
};