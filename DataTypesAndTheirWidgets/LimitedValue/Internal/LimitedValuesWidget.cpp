#include "LimitedValuesWidget.h"
#include "DataTypesAndTheirWidgets/LimitedValue/Internal/BaseXYPadWidget.h"
#include "DataTypesAndTheirWidgets/LimitedValue/Internal/XYPadWithPresetsWidget.h"

LimitedValuesWidget::LimitedValuesWidget(const LimitedDoubleVec& initialValue, QWidget *parent)
    : QFrame(parent)
{
    basicWidgetsLayout = new QVBoxLayout(this);
    initLayoutSpacing(basicWidgetsLayout);

    //this will (unless 'initialValue' is empty) create 'basicWidgets' and put them in 'basicWidgetsLayout'
    setValue(initialValue);
}

const LimitedDoubleVec& LimitedValuesWidget::getValue() const
{
    return value;
}

void LimitedValuesWidget::setValue(const LimitedDoubleVec& newValue)
{
    value = newValue;

    setWidgetsStateFromValue(value);

    emit valueChanged(value);
}

void LimitedValuesWidget::onSomethingChanged()
{
    setCurrentValueFromWidgetsState();

    emit valueChanged(value);
}

void LimitedValuesWidget::setCurrentValueFromWidgetsState()
{
    auto valueSize = value.size();
    auto widgetsSize = basicWidgets.size();

    if (valueSize != widgetsSize)
    {
        SV_LOG(std::format("LimitedValuesWidget, set val from widgets: valueSize[{}] but widgetsSize[{}], will resize",
                valueSize, widgetsSize));
        value.resize(widgetsSize);
    }

    SV_ASSERT(value.size() == basicWidgets.size());
    for (int i = 0; i < widgetsSize; ++i)
    {
        value[i] = basicWidgets[i]->currentDoubleValue();
    }
}

void LimitedValuesWidget::setBasicWidgetsCount(int requiredBasicWidgetsCount)
{
    auto existingbasicWidgets = basicWidgets.size();

    if (existingbasicWidgets > requiredBasicWidgetsCount)
    {
        int widgetsToDelete = existingbasicWidgets - requiredBasicWidgetsCount;

        //SV_LOG(std::format("setbasicWidgetsCount will delete [{}] widgets", widgetsToDelete));

        for (int i = 0; i < widgetsToDelete; ++i)
        {
            delete basicWidgets.back();
            basicWidgets.pop_back();
        }
    }
    else if (requiredBasicWidgetsCount > existingbasicWidgets)
    {
        int widgetsToAdd = requiredBasicWidgetsCount - existingbasicWidgets;

        //SV_LOG(std::format("setbasicWidgetsCount will add [{}] widgets", widgetsToAdd));

        for (int i = 0; i < widgetsToAdd; ++i)
        {
            //not even setting value, well do it later
            auto widget = new LimitedValueWidget(LimitedDouble{}, this);

            connect(widget, &LimitedValueWidget::doubleValueChanged, this, &LimitedValuesWidget::onSomethingChanged);

            basicWidgets.push_back(widget);
            basicWidgetsLayout->addWidget(widget);

            int thisBasicIndex = basicWidgets.size() - 1;
        }
    }
    else
    {
        //SV_LOG("setbasicWidgetsCount will do nothing");
    }
}

void LimitedValuesWidget::setWidgetsStateFromValue(const LimitedDoubleVec& value)
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