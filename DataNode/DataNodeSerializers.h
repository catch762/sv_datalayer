#pragma once
#include "DataNode/DataNodeHeader.h"
#include "SerializationLogic/SerializerInterface.h"
#include "WidgetLogic/WidgetDefs.h"

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

    static QJsonValueOpt toJson(const DataNodeShared& value);

    //returns root node and widget for root node
    static std::tuple<DataNodeShared, NodeWidget*> jsonToRootNodeAndItsWidget(const QJsonValue& json);

    //returns root node (no widget is made for it. its an invisible root.) and list of widgets for all immediate children of root
    static std::tuple<DataNodeShared, NodeWidgetVec> jsonToRootNodeAndTopLevelChildrenWidgets(const QJsonValue& json);

private:
    static bool onJsonCreatedFromNode_saveWidgetOptions(ConstDataNodeShared node, QJsonObject &jsonOfNode, int level);
    static bool onNodeCreatedFromJson_restoreWidget(DataNodeShared node, const QJsonObject &jsonOfNode, int level,
                                             bool makeWidgetForRootNode);
    
private:
    static inline const QString widgetsKey  = "widgets";
};