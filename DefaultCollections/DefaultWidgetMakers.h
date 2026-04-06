#pragma once
#include "sv_qtcommon.h"
#include "DataNode/DataNodeHeader.h"
#include "TypesAndWidgets/TypesAndWidgets.h"
#include "DataLayerUtils.h"

class WidgetMakerSystem;

class DefaultWidgetMakers
{
public:
    static void RegisterEverything(WidgetMakerSystem *system);

    static QVariantHoldingWidget widgetMakerForQString(DataNodeShared leafWithQString, const QJsonObjectWithWidgetOptionsOpt &options);

    static QVariantHoldingWidget widgetMakerForLimitedDouble(DataNodeShared leafWithLimitedDouble, const QJsonObjectWithWidgetOptionsOpt &options);

    static QVariantHoldingWidget widgetMakerForLimitedDoubleVec(DataNodeShared leafWithLimitedDoubleVec, const QJsonObjectWithWidgetOptionsOpt &options);
};