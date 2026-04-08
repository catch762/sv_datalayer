#pragma once
#include "DataNode/DataNodeHeader.h"
#include "SerializerInterface.h"
#include "DataLayerUtils.h"

/*template<>
class BasicSerializer< DataNodeShared >
{
public:
    QJsonValue toJson(const DataNodeShared& value)
    {
        if (!value)
        {
            SV_LOG("Error: trying to serialize null DataNodeShared value");
            return QJsonValue();
        }

        auto jsonOpt = value->toJSON();
        return jsonOpt.value_or(QJsonValue());
    }
    
    std::optional<DataNodeShared> fromJson(const QJsonValue& json)
    {
        auto result = DataNode::fromJSON(json);
        if (result) return result;
        else return {};
    }
};*/



class SerializerForDataNodeTreeAndItsWidgets
{
public:
    static bool onJsonCreatedFromNode_saveWidgetOptions(ConstDataNodeShared node, QJsonObject &jsonOfNode);

    bool onNodeCreatedFromJson_restoreWidget(DataNodeShared node, const QJsonObject &jsonOfNode);

    QJsonValue toJson(const DataNodeShared& value);
    
    //returns root node and widget for root node
    std::tuple<DataNodeShared, QVariantHoldingWidget> fromJson(const QJsonValue& json);

private:
    //Since we go depth first, when we are done deserializing, lastCreatedWidget should hold root widget.
    QVariantHoldingWidget lastCreatedWidget;

private:
    static inline const QString widgetsKey  = "widgets";
};