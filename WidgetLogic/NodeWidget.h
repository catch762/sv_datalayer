#pragma once
#include "sv_qtcommon.h"
#include "SerializationLogic/SerializerInterface.h"
#include "WidgetLogic/WidgetDefs.h"
#include "WidgetUtils.h"

//*****************************************************************************************************
//
// Widget that wraps content widget(s) for any DataNode (leaf or composite).
// If you press "+/-" on the top stripe, it will show/hide all content widget(s).
//
// Whether its:
//      - Leaf DataNode with single content widget,
//      - Leaf DataNode with multiple content widgets (if the type is vector<Something>, for example) 
// 
//                  ^is it true ?
// 
// 
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


/////////////////////////////////////////////////////////////////
    //new constructor, soon the only one
    NodeWidget(DataNodeShared node,
        const QString& name = {},
        const QJsonObjectWithWidgetOptionsOpt& options = {},
        QWidget* parent = nullptr);

    static NodeWidget* makeNodeWidgetForCompositeNode( const std::vector<NodeWidget*>& contentWidgets,
                                                DataNodeShared node,
                                                const QString& name = {},
                                                const QJsonObjectWithWidgetOptionsOpt& options = {},
                                                QWidget* parent = nullptr)
    {
        SV_ASSERT(node && node->isComposite());
        NodeWidget* widget = new NodeWidget(node, name, options, parent);

        for (auto contentWidget : contentWidgets)
        {
            SV_ASSERT(contentWidget);

            widget->addContentWidget(contentWidget);
        }

        return widget;
    }


    virtual bool setNodeValueFromWidgetValue()
    {
        SV_ASSERT(false);
        return false;
    }

    virtual bool setWidgetValueFromNodeValue()
    {
        //SV_ASSERT(false); //its fine: plain NodeWidget for comp node doesnt do anything to update self
        return false;
    }

    DataNodeWeak getNode()
    {
        return weakNode;
    }

    template<typename WidgetT, typename WidgetValueChangedSignal>
    void trackValueChanges(const WidgetT* widget, WidgetValueChangedSignal valChangedSignal)
    {
        connect(widget, valChangedSignal, this, &NodeWidget::valueChanged);
    }

    void addContentWidget(QWidget* widget)
    {
        contentLayout->addWidget(widget);
        widget->setVisible(isExpanded());
    }

    int contentWidgetsCount() const
    {
        return contentLayout->count();
    }

    QWidget* getContentWidget(int index)
    {
        if (isValidIndex(index, contentWidgetsCount()))
        {
            auto* item = contentLayout->itemAt(index);
            return item->widget();
        }
        else return nullptr;
    }

    TabIndexOpt getTabIndex() const
    {
        return tabIndex;
    }
    void setTabIndex(TabIndexOpt newIndex)
    {
        tabIndex = newIndex;
    }


signals:
    void valueChanged();


protected:    
    virtual QJsonObjectWithWidgetOptionsOpt makeContentWidgetOptions() const
    {
        return {};
    };

public:
///////////////////////////////////////////////////////////////

    void setExpanded(bool expanded);
    bool isExpanded() const;

    QHBoxLayout* getStripeButtonsLayout();
    
    QJsonObjectWithWidgetOptions makeOptions() const;


private:
    void createAndInitTopStripe(const QString &name);
    void iterateContentWidgets(std::function<void(QWidget*)> visitor);
    
    void setContentWidgetsVisibleStatus(bool visible);

public:
    static const inline QString isExpandedKey   = "_NW_isExpanded";
    static const inline QString tabIndexKey     = "_NW_tabIndex";

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

private:
    const bool isForCompositeNode;
    DataNodeWeak weakNode;

    TabIndexOpt tabIndex;
};

QPushButton* makeTopStripeCheckableButtonWithIcon(QIcon::ThemeIcon offIcon, QIcon::ThemeIcon onIcon);