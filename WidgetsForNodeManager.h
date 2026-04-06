#pragma once
#include "DataNode/DataNode.h"
#include <list>
#include <algorithm>
#include "DataLayerUtils.h"

//******************************************************************************************
//
// This class lets you see 'what widgets exist for any given DataNode'.
// Mainly, this is needed for:
//    - synchronizing all widgets state if you change value in one of them
//    - when serializing DataNode, we also want to pull existing widget(s) and save
//      their 'QJsonObjectWithWidgetOptions' as well. (see DataNodeSerializers.h)
//
// As of now, there will be only one widget for any node, ever, but soon this will change
//
//******************************************************************************************

class WidgetsForNodeManager
{
public:
    // Note: QPointer is not owning, but it will become invalid if widget gets deleted.
    // This means, we might need to cleanup empty QPointer's (not done yet), or this
    // class will hold them forever.
    //I assume, if there will be multiple widgets, then most will be short-lived
    using WidgetsContainer = std::list<QVariantHoldingWidget>;

    static void registerWidgetForNode(ConstDataNodeWeak node, QVariantHoldingWidget widget)
    {
        auto *container = instance().getOrCreateContainerForNode(node);
        SV_ASSERT(container);

        container->push_back(widget);
    }
    
    static WidgetsContainer* getWidgetsForNode(ConstDataNodeWeak node)
    {
        return instance().getContainerForNode(node);
    }

    // Figuring 'so which ones do i need to serialize when i serialize DataNode tree'
    // is undecided yet, so i ll just pick first widget.
    static QVariantHoldingWidget getSaveablePrimaryWidgetForNode(ConstDataNodeWeak node)
    {
        if (auto *container = getWidgetsForNode(node))
        {
            auto firstNotNull = std::ranges::find_if(*container, [](const auto& qVariantHoldingWidget) {
                return !qVariantHoldingWidget.isNull();
            });

            return firstNotNull != container->end() ? *firstNotNull : QVariantHoldingWidget{};
        }

        return {};
    }

private:
    static WidgetsForNodeManager instance()
    {
        static WidgetsForNodeManager inst;
        return inst;
    }

    WidgetsContainer* getContainerForNode(ConstDataNodeWeak node)
    {
        auto found = entries.find(node);
        return found != entries.end() ? &found->second : nullptr;
    }
    WidgetsContainer* getOrCreateContainerForNode(ConstDataNodeWeak node)
    {
        if (auto *existingContainer = getContainerForNode(node))
        {
            return existingContainer;
        }
        else
        {
            entries[node] = WidgetsContainer();
            return getContainerForNode(node);
        }
    }

private:
    std::map<ConstDataNodeWeak, WidgetsContainer, std::owner_less<>> entries;
};