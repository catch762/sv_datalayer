#include "DataNodeWrapperWidget.h"

namespace
{
    const int StripeHeight = 24;
    const int StripeMargin = 2;
    const int StripeContentHeight = StripeHeight - 2 * StripeMargin;
}

DataNodeWrapperWidget::DataNodeWrapperWidget(const std::vector<QVariantWithWidgetPointer> &theContentWidgets, const QString &name, QWidget *parent)
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
}

void DataNodeWrapperWidget::setExpanded(bool expanded)
{
    stripeShowHideContentButton->setChecked(expanded);
}

void DataNodeWrapperWidget::createAndInitTopStripe(const QString &name)
{
    topStripe = new QWidget(this);
    topStripe->setFixedHeight(StripeHeight);
    topStripe->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    auto stripeLayout = new QHBoxLayout(topStripe);
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
