#pragma once
#include "sv_qtcommon.h"
#include "DataNode/DataNodeHeader.h"
#include "DataTypesAndTheirWidgets/DataTypesAndTheirWidgets.h"
#include "WidgetLogic/WidgetDefs.h"

class NodeWidget;

class DefaultWidgetMakers
{
public:
    static void RegisterEverything();

    static NodeWidget* widgetMakerForQString(DataNodeShared leafWithQString, const QJsonObjectWithWidgetOptionsOpt &options);

    static NodeWidget* widgetMakerForBool    (DataNodeShared leafWithBool,    const QJsonObjectWithWidgetOptionsOpt &options);
    static NodeWidget* widgetMakerForBoolVec (DataNodeShared leafWithBoolVec, const QJsonObjectWithWidgetOptionsOpt &options);

    static NodeWidget* widgetMakerForLimitedDouble(DataNodeShared leafWithLimitedDouble, const QJsonObjectWithWidgetOptionsOpt &options);
    static NodeWidget* widgetMakerForLimitedInt   (DataNodeShared leafWithLimitedInt,    const QJsonObjectWithWidgetOptionsOpt &options);

    static NodeWidget* widgetMakerForLimitedDoubleVec(DataNodeShared leafWithLimitedDoubleVec, const QJsonObjectWithWidgetOptionsOpt &options);
    static NodeWidget* widgetMakerForLimitedIntVec   (DataNodeShared leafWithLimitedIntVec,    const QJsonObjectWithWidgetOptionsOpt &options);
    
    static NodeWidget* widgetMakerForEnum(DataNodeShared leafWithEnum, const QJsonObjectWithWidgetOptionsOpt &options);
    static NodeWidget* widgetMakerForEnumVec(DataNodeShared leafWithEnumVec, const QJsonObjectWithWidgetOptionsOpt &options);
};