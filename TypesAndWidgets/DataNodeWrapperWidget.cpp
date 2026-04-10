#include "DataNodeWrapperWidget.h"
#include "SerializationSystem.h"
namespace
{
    const int StripeHeight = 24;
    const int StripeMargin = 2;
    const int StripeContentHeight = StripeHeight - 2 * StripeMargin;
}

DataNodeWrapperWidget::DataNodeWrapperWidget(const std::vector<QVariantHoldingWidget> &theContentWidgets,
                                             const QString &name,
                                             const QJsonObjectWithWidgetOptionsOpt& options,
                                             QWidget *parent)
 : QFrame(parent)
{
    setFrameStyle(QFrame::Panel | QFrame::Raised);

    layout = new QVBoxLayout(this);
    layout->setContentsMargins(2,2,2,2);
    layout->setSpacing(5);
    layout->setAlignment(Qt::AlignTop);
    
    createAndInitTopStripe(name);

    contentWidgets = theContentWidgets;
    for (auto contentWidget : contentWidgets)
    {
        SV_ASSERT(contentWidget.canConvert<QWidget*>());
        QWidget* contentAsQWidget = contentWidget.value<QWidget*>();
        
        layout->addWidget(contentAsQWidget);
    }

    if (options)
    {
        if (auto isExpandedOpt = getFromJson<bool>(*options, isExpandedKey))
        {
            SV_LOG("applied expansion");
            setExpanded(*isExpandedOpt);
        }
    }
}

void DataNodeWrapperWidget::setExpanded(bool expanded)
{
    stripeShowHideContentButton->setChecked(expanded);
}

QHBoxLayout *DataNodeWrapperWidget::getStripeLayout()
{
    return stripeLayout;
}

void DataNodeWrapperWidget::createAndInitTopStripe(const QString &name)
{
    topStripe = new QWidget(this);
    topStripe->setFixedHeight(StripeHeight);
    topStripe->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    stripeLayout = new QHBoxLayout(topStripe);
    stripeLayout->setContentsMargins(StripeMargin, StripeMargin, StripeMargin, StripeMargin);
    stripeLayout->setSpacing(5);

    // stripeShowHideContentButton: creating in 'checked (expanded)' state
    {
        static const QString expandedStateText = "-";
        static const QString collapsedStateText = "+";

        stripeShowHideContentButton = new QPushButton(expandedStateText, this);
        stripeShowHideContentButton->setFixedSize(StripeContentHeight, StripeContentHeight);
        
        stripeShowHideContentButton->setCheckable(true);
        stripeShowHideContentButton->setChecked(true);

        connect(stripeShowHideContentButton, &QPushButton::toggled, this, [this](bool checked)
        {
            bool wasExpanded = checked;

            stripeShowHideContentButton->setText(wasExpanded ? expandedStateText : collapsedStateText);
            
            setContentWidgetsVisibleStatus(wasExpanded);
        });

        stripeLayout->addWidget(stripeShowHideContentButton);
    }

    // stripeNameLabel
    {
        stripeNameLabel = new QLabel(name, this);
        stripeLayout->addWidget(stripeNameLabel);
    }

    layout->addWidget(topStripe, 0);
}

void DataNodeWrapperWidget::iterateContentWidgets(std::function<void(QWidget *)> visitor)
{
    for (int i = 1; i < layout->count(); ++i)
    {
        if(auto *item = layout->itemAt(i))
        {
            if (auto *widget = item->widget())
            {
                visitor(widget);
            }
        }
    }
}

void DataNodeWrapperWidget::setContentWidgetsVisibleStatus(bool visible)
{
    iterateContentWidgets([=](auto *widget)
    {
        widget->setVisible(visible);
    });
}

QJsonObjectWithWidgetOptions DataNodeWrapperWidget::makeOptions() const
{
    QJsonObjectWithWidgetOptions obj;
    
    //todo add widget name...

    //then this widget is for leaf node; and if its for comp node, we dont save anything else
    if (contentWidgets.size() == 1) 
    {
        //its perfectly ok to receive no value here - simply means content widget doesnt save anything
        if (auto contentOptions = convertJson<QJsonObject>( SerializationSystem::instance().qVariantToJson(contentWidgets.front()) ))
        {
            obj = *contentOptions;
        }
    }

    obj[isExpandedKey] = stripeShowHideContentButton->isChecked();

    return obj;
}
