#pragma once
#include "sv_qtcommon.h"
#include "DataNode.h"
#include "TypesAndWidgets/TypesAndWidgets.h"

class WidgetMakerSystem;

class DefaultWidgetMakers
{
public:
    static void RegisterEverything(WidgetMakerSystem *system);

    static QWidget* widgetMakerForQString(DataNodeShared leafWithQString, const WidgetOptionsJsonOpt &options);

    static QWidget* widgetMakerForLimitedDouble(DataNodeShared leafWithLimitedDouble, const WidgetOptionsJsonOpt &options);

    static QWidget* widgetMakerForLimitedDoubleVec(DataNodeShared leafWithLimitedDoubleVec, const WidgetOptionsJsonOpt &options);
};