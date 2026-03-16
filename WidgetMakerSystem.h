#pragma once
#include "DataNode.h"
#include <typeindex>
#include <map>
#include "WidgetOptions.h"



//************************************************************************************************
//
// System for creating widgets for tree containing arbitrary types.
// 
// Usage: 
//
//  - First you build a 'DataNode' tree (Read more in DataNode.h)
//    The tree wraps arbitrary type variables in QVariant.
//    This tree is your data model.
//
//    (Note: tree is 'model' that will be updated when 'view' (widgets) change; but it works
//    one way. As of now, theres no mechanism to update 'view' if you change 'model'
//    other than delete and rebuild entire tree of widgets.)
//
//  - For every type T that is wrapped in QVariant's in the DataNode tree,
//    there must exist a registered 'WidgetMakerForTypeT'.
//    You do register types with 'WidgetMakerSystem::instance().registerWidgetMaker(widgetMaker)'
//
//    See 'WidgetMakerForTypeT' definition and details below, in the class.
//
//  - Then you call 'WidgetMakerSystem::instance().makeWidgetForNode(rootDataNode)'
//    And it will build widgets for your entire tree, walking through every node.
//
// WidgetMakerSystem comes with some types already registered: thats because
// in its constructor it loads registrations from DefaultWidgetMakers class.
// You may want to look into it, and load your own widget makers collection in a simillar fashion.
//
//************************************************************************************************

class WidgetMakerSystem
{
public:
    //************************************************************************************************
    //  - 'WidgetMakerForTypeT' must create appropriate widget and initialize it with node's value
    //
    //  - But most importantly: the widget being created is not the owner of value,
    //    the widget is 'view' and the DataNode is the 'model'. This means that in the 'WidgetMakerForTypeT'
    //    you must setup updating DataNode on widget changes, if necessary.
    //    Widget also doesnt own the DataNode, so it must only hold weak_ptr to it.
    //
    //    So, setting up updating the node would look like that:
    //
    //      connect(widget, &Widget::valueChanged, [nodeWeakPtr](auto value) {
    //          if (auto nodePtr = nodeWeakPtr.lock())
    //              *nodePtr->tryGetLeafvalue() = value;
    //      });
    //
    //    You may see examples in
    //  
    //************************************************************************************************
    using WidgetMakerForTypeT = std::function<QWidget*(DataNodeShared leafNodeContainingValueOfTypeT,
                                                       const WidgetOptionsJsonOpt &options)>;

    struct WidgetMakerCollection
    {
        std::map<QString, WidgetMakerForTypeT> widgetMakers;
        QString defaultWidgetMakerName;
    };

    static WidgetMakerSystem& instance();

    QWidget* makeWidgetForNode(DataNodeShared node, const WidgetOptionsJsonOpt &options = {});      

    template<class T>
    void registerWidgetMaker(WidgetMakerForTypeT maker, const QString& widgetMakerName);

public:    
    // Checks that node is a leaf containing fully registered type T,
    // prints verbose logs if something's wrong.
    // Should be first line in 'WidgetMakerForTypeT' to check its argument node.
    template<typename T>
    static bool checkIsProperLeafNodeForCreatingWidgetOfType(DataNodeShared node);

private:
    //returns nullptr if not found
    WidgetMakerCollection* getCollection(QtTypeIndex typeIndex);
    //always returns valid ptr
    WidgetMakerCollection* getCollectionAndCreateIfNotFound(QtTypeIndex typeIndex);

    //if 'widgetMakerNameOpt' is {}, returns default widget maker for this WidgetMakerCollection
    const WidgetMakerForTypeT* getWidgetMakerForContentType(const QVariant &var, QStringOpt widgetMakerNameOpt = {});
    WidgetMakerSystem();

private:
    std::map<QtTypeIndex, WidgetMakerCollection> widgetMakerCollections;
};



template<class T>
void WidgetMakerSystem::registerWidgetMaker(WidgetMakerForTypeT maker, const QString& widgetMakerName)
{
    QtTypeIndex typeIndex = qMetaTypeId<T>();

    auto *collection = getCollectionAndCreateIfNotFound(typeIndex);
    SV_ASSERT(collection);
    SV_ASSERT(!collection->widgetMakers.contains(widgetMakerName) && "You are trying to register widetMaker, but such name already exists");

    //setting default name to first registered maker
    if (collection->widgetMakers.empty())
    {
        collection->defaultWidgetMakerName = widgetMakerName;
    }

    collection->widgetMakers[widgetMakerName] = maker;
}

// Checks that node is a leaf containing fully registered type T,
// prints verbose logs if something's wrong
template<typename T>
bool WidgetMakerSystem::checkIsProperLeafNodeForCreatingWidgetOfType(DataNodeShared node)
{
    //todo: write about how exactly to register

    int typeId = qtTypeId<T>();
    if (typeId == 0)
    {
        SV_ERROR("WidgetMakerSystem", "Can not create widget: the type has typeid=0, so its not registered");
        return false;
    }

    QString typeName = qtTypeName<T>();
    if(typeName.isEmpty())
    {
        SV_ERROR("WidgetMakerSystem", std::format("Can not create widget: type has typeid=[{}], but empty typeName, so is not FULLY registered", typeId));
        return false;
    }

    auto errMsgHeader = std::format("Can not create widget of type[{}]: ", typeName.toStdString());

    if (!node)
    {
        SV_ERROR("WidgetMakerSystem", errMsgHeader + "null node passed in");
        return false;
    }

    if (!node->isLeaf())
    {
        SV_ERROR("WidgetMakerSystem", errMsgHeader + "node isnt even leaf, its: " + node->stdBasicInfo());
        return false;
    }

    if (!node->isLeafWithType<T>())
    {
        //todo better log
        SV_ERROR("WidgetMakerSystem", errMsgHeader + "its a leaf but types mismatch: " + node->stdBasicInfo());
        return false;
    }

    return true;
}