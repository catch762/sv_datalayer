#include "NodeWidget.h"
#include "SerializationLogic/SerializationSystem.h"
#include "NodeWidgetChangeNotifier.h"

namespace
{
    const int StripeHeight = 24;
    const int StripeMargin = 2;
    const int StripeContentHeight = StripeHeight - 2 * StripeMargin;
    const int VerticalLineWidth = 16;
}

NodeWidget::NodeWidget(DataNodeShared node,
                        const QString &name,
                        const WidgetOptionsJsonOpt& options,
                        QWidget *parent)
 : QWidget(parent), isForCompositeNode(node->isComposite())
{
    SV_ASSERT(node);
    weakNode = node;

    layout = new QVBoxLayout(this);
    initLayoutSpacing(layout);
    layout->setAlignment(Qt::AlignTop);

    //Item 1 in layout: topStripe
    {
        createAndInitTopStripe(name);
        layout->addWidget(topStripe);
    }
    //Item 2 in layout: everything else
    {
        frameAndContentLayout = new QHBoxLayout();
        initLayoutSpacing(frameAndContentLayout);
        frameAndContentLayout->setAlignment(Qt::AlignLeft);
        layout->addLayout(frameAndContentLayout);

        //Item 1 in frameAndContentLayout: frameVerticalLine
        if(isForCompositeNode)
        {
            frameVerticalLine = new QFrame(this);
            frameVerticalLine->setFixedWidth(VerticalLineWidth);
            frameVerticalLine->setFrameStyle(QFrame::VLine | QFrame::Plain);
            frameVerticalLine->setLineWidth(1);
            frameVerticalLine->setMidLineWidth(0);
            frameVerticalLine->setFrameStyle(QFrame::NoFrame);
            frameVerticalLine->setFrameStyle(QFrame::NoFrame);
            frameVerticalLine->setStyleSheet(R"(
                QFrame {
                    border: none;
                    background: transparent;
                    border-left: 1px solid palette(mid);
                    margin-left: 10px;  /* Centers: shift line from left edge */
                    padding: 0;
                }
            )");
            frameVerticalLine->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
            frameAndContentLayout->addWidget(frameVerticalLine);
        }

        //Item 2 in frameAndContentLayout: contentLayout
        {
            contentLayout = new QVBoxLayout();
            contentLayout->setSpacing(0);
            contentLayout->setContentsMargins(0,0,0,0);
            contentLayout->setAlignment(Qt::AlignTop);
            frameAndContentLayout->addLayout(contentLayout);
        }
    }


    if (options)
    {
        if (auto isExpandedOpt = getFromJson<bool>(*options, isExpandedKey))
        {
            //SV_LOG("applied expansion");
            setExpanded(*isExpandedOpt);
        }

        if (auto tabIndexFromJson = getFromJson<int>(*options, tabIndexKey))
        {
            tabIndex = tabIndexFromJson;
            SV_LOG(std::format("NodeWidget [{}] got assigned tab index [{}]", name, *tabIndex));
        }

        if (auto creationStringFromJson = getCreationStringOpt(*options))
        {
            creationString = creationStringFromJson;
        }

        if (auto updateStatusFromJson = getUpdateStatusOpt(*options))
        {
            setUpdateStatus(*updateStatusFromJson);
        }
    }

    //note: derived not constructed yet, make sure its not get called yet
    connect(this, &NodeWidget::valueChanged, this, [this]()
    {
        setNodeValueFromWidgetValue();
        if (auto notifier = NodeWidgetChangeNotifier::instance())
        {
            emit notifier->someNodeWidgetChanged();
        }
    });
}

void NodeWidget::setExpanded(bool expanded)
{
    stripeShowHideContentButton->setChecked(expanded);
}

bool NodeWidget::isExpanded() const
{
    return stripeShowHideContentButton->isChecked();
}

QHBoxLayout *NodeWidget::getStripeButtonsLayout()
{
    return stripeButtonsLayout;
}

QPushButton* makeTopStripeCheckableButtonWithIcon(QIcon::ThemeIcon offIcon, QIcon::ThemeIcon onIcon)
{
    auto *button = new QPushButton();
    button->setFixedSize(StripeContentHeight, StripeContentHeight);
    button->setCheckable(true);

    QIcon icon;
    {
        int theIconSize = StripeContentHeight - 8;
        QSize iconSize(theIconSize, theIconSize);

        // Expand state (plus) → On
        icon.addPixmap(QIcon::fromTheme(offIcon)
                        .pixmap(iconSize),
                    QIcon::Normal, QIcon::Off);

        // Collapse state (minus) → Off
        icon.addPixmap(QIcon::fromTheme(onIcon)
                        .pixmap(iconSize),
                    QIcon::Normal, QIcon::On);
    }

    button->setIcon(icon);
    //stripeShowHideContentButton->setFlat(true);

    // - move to the right 2px, because otherwise icon is not fucking centered
    // - make it so when its checked its same color as when its not
    button->setStyleSheet(R"(
        QPushButton {
            padding: 0px 0px 0px 2px;
            margin: 0px;
            border: 1px solid rgb(180,180,180);
            border-radius: 1px;
        }
        QPushButton:checked {
            background-color: palette(button);
        }
    )");

    return button;
}

void NodeWidget::createAndInitTopStripe(const QString &name)
{
    topStripe = new QWidget(this);
    topStripe->setObjectName("TopStripe");
    topStripe->setFixedHeight(StripeHeight);
    topStripe->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    stripeLayout = new QHBoxLayout(topStripe);
    stripeLayout->setContentsMargins(StripeMargin, StripeMargin, StripeMargin, StripeMargin);
    stripeLayout->setSpacing(5);

    // stripeShowHideContentButton: creating in 'checked (expanded)' state
    {
        static const QString expandedStateText = "-";
        static const QString collapsedStateText = "+";

        stripeShowHideContentButton = makeTopStripeCheckableButtonWithIcon(QIcon::ThemeIcon::ListAdd,
                                                                           QIcon::ThemeIcon::ListRemove);
        stripeShowHideContentButton->setChecked(true);

        connect(stripeShowHideContentButton, &QPushButton::toggled, this, [this](bool checked)
        {
            bool wasExpanded = checked;

            //stripeShowHideContentButton->setText(wasExpanded ? expandedStateText : collapsedStateText);
            
            setContentWidgetsVisibleStatus(wasExpanded);
        });

        stripeLayout->addWidget(stripeShowHideContentButton);
    }

    // stripeNameLabel
    {
        stripeNameLabel = new QLabel(name, this);
        stripeLayout->addWidget(stripeNameLabel);
    }

    {
        stripeButtonsLayout = new QHBoxLayout();
        initLayoutSpacing(stripeButtonsLayout);
        stripeButtonsLayout->setAlignment(Qt::AlignRight);
        stripeLayout->addLayout(stripeButtonsLayout);
    }
}

void NodeWidget::iterateContentWidgets(std::function<void(QWidget *)> visitor)
{
    for (int i = 0; i < contentLayout->count(); ++i)
    {
        if(auto *item = contentLayout->itemAt(i))
        {
            if (auto* widget = item->widget())
            {
                visitor(widget);
            }
            else SV_ASSERT(false); //so far i expect nothing else can be there
        }
    }
}

void NodeWidget::setContentWidgetsVisibleStatus(bool visible)
{
    if(frameVerticalLine)
    {
        frameVerticalLine->setVisible(visible);
    }
    
    iterateContentWidgets([=](auto *widget)
    {
        widget->setVisible(visible);
    });

    for (int i = 0; i < stripeButtonsLayout->count(); ++i)
    {
        if(auto *item = stripeButtonsLayout->itemAt(i))
        {
            if (auto *widget = item->widget())
            {
                widget->setVisible(visible);
            }
        }
    }
}

WidgetOptionsJson NodeWidget::makeOptions() const
{
    WidgetOptionsJson obj = makeContentWidgetOptions().value_or(WidgetOptionsJson());
    
    //todo add widget name...

    obj[isExpandedKey] = isExpanded();

    if (tabIndex)
    {
        obj[tabIndexKey] = tabIndex.value();
    }

    if (creationString)
    {
        setCreationString(obj, *creationString);
    }

    //Lets not save it for now, see if we truly need it.
    /*
    if (updateStatus)
    {
        ::setUpdateStatus(obj, *updateStatus);
    }
    */

    return obj;
}