#include "LimitedIntWidget.h"

LimitedIntWidget::LimitedIntWidget(const LimitedInt &initialValue, QWidget *parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);

    slider = new QSlider(this);
    {
        slider->setMinimum(0);
        slider->setMaximum(10'000);
        slider->setTickInterval(1);

        connect(slider, &QSlider::valueChanged, this, &LimitedIntWidget::onSomethingChanged);
    }

    spinboxLeftLimit = createSpinbox();

    spinboxRightLimit = createSpinbox();

    layout->addWidget(spinboxLeftLimit);
    layout->addWidget(slider);
    layout->addWidget(spinboxRightLimit);
}

QSpinBox *LimitedIntWidget::createSpinbox()
{
    auto* spinbox = new QSpinBox(this);
    spinbox->setButtonSymbols(QAbstractSpinBox::NoButtons);
    spinbox->setSingleStep(1);
    spinbox->setRange(MinIntInUI, MaxIntInUI);

    connect(spinbox, &QSpinBox::valueChanged, this, &LimitedIntWidget::onSomethingChanged);

    return spinbox;
}

void LimitedIntWidget::onSomethingChanged()
{
    emit valueChanged(currentValue());
}

LimitedInt LimitedIntWidget::currentValue()
{
    int left     = spinboxLeftLimit->value();
    int right    = spinboxRightLimit->value();
    int value    = mix(left, right, getSliderValue01(slider));

    return LimitedInt(value, left, right);
}