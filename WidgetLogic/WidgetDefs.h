#pragma once
#include "sv_qtcommon.h"
#include "DataNode/DataNode.h"

static constexpr auto TypeFieldKey = "_type";
template <typename T>
inline void addTypeFieldToJson(QJsonObject &obj)
{
    obj[TypeFieldKey] = qtTypeName<T>();
}

// todo: emphasis on serializing just options.
// Must contain QPointer<ConcreteWidgetType>
//
// Thats how serializable widgets are stored.
// This way, for your ConcreteWidget type, in the same header, you implement
//      Serializer< QPointer<ConcreteWidgetType> > 
//          (Note: you only implement toJson() method, because unlike simple data,
//          creating widgets is more complicated - you also need DataNode, not just json.
//          This is out of scope for Serializer interface, and its done in WidgetMakerSystem. Just FYI.)
//
// ...and then all the other code can serialize widgets like all other items, i.e. simply
// giving 'QVariantHoldingWidget' to 'SerializerSystem'.
//
// The alternative to that would be forcing all widgets inherit from some serializing
// interface. I dont want that, i want any existing widget ready to be used. If you want,
// say, QLineEdit, you dont make more widget classes, you just supply Serializer< QPointer<QLineEdit> >
// and everything should just work.
using QVariantHoldingWidget = QVariant;

//Yes, this casts from QPointer<ConcreteWidget> to QPointer<QWidget>
inline QWidget* getWidgetFromQVariant(const QVariantHoldingWidget& qvariant)
{
    return getValueOr< QWidget* >(qvariant, nullptr);
}

inline bool qVariantHasWidget(const QVariantHoldingWidget& qvariant)
{
    return getWidgetFromQVariant(qvariant) != nullptr;
}  


using QJsonObjectWithWidgetOptions = QJsonObject;
SV_DECL_OPT(QJsonObjectWithWidgetOptions);

constexpr auto WidgetMakerNameKey = "_maker";

inline QStringOpt getWidgetMakerNameOpt(const QJsonObjectWithWidgetOptions &obj)
{
    //return getFromJsonAndLogError<QString>(obj, WidgetMakerNameKey);

    auto value = obj[WidgetMakerNameKey];
    if (value.isString()) return value.toString();

    return {};
}

inline QStringOpt getWidgetMakerNameOpt(const QJsonObjectWithWidgetOptionsOpt &objOpt)
{
    return objOpt ? getWidgetMakerNameOpt(*objOpt) : QStringOpt();
}

inline void setWidgetMakerName(const QJsonObjectWithWidgetOptions &obj, QString widgetMakerName)
{
    obj[WidgetMakerNameKey] = widgetMakerName;
}

struct NodeAndWidgetPair
{
    DataNodeShared node;
    QVariantHoldingWidget widget;

    inline bool isValid()
    {
        return node && qVariantHasWidget(widget);
    }
};
SV_DECL_OPT(NodeAndWidgetPair);