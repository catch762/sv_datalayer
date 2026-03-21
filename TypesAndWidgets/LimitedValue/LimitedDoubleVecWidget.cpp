#include "LimitedDoubleVecWidget.h"
#include "TypesAndWidgets/LimitedValue/Internal/BaseXYPadWidget.h"
#include "TypesAndWidgets/LimitedValue/Internal/XYPresetsWidget.h"

LimitedDoubleVecWidget::LimitedDoubleVecWidget(const LimitedDoubleVec& initialValue, QWidget *parent)
    : QFrame(parent)
{
    layout = new QVBoxLayout(this);

    {
        tempSwapModesButton = new QPushButton("swap", this);
        connect(tempSwapModesButton, &QPushButton::clicked, [this]()
        {
            auto newMode = (curMode == Mode::ShowJustLimitedDoubleWidgets) ? Mode::ShowXYPad :
                                                                             Mode::ShowJustLimitedDoubleWidgets;
            setMode(newMode);
        });
        layout->addWidget(tempSwapModesButton);
    }

    basicWidgetsLayout = new QVBoxLayout();
    layout->addLayout(basicWidgetsLayout);

    xyPad = new BaseXYPadWidget(this);
    layout->addWidget(xyPad);

    xyPadPresets = new XYPresetsWidget(this);
    layout->addWidget(xyPadPresets);

    //this will (unless 'initialValue' is empty) create 'basicWidgets' and put them in 'basicWidgetsLayout'
    setValue(initialValue);
}

const LimitedDoubleVec& LimitedDoubleVecWidget::getValue() const
{
    return value;
}

void LimitedDoubleVecWidget::setValue(const LimitedDoubleVec& newValue)
{
    value = newValue;

    setWidgetsStateFromValue(value);

    emit valueChanged(value);
}

void LimitedDoubleVecWidget::setMode(Mode newMode)
{
    curMode = newMode;

    for (int i = 0; i < basicWidgets.size(); ++i)
    {
        basicWidgets[i]->setVisible(basicWidgetShouldBeVisible(i));
    }

    if(curMode == Mode::ShowJustLimitedDoubleWidgets)
    {
        xyPad->setVisible(false);
        xyPadPresets->setVisible(false);
    }
    else if (curMode == Mode::ShowXYPad)
    {
        xyPad->setVisible(true);
        xyPadPresets->setVisible(true);
    }
    else
    {
        SV_UNREACHABLE();
    }
}

void LimitedDoubleVecWidget::onSomethingChanged()
{
    setCurrentValueFromWidgetsState();

    emit valueChanged(value);
}

void LimitedDoubleVecWidget::setCurrentValueFromWidgetsState()
{
    auto valueSize = value.size();
    auto widgetsSize = basicWidgets.size();

    if (valueSize != widgetsSize)
    {
        SV_LOG(std::format("LimitedDoubleVecWidget, set val from widgets: valueSize[{}] but widgetsSize[{}], will resize",
                valueSize, widgetsSize));
        value.resize(widgetsSize);
    }

    SV_ASSERT(value.size() == basicWidgets.size());
    for (int i = 0; i < widgetsSize; ++i)
    {
        value[i] = basicWidgets[i]->currentValue();
    }
}

void LimitedDoubleVecWidget::setBasicWidgetsCount(int requiredBasicWidgetsCount)
{
    auto existingbasicWidgets = basicWidgets.size();

    if (existingbasicWidgets > requiredBasicWidgetsCount)
    {
        int widgetsToDelete = existingbasicWidgets - requiredBasicWidgetsCount;

        SV_LOG(std::format("setbasicWidgetsCount will delete [{}] widgets", widgetsToDelete));

        for (int i = 0; i < widgetsToDelete; ++i)
        {
            delete basicWidgets.back();
            basicWidgets.pop_back();
        }
    }
    else if (requiredBasicWidgetsCount > existingbasicWidgets)
    {
        int widgetsToAdd = requiredBasicWidgetsCount - existingbasicWidgets;

        SV_LOG(std::format("setbasicWidgetsCount will add [{}] widgets", widgetsToAdd));

        for (int i = 0; i < widgetsToAdd; ++i)
        {
            //not even setting value, well do it later
            auto widget = new LimitedDoubleWidget(LimitedDouble{}, this);

            connect(widget, &LimitedDoubleWidget::valueChanged, this, &LimitedDoubleVecWidget::onSomethingChanged);

            basicWidgets.push_back(widget);
            basicWidgetsLayout->addWidget(widget);

            int thisBasicIndex = basicWidgets.size() - 1;
            widget->setVisible(basicWidgetShouldBeVisible(thisBasicIndex));
        }
    }
    else
    {
        SV_LOG("setbasicWidgetsCount will do nothing");
    }
}

void LimitedDoubleVecWidget::setWidgetsStateFromValue(const LimitedDoubleVec& value)
{
    setBasicWidgetsCount(value.size());

    SV_ASSERT(value.size() == basicWidgets.size());

    for (int i = 0; i < basicWidgets.size(); ++i)
    {
        auto widget = basicWidgets[i];

        QSignalBlocker blocker(widget);
        widget->setValue(value[i]);
    }
}

bool LimitedDoubleVecWidget::basicWidgetShouldBeVisible(int basicWidgetindex)
{
    if (curMode == Mode::ShowJustLimitedDoubleWidgets)
    {
        return true;
    }
    else if (curMode == Mode::ShowXYPad)
    {
        //temp
        return false;
    }
    else
    {
        SV_UNREACHABLE();
    }
}
