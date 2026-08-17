#pragma once
#include "sv_qtcommon.h"
#include "DataNode/DataNode.h"

//NodeWidget's belong in the tabs, and they have this property to identify the tab 
using TabIndex = int;
SV_DECL_OPT(TabIndex);

using NodeWidgetVec = std::vector<NodeWidget*>;
using NodeWidgetQPointerVec = std::vector<QPointer<NodeWidget>>;

static constexpr auto TypeFieldKey = "_type";
template <typename T>
inline void addTypeFieldToJson(QJsonObject &obj)
{
    SV_ASSERT(typeIsNamed<T>());
    obj[TypeFieldKey] = typeName<T>();
}


using WidgetOptionsJson = QJsonObject;
SV_DECL_OPT(WidgetOptionsJson);
SV_DECL_ERR(WidgetOptionsJsonOpt);
inline WidgetOptionsJsonOpt getWidgetOptionsFromString(const QStringOpt &jsonString)
{
    return jsonString ? jsonStringToObject(*jsonString) : WidgetOptionsJsonOpt{};
}

using MapOfWidgetOptionsForNodes = std::map<ConstDataNodeWeak, WidgetOptionsJson, std::owner_less<>>;

static constexpr auto WidgetMakerNameKey = "_maker";
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


static constexpr auto CreationStringKey = "_creationString";
inline QStringOpt getCreationStringOpt(const WidgetOptionsJson& obj)
{
    return getFromJson<QString>(obj, CreationStringKey);
}
inline QStringOpt getCreationStringOpt(const WidgetOptionsJsonOpt& objOpt)
{
    return objOpt ? getCreationStringOpt(*objOpt) : QStringOpt();
}
inline void setCreationString(WidgetOptionsJson& obj, const QString& creationString)
{
    obj[CreationStringKey] = creationString;
}



struct NodeAndWidgetPair
{
    DataNodeShared node;
    NodeWidget* widget;

    inline bool isValid()
    {
        return node && widget;
    }
};
SV_DECL_OPT(NodeAndWidgetPair);
using NodeAndWidgetPairList = std::vector<NodeAndWidgetPair>;
SV_DECL_OPT(NodeAndWidgetPairList);


inline void deleteWidgetsAndClear(NodeWidgetVec& vec)
{
    for (auto widget : vec)
    {
        if (widget)
        {
            delete widget;
        }
    }

    vec.clear();
}