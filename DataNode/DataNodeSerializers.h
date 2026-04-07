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

    static bool onNodeCreatedFromJson_restoreWidget(DataNodeShared node, const QJsonObject &jsonOfNode);

    QJsonValue toJson(const DataNodeShared& value);
    
    std::optional<DataNodeShared> fromJson(const QJsonValue& json);

    QVariantHoldingWidget getRootWidget();

private:
    //This assumes that widgets for node's children are already created
    static QVariantHoldingWidget createAndRegisterWidgetForCompositeNode(DataNodeShared node);

private:
    QVariantHoldingWidget rootWidget;

private:
    static inline const QString widgetsKey  = "widgets";
};