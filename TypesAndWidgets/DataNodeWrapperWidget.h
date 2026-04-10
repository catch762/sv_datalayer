#pragma once
#include "sv_qtcommon.h"
#include "DataLayerUtils.h"
#include "SerializerInterface.h"
#include "WidgetOptions.h"

//*****************************************************************************************************
//
// Widget that wraps content widget(s) for DataNode of any type.
// If you press "+/-" on the top stripe, it will show/hide all content widget(s).
//
// Whether its:
//      - Leaf DataNode with single content widget,
//      - Leaf DataNode with multiple content widgets (if the type is vector<Something>, for example) 
//      - Composite DataNode with multiple content widgets for each DataNode child --
//   all these content widgets are used uniformly, and simply inserted into layout.
//
//*****************************************************************************************************

//todo write abt it


class DataNodeWrapperWidget : public QFrame
{
    Q_OBJECT
public:
    DataNodeWrapperWidget(  const std::vector<QVariantHoldingWidget>& contentWidgets,
                            const QString&                            name           = {},
                            const QJsonObjectWithWidgetOptionsOpt&    options        = {},
                            QWidget*                                  parent         = nullptr );

    template<class WidgetType>                        
    DataNodeWrapperWidget(  WidgetType*                             widget,
                            const QString&                          name    = {},
                            const QJsonObjectWithWidgetOptionsOpt&  options = {},
                            QWidget*                                parent  = nullptr )
        : DataNodeWrapperWidget({QVariantHoldingWidget::fromValue(widget)}, name, options, parent)
    {
    }

    void setExpanded(bool expanded);
    
    QHBoxLayout* getStripeLayout();
    
    QJsonObjectWithWidgetOptions makeOptions() const;
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
    std::vector<QVariantHoldingWidget>      contentWidgets;
};

template<>
class Serializer< DataNodeWrapperWidget* >
{
public:
    using WidgetPtr = DataNodeWrapperWidget*;

    QJsonValue toJson(const WidgetPtr& value)
    {
        return value->makeOptions();
    }

    std::optional<WidgetPtr> fromJson(const QJsonValue& json)
    {
        SV_UNREACHABLE();
    }
};