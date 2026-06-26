#pragma once
#include "sv_qtcommon.h"
#include "SerializationLogic/SerializerInterface.h"
#include "WidgetLogic/WidgetDefs.h"

//*****************************************************************************************************
//
// Widget that wraps content widget(s) for any DataNode (leaf or composite).
// If you press "+/-" on the top stripe, it will show/hide all content widget(s).
//
// Whether its:
//      - Leaf DataNode with single content widget,
//      - Leaf DataNode with multiple content widgets (if the type is vector<Something>, for example) 
//      - Composite DataNode with multiple content widgets for each DataNode child --
// 
//   -- all these content widgets are used uniformly, and simply inserted into layout.
//
//*****************************************************************************************************

//todo write abt it


class NodeWidget : public QWidget
{
    Q_OBJECT
public:
    // Assuming argument is a widget for certain DataNode, this function should update
    // widget value from that DataNode.
    //
    // Its simply called on all content widgets when you call updateContentWidgetsFromDataNode()
    //
    // (At this point, its only supplied when we are wrapping single widget for leaf node - 
    // because currently i dont change tree structure, only leaf node values, so thats all
    // widgets should account for. At the moment. So its only passed in constructor taking single widget.)
    using UpdateContentWidgetFromNodeFunc = std::function<void(QWidget*, ConstDataNodeWeak)>;

    NodeWidget(  const std::vector<QVariantHoldingWidget>& contentWidgets,
                            bool                                      isForCompositeNode,
                            const QString&                            name           = {},
                            const QJsonObjectWithWidgetOptionsOpt&    options        = {},
                            QWidget*                                  parent         = nullptr );

    template<class WidgetType>                        
    NodeWidget(  WidgetType*                             widget,
                            const QString&                          name    = {},
                            const QJsonObjectWithWidgetOptionsOpt&  options = {},
                            UpdateContentWidgetFromNodeFunc         theContentUpdater = nullptr,
                            QWidget*                                parent  = nullptr )
        : NodeWidget({QVariantHoldingWidget::fromValue(widget)}, false, name, options, parent)
    {
        contentUpdater = std::move(theContentUpdater);
    }

/////////////////////////////////////////////////////////////////
    //new constructor, soon the only one
    NodeWidget(     DataNodeShared node,
                    bool isForCompositeNode,
                    const QString& name = {},
                    const QJsonObjectWithWidgetOptionsOpt& options = {},
                    QWidget* parent = nullptr)
        : NodeWidget(std::vector<QVariantHoldingWidget>{  }, isForCompositeNode, name, options, parent)
    {
        weakNode = node;

        //note: derived not constructed yet, make sure its not get called yet
        connect(this, &NodeWidget::valueChanged, this, &NodeWidget::setNodeValueFromWidgetValue);
    }

    //any real reason this is separate func? no.
    virtual void createAndInitContentWidgets(DataNodeShared node, const QJsonObjectWithWidgetOptionsOpt& options = {})
    {
        SV_ASSERT(false);
    }

    virtual bool setNodeValueFromWidgetValue()
    {
        SV_ASSERT(false);
        return false;
    }

    virtual bool setWidgetValueFromNodeValue()
    {
        SV_ASSERT(false);
        return false;
    }

    DataNodeWeak getNode()
    {
        return weakNode;
    }

signals:
    void valueChanged();

public:
///////////////////////////////////////////////////////////////

    void setExpanded(bool expanded);
    
    QHBoxLayout* getStripeButtonsLayout();
    
    QJsonObjectWithWidgetOptions makeOptions() const;

    void updateContentWidgetsFromDataNode(ConstDataNodeWeak weakNode);

private:
    void createAndInitTopStripe(const QString &name);
    void iterateContentWidgets(std::function<void(QWidget*)> visitor);
    
    void setContentWidgetsVisibleStatus(bool visible);

private:
    static const inline QString isExpandedKey = "_DNWW_isExpanded";

private:
    QVBoxLayout*                            layout                              = nullptr;
    QWidget*                                    topStripe                       = nullptr;
    QHBoxLayout*                                stripeLayout                    = nullptr;
    QPushButton*                                    stripeShowHideContentButton = nullptr;
    QLabel*                                         stripeNameLabel             = nullptr;
    //content widget may set up additional buttons that go in the stripe, in this layout
    //and they all are only visible when expanded.
    QHBoxLayout*                                    stripeButtonsLayout         = nullptr;
    QHBoxLayout*                            frameAndContentLayout               = nullptr;
    QFrame*                                     frameVerticalLine               = nullptr;
    QVBoxLayout*                                contentLayout                   = nullptr;
    std::vector<QVariantHoldingWidget>              contentWidgets;

    bool isForCompositeNode = false;
    UpdateContentWidgetFromNodeFunc contentUpdater;

private:
    DataNodeWeak weakNode;
};

QPushButton* makeTopStripeCheckableButtonWithIcon(QIcon::ThemeIcon offIcon, QIcon::ThemeIcon onIcon);

template<>
class Serializer< NodeWidget* >
{
public:
    using WidgetPtr = NodeWidget*;

    QJsonValue toJson(const WidgetPtr& value)
    {
        return value->makeOptions();
    }

    std::optional<WidgetPtr> fromJson(const QJsonValue& json)
    {
        SV_UNREACHABLE();
    }
};