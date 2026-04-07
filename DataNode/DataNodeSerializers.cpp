#include "DataNodeSerializers.h"

#include "DataLayerUtils.h"
#include "WidgetsForNodeManager.h"
#include "SerializationSystem.h"
#include "WidgetMakerSystem.h"
#include "TypesAndWidgets/DataNodeWrapperWidget.h"

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

    if (node->isLeaf())
    {
        //todo: who registers ?
        QVariantHoldingWidget widgetVariant = WidgetMakerSystem::instance().makeWidgetForLeafNode(node, widgetOptionsOpt);
        if (qVariantHasWidget(widgetVariant))
        {
            WidgetsForNodeManager::registerWidgetForNode(node, widgetVariant);
            return true;
        }
        else
        {
            SV_ERROR(std::format("restoring widget failed: Received null QVariantHoldingWidget for leaf node {}", node));
            return false;
        }
    }
    else
    {
        auto widgetForCompNode = createAndRegisterWidgetForCompositeNode(node);
        if (!qVariantHasWidget(widgetForCompNode))
        {
            return false;
        }
        else return true;
    }
}

QVariantHoldingWidget SerializerForDataNodeTreeAndItsWidgets::createAndRegisterWidgetForCompositeNode(DataNodeShared node)
{
    SV_ASSERT(node);
    SV_ASSERT(node->isComposite())

    std::vector<QVariantHoldingWidget> widgetsOfChildren;

    for (auto childNode : node->tryGetCompositeData()->children)
    {
        auto widgetVariant = WidgetsForNodeManager::getSaveablePrimaryWidgetForNode(node);
        auto hasWidget = qVariantHasWidget(widgetVariant);

        if (hasWidget)
        {
            widgetsOfChildren.push_back(widgetVariant);
        }
        else
        {
            //is it fine? maybe it is
            SV_ERROR(std::format("Could not obtain widget for node {} which is child of {}", childNode, node));
        }
    }

    if (widgetsOfChildren.empty())
    {
        SV_ERROR("Could not createWidgetForCompositeNode because didnt find a single child widget. Not doing anything then.");
        return {};
    }

    auto *wrapper = new DataNodeWrapperWidget(widgetsOfChildren, node->getName());
    QVariantHoldingWidget finalWidgetVariant = QVariant::fromValue( QPointer<DataNodeWrapperWidget>(wrapper) );
    WidgetsForNodeManager::registerWidgetForNode(node, finalWidgetVariant);

    return finalWidgetVariant;
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

QVariantHoldingWidget SerializerForDataNodeTreeAndItsWidgets::getRootWidget()
{
    return rootWidget;
}
