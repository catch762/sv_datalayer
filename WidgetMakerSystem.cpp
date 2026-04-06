#include "WidgetMakerSystem.h"
#include "DefaultCollections/DefaultWidgetMakers.h"
#include "TypesAndWidgets/DataNodeWrapperWidget.h"

WidgetMakerSystem& WidgetMakerSystem::instance()
{
    static WidgetMakerSystem system;
    return system;
}

QVariantHoldingWidget WidgetMakerSystem::makeWidgetForLeafNode(DataNodeShared node, const QJsonObjectWithWidgetOptionsOpt &options)
{
    if (!node)
    {
        return {};
    }

    auto widgetMakerNameOpt = getWidgetMakerNameOpt(options);

    if (auto leafValue = node->tryGetLeafvalue())
    {
        if (auto widgetmaker = getWidgetMakerForContentType(*leafValue))
        {
            auto createdInnerWidget = (*widgetmaker)(node, options);
            if (qVariantHasWidget(createdInnerWidget))
            {
                auto *wrapperWidget = new DataNodeWrapperWidget({createdInnerWidget}, node->getName());

                return QVariant::fromValue( QPointer<DataNodeWrapperWidget>(wrapperWidget) );
            }
            else
            {
                SV_ERROR("WidgetMakerSystem", "widget maker returned null for: " + node->stdBasicInfo());
                return {};
            }
        }
        else
        {
            //todo bad inspection
            SV_ERROR("WidgetMakerSystem", "No widget maker exist for: " + node->stdBasicInfo());
            return {};
        }
    }
    else
    {
        SV_ERROR("WidgetMakerSystem", "makeWidgetForLeafNode() called on non-leaf node");
        return {};
    }
}

WidgetMakerSystem::WidgetMakerCollection *WidgetMakerSystem::getCollection(QtTypeIndex typeIndex)
{
    auto found = widgetMakerCollections.find(typeIndex);
    if (found != widgetMakerCollections.end())
    {
        return &found->second;
    }

    //SV_ERROR("No widget maker collection registered for " + qtTypeInfo(typeIndex).toStdString());
    return nullptr;
}

WidgetMakerSystem::WidgetMakerCollection *WidgetMakerSystem::getCollectionAndCreateIfNotFound(QtTypeIndex typeIndex)
{
    if (auto existing = getCollection(typeIndex))
    {
        return existing;
    }

    widgetMakerCollections[typeIndex] = WidgetMakerCollection{};

    auto* res = getCollection(typeIndex);
    SV_ASSERT(res);
    return res;
}

const WidgetMakerSystem::WidgetMakerForTypeT *WidgetMakerSystem::getWidgetMakerForContentType(const QVariant &var,
                                                                                              QStringOpt widgetMakerNameOpt)
{
    if (auto* collection = getCollection(var.typeId()))
    {
        QString widgetMakerName = widgetMakerNameOpt.value_or(collection->defaultWidgetMakerName);

        auto foundWidgetMaker = collection->widgetMakers.find(widgetMakerName);
        if (foundWidgetMaker != collection->widgetMakers.end())
        {
            return &foundWidgetMaker->second;
        }
        else
        {
            SV_ERROR(QString("Requested widgetMakerName=[%1] for %2, but no such widgetMaker in collection")
                    .arg(widgetMakerName).arg(qVariantInfo(var)).toStdString());
        }
    }
    else
    {
        SV_ERROR(QString("Requested widgetMaker for %1, but nothing is registered for this type")
                .arg(qVariantInfo(var)).toStdString());
    }

    return nullptr;
}

WidgetMakerSystem::WidgetMakerSystem()
{
    DefaultWidgetMakers::RegisterEverything(this);
}
