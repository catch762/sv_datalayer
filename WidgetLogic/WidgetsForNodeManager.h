#pragma once
#include "DataNode/DataNode.h"
#include <list>
#include <algorithm>
#include "WidgetDefs.h"

//******************************************************************************************
//
// This class lets you see 'what widgets exist for any given DataNode'.
// Mainly, this is needed for:
//    - synchronizing all widgets state if you  a) change value in one of them
//                                              b) change DataNode directly
//    - when serializing DataNode, we also want to pull existing widget(s) and save
//      their 'QJsonObjectWithWidgetOptions' as well. (see DataNodeSerializers.h)
//
// As of now, there will be only one widget for any node, ever, but soon this will change
//
//******************************************************************************************

// WARNING AS OF NOW DOESNT TRACK DELETED WIDGETS, SO IF U DELETE SMTH, INVALID PTR WILL REMAIN HERE.
class WidgetsForNodeManager
{
public:
    
    // Unfortunately, cant use one variable.
    // Note: QPointer is not owning, but it will become invalid if widget gets deleted.
    // This means, we might need to cleanup empty QPointer's (not done yet), or this
    // class will hold them forever.
    // I assume, if there will be multiple widgets, then most will be short-lived
    struct WidgetEntry
    {
        WidgetEntry(const QVariantHoldingWidget &widget) :
            qVariant(widget), qPointer(getWidgetFromQVariant(widget)) {}

        bool stillAlive() const {return qPointer;}

        QVariantHoldingWidget qVariant; //e.g. QVariant holding raw pointer, like a QLineEdit* 
        QPointer<QWidget>     qPointer; //exact same pointer wrapped in QPointer so we can check if widget is already deleted.
    };

    using WidgetsContainer = std::list<WidgetEntry>;

    static void registerWidgetForNode(ConstDataNodeWeak node, QVariantHoldingWidget widget)
    {
        auto *container = instance().getOrCreateContainerForNode(node);
        SV_ASSERT(container);

        container->push_back(widget);

        SV_LOG(std::format("Registered widget (now {}) for node {}", container->size(), node));
    }
    
    static void updateAllWidgetsFromNodeState(ConstDataNodeWeak node)
    {
        if (auto container = getWidgetsForNode(node))
        {
            for (const auto &widgetEntry : *container)
            {
                if (widgetEntry.stillAlive())
                {
                    if (auto* wrapperWidget = qobject_cast<DataNodeWrapperWidget*>(widgetEntry.qPointer.data()))
                    {
                        wrapperWidget->updateContentWidgetsFromDataNode(node);
                    }
                    else SV_WARN(std::format("WidgetsForNodeManager: couldnt cast widget to DataNodeWrapperWidget "
                                             "to update it from node {}", node));
                }
            }
        }
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
            auto firstNotNull = std::ranges::find_if(*container, [](const auto& qVariantHoldingWidget)
            {
                return qVariantHoldingWidget.stillAlive();
            });

            return firstNotNull != container->end() ? firstNotNull->qVariant : QVariantHoldingWidget{};
        }

        return {};
    }

    static void clear()
    {
        instance().entries.clear();
    }

    static void clearAllDeletedWidgets()
    {
        auto &entries = instance().entries;

        for (auto& [node, container] : entries)
        {
            std::erase_if(container, [](const auto &widget){ return !widget.stillAlive(); });
        }

        //delete empty containers
        std::erase_if(entries, [](const auto &keyAndValue){ return keyAndValue.second.empty(); });
    }

private:
    static WidgetsForNodeManager& instance()
    {
        static WidgetsForNodeManager inst;
        return inst;
    }

    WidgetsContainer* getContainerForNode(ConstDataNodeWeak node)
    {
        auto found = entries.find(node);

        if (found != entries.end())
        {
            return &found->second;
        }
        else
        {
            return nullptr;
        }
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
            auto *createdContainer = getContainerForNode(node);
            return createdContainer;
        }
    }

private:
    std::map<ConstDataNodeWeak, WidgetsContainer, std::owner_less<>> entries;
};