#include "DataNodeSerializers.h"

#include "DataLayerUtils.h"
#include "WidgetsForNodeManager.h"
#include "SerializationSystem.h"
#include "WidgetMakerSystem.h"


bool SerializerForDataNodeTreeAndItsWidgets::onJsonCreatedFromNode_saveWidgetOptions(ConstDataNodeShared node, QJsonObject &jsonOfNode)
{
    auto widgetVariant = WidgetsForNodeManager::getSaveablePrimaryWidgetForNode(node);
    auto hasWidget = !getWidgetFromQVariant(widgetVariant).isNull();
    if (hasWidget)
    {
        QJsonObjectWithWidgetOptions widgetOptions = SerializationSystem::instance().qVariantToJson(widgetVariant).toObject();
        if (!widgetOptions.isEmpty())
        {
            SV_LOG("Did write widget data to json");
            jsonOfNode[widgetsKey] = widgetOptions;
        }
    }
}

bool SerializerForDataNodeTreeAndItsWidgets::onNodeCreatedFromJson_restoreWidget(DataNodeShared node, const QJsonObject &jsonOfNode)
{
    QJsonObjectWithWidgetOptionsOpt widgetOptionsOpt = getFromJson<QJsonObject>(jsonOfNode, widgetsKey);

    WidgetMakerSystem::instance().makeWidgetForLeafNode(node, widgetOptionsOpt);
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

std::optional<DataNodeShared> SerializerForDataNodeTreeAndItsWidgets::fromJson(const QJsonValue& json)
{
    auto result = DataNode::fromJSON(json, onNodeCreatedFromJson_restoreWidget);
    if (result) return result;
    else return {};
}