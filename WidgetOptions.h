#pragma once
#include "sv_qtcommon.h"

using WidgetOptionsJson = QJsonObject;
SV_DECL_OPT(WidgetOptionsJson);

constexpr auto WidgetMakerNameKey = "_maker";

inline QStringOpt getWidgetMakerNameOpt(const WidgetOptionsJson &obj)
{
    //return getFromJsonAndLogError<QString>(obj, WidgetMakerNameKey);

    auto value = obj[WidgetMakerNameKey];
    if (value.isString()) return value.toString();

    return {};
}

inline QStringOpt getWidgetMakerNameOpt(const WidgetOptionsJsonOpt &objOpt)
{
    return objOpt ? getWidgetMakerNameOpt(*objOpt) : QStringOpt();
}

inline void setWidgetMakerName(const WidgetOptionsJson &obj, QString widgetMakerName)
{
    obj[WidgetMakerNameKey] = widgetMakerName;
}