#pragma once
#include "sv_qtcommon.h"
#include "DataNode/DataNodeHeader.h"
#include "DataTypesAndTheirWidgets/DataTypesAndTheirWidgets.h"
#include "WidgetLogic/WidgetDefs.h"

class WidgetWrapper;

class DefaultWidgetMakers
{
public:
    static void RegisterEverything();

    static WidgetWrapper* widgetMakerForQString(DataNodeShared leafWithQString, const QJsonObjectWithWidgetOptionsOpt &options);

    static WidgetWrapper* widgetMakerForBool    (DataNodeShared leafWithBool,    const QJsonObjectWithWidgetOptionsOpt &options);
    static WidgetWrapper* widgetMakerForBoolVec (DataNodeShared leafWithBoolVec, const QJsonObjectWithWidgetOptionsOpt &options);

    static WidgetWrapper* widgetMakerForLimitedDouble(DataNodeShared leafWithLimitedDouble, const QJsonObjectWithWidgetOptionsOpt &options);
    static WidgetWrapper* widgetMakerForLimitedInt   (DataNodeShared leafWithLimitedInt,    const QJsonObjectWithWidgetOptionsOpt &options);

    static WidgetWrapper* widgetMakerForLimitedDoubleVec(DataNodeShared leafWithLimitedDoubleVec, const QJsonObjectWithWidgetOptionsOpt &options);
    static WidgetWrapper* widgetMakerForLimitedIntVec   (DataNodeShared leafWithLimitedIntVec,    const QJsonObjectWithWidgetOptionsOpt &options);
    
    static WidgetWrapper* widgetMakerForEnum(DataNodeShared leafWithEnum, const QJsonObjectWithWidgetOptionsOpt &options);
    static WidgetWrapper* widgetMakerForEnumVec(DataNodeShared leafWithEnumVec, const QJsonObjectWithWidgetOptionsOpt &options);
};