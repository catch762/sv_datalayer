#include "LimitedDoubleWidget.h"

LimitedDoubleWidget::LimitedDoubleWidget(const LimitedDouble &initialValue, QWidget *parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setSpacing(2);
    layout->setContentsMargins(2,2,2,2);
    
    slider = new QSlider(this);
    {
        slider->setMinimum(0);
        slider->setMaximum(10'000);
        slider->setTickInterval(1);

        connect(slider, &QSlider::valueChanged, this, &LimitedDoubleWidget::onSomethingChanged);
    }

    spinboxLeftLimit = createSpinbox();

    spinboxRightLimit = createSpinbox();

    layout->addWidget(spinboxLeftLimit);
    layout->addWidget(slider);
    layout->addWidget(spinboxRightLimit);
}

QDoubleSpinBox *LimitedDoubleWidget::createSpinbox()
{
    auto* spinbox = new QDoubleSpinBox(this);
    spinbox->setButtonSymbols(QAbstractSpinBox::NoButtons);
    spinbox->setDecimals(3);
    spinbox->setSingleStep(0.1);
    spinbox->setRange(MinFloatInUI, MaxFloatInUI);

    connect(spinbox, &QDoubleSpinBox::valueChanged, this, &LimitedDoubleWidget::onSomethingChanged);

    return spinbox;
}

void LimitedDoubleWidget::onSomethingChanged()
{
    emit valueChanged(currentValue());
}

LimitedDouble LimitedDoubleWidget::currentValue()
{
    double left     = spinboxLeftLimit->value();
    double right    = spinboxRightLimit->value();
    double value    = mix(left, right, getSliderValue01(slider));

    return LimitedDouble(value, left, right);
}