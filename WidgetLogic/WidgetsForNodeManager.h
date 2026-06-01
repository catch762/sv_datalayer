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
        std::string toString() const {
            bool variantOk = qVariantHasWidget(qVariant);
            bool pointerOk = stillAlive();
            if (variantOk == pointerOk)
            {
                return std::format("WEntry[{}]", variantOk);
            }
            else return std::format("WEntry[{} {} MIXED]", variantOk, pointerOk);
        };

        QVariantHoldingWidget qVariant; //e.g. QVariant holding raw pointer, like a QLineEdit* 
        QPointer<QWidget>     qPointer; //exact same pointer wrapped in QPointer so we can check if widget is already deleted.
    };

    using WidgetsContainer = std::list<WidgetEntry>;

    static void registerWidgetForNode(ConstDataNodeWeak node, QVariantHoldingWidget widget)
    {
        if (!qVariantHasWidget(widget))
        {
            SV_ERROR(std::format("Cant register empty widget for {}", node));
        }

        auto *container = instance().getOrCreateContainerForNode(node);
        SV_ASSERT(container);

        container->push_back(widget);

        SV_LOG(std::format("Registered widget (now {}) for node {}", container->size(), node));

        if(node.lock()->getName() == "hello")
        {
            SV_LOG("this is it");
        }
    }
    
    static void updateAllWidgetsFromNodeState(ConstDataNodeWeak node);

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
        SV_LOG("WidgetsForNodeManager: cleared all entries");
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

    static void printEntries(ConstDataNodeWeak node)
    {
        if (auto *container = instance().getContainerForNode(node))
        {
            std::string line = "{ ";
            for(const auto &e : *container)
            {
                line += e.toString() + " ";
            }
            line += "}";

            SV_LOG(std::format("Widgets for {} ---> {}", node, line));
        }
        else SV_LOG(std::format("No widgets for {}", node));
    }
    static void printAllEntries()
    {
        SV_LOG("WidgetsForNodeManager ENTRIES BEGIN");
        for (auto &e : instance().entries)
        {
            printEntries(e.first);
        }
        SV_LOG("WidgetsForNodeManager ENTRIES END");
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