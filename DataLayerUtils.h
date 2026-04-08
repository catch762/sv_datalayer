#pragma once
#include "sv_qtcommon.h"
#include <QPointer>

static constexpr auto TypeFieldKey = "_type";

static constexpr auto MinFloatInUI  = std::numeric_limits<float>::lowest() / 10.0f;
static constexpr auto MaxFloatInUI  = std::numeric_limits<float>::max() / 10.0f;
static constexpr auto MinIntInUI    = std::numeric_limits<int>::lowest() / 10;
static constexpr auto MaxIntInUI    = std::numeric_limits<int>::max() / 10;

template <typename T>
inline void addTypeFieldToJson(QJsonObject &obj)
{
    obj[TypeFieldKey] = qtTypeName<T>();
}

class AdhocTesting
{
public:
    static void runTest();
};

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