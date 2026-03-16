#include "WidgetMakerSystem.h"
#include "DefaultCollections/DefaultWidgetMakers.h"
#include "TypesAndWidgets/DataNodeWrapperWidget.h"

WidgetMakerSystem& WidgetMakerSystem::instance()
{
    static WidgetMakerSystem system;
    return system;
}

QWidget* WidgetMakerSystem::makeWidgetForNode(DataNodeShared node, const WidgetOptionsJsonOpt &options)
{
    if (!node)
    {
        return nullptr;
    }

    auto widgetMakerNameOpt = getWidgetMakerNameOpt(options);

    if (auto leafValue = node->tryGetLeafvalue())
    {
        if (auto widgetmaker = getWidgetMakerForContentType(*leafValue))
        {
            if (auto createdInnerWidget = (*widgetmaker)(node, options))
            {
                return new DataNodeWrapperWidget({createdInnerWidget}, node->getName());
            }
            else
            {
                SV_ERROR("WidgetMakerSystem", "widget maker returned nullptr for: " + node->stdBasicInfo());
                return nullptr;
            }
        }
        else
        {
            //todo bad inspection
            SV_ERROR("WidgetMakerSystem", "No widget maker exist for: " + node->stdBasicInfo());
            return nullptr;
        }
    }
    else if (auto compData = node->tryGetCompositeData())
    {
        //This node doesnt contain QVariant value, it does contain list of other DataNode's -
        //this is a special case handled here.
        
        QList<QWidget*> subwidgets;
        for (auto subNode : compData->children)
        {
            if(auto subwidget = makeWidgetForNode(subNode))
            {
                subwidgets.append(subwidget);
            }
        }

        return new DataNodeWrapperWidget(subwidgets, node->getName());
    }

    SV_UNREACHABLE();
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
