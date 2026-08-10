#pragma once
#include "DataNode/DataNodeHeader.h"
#include "SerializationLogic/SerializerInterface.h"
#include "WidgetLogic/WidgetDefs.h"

using MapOfWidgetOptionsForNodes = std::map<ConstDataNodeWeak, WidgetOptionsJson, std::owner_less<>>;

class SerializerForDataNodeTreeAndItsWidgets
{
public:

    static MapOfWidgetOptionsForNodes getOptionsFromWidgetsOfTree(DataNodeShared tree);

    static QJsonValueOpt toJson(const DataNodeShared& tree);
    static QJsonValueOpt toJson(const DataNodeShared& tree, const MapOfWidgetOptionsForNodes& options);

    //returns root node and widget for root node
    static std::tuple<DataNodeShared, NodeWidget*> jsonToRootNodeAndItsWidget(const QJsonValue& json);

    //returns root node (no widget is made for it. its an invisible root.) and list of widgets for all immediate children of root
    static std::tuple<DataNodeShared, NodeWidgetVec> jsonToRootNodeAndTopLevelChildrenWidgets(const QJsonValue& json);

private:
    static bool onNodeCreatedFromJson_restoreWidget(DataNodeShared node, const QJsonObject &jsonOfNode, int level,
                                                    bool makeWidgetForRootNode);
    
private:
    static inline const QString widgetsKey  = "widgets";
};