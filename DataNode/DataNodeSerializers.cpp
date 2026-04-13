#include "DataNodeSerializers.h"

#include "WidgetLogic/WidgetDefs.h"
#include "WidgetLogic/WidgetsForNodeManager.h"
#include "SerializationLogic/SerializationSystem.h"
#include "WidgetLogic/WidgetMakerSystem.h"
#include "WidgetLogic/DataNodeWrapperWidget.h"

bool SerializerForDataNodeTreeAndItsWidgets::onJsonCreatedFromNode_saveWidgetOptions(ConstDataNodeShared node, QJsonObject &jsonOfNode)
{
    auto widgetVariant = WidgetsForNodeManager::getSaveablePrimaryWidgetForNode(node);
    auto hasWidget = qVariantHasWidget(widgetVariant);

    if (hasWidget)
    {
        //if widget isnt supposed to save anything, we will receive empty value - its fine.
        QJsonObjectWithWidgetOptions widgetOptions = SerializationSystem::instance().qVariantToJson(widgetVariant).toObject();
        if (!widgetOptions.isEmpty())
        {
            SV_LOG("Did write widget data to json");
            jsonOfNode[widgetsKey] = widgetOptions;
        }
    }

    return true;
}

bool SerializerForDataNodeTreeAndItsWidgets::onNodeCreatedFromJson_restoreWidget(DataNodeShared node, const QJsonObject &jsonOfNode)
{
    QJsonObjectWithWidgetOptionsOpt widgetOptionsOpt = getFromJson<QJsonObject>(jsonOfNode, widgetsKey);

    lastCreatedWidget = WidgetMakerSystem::instance().createAndRegisterWidgetForNode(node, widgetOptionsOpt);

    if (!qVariantHasWidget(lastCreatedWidget))
    {
        SV_ERROR(std::format("restoring widget failed: Received null QVariantHoldingWidget for leaf node {}", node));
        return false;
    }

    return true;
}


QJsonValue SerializerForDataNodeTreeAndItsWidgets::toJson(const DataNodeShared& value)
{
    if (!value)
    {
        SV_LOG("Error: trying to serialize null DataNodeShared value");
        return QJsonValue();
    }

    if (auto jsonOpt = value->toJSON(onJsonCreatedFromNode_saveWidgetOptions))
    {
        return *jsonOpt;
    }
    else return QJsonValue();
}

std::tuple<DataNodeShared, QVariantHoldingWidget> SerializerForDataNodeTreeAndItsWidgets::fromJson(const QJsonValue& json)
{
    auto resultNode = DataNode::fromJSON(json, std::bind(onNodeCreatedFromJson_restoreWidget, this, std::placeholders::_1, std::placeholders::_2));
    return {resultNode, lastCreatedWidget};
}

