#include "LimitedDoubleWidget.h"

LimitedDoubleWidget::LimitedDoubleWidget(const LimitedDouble &initialValue, QWidget *parent) : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setSpacing(2);
    layout->setContentsMargins(1,1,1,1);
    
    sliderValueLeftToRight = new QSlider(Qt::Horizontal, this);
    {
        sliderValueLeftToRight->setMinimumWidth(30);

        sliderValueLeftToRight->setMinimum(0);
        sliderValueLeftToRight->setMaximum(10'000);
        sliderValueLeftToRight->setTickInterval(1);

        connect(sliderValueLeftToRight, &QSlider::valueChanged, this, [this]()
        {
            onSomethingChanged(sliderValueLeftToRight);    
        });
    }

    spinboxValue        = createSpinbox();
    spinboxLeftLimit    = createSpinbox();
    spinboxRightLimit   = createSpinbox();

    layout->addWidget(spinboxValue);
    layout->addWidget(sliderValueLeftToRight);
    layout->addWidget(spinboxLeftLimit);
    layout->addWidget(spinboxRightLimit);

    setValue(initialValue);
}

QDoubleSpinBox *LimitedDoubleWidget::createSpinbox()
{
    auto* spinbox = new QDoubleSpinBox(this);
    spinbox->setKeyboardTracking(false);
    spinbox->setMaximumWidth(60);
    spinbox->setButtonSymbols(QAbstractSpinBox::NoButtons);
    spinbox->setDecimals(3);
    spinbox->setSingleStep(0.1);
    spinbox->setRange(MinFloatInUI, MaxFloatInUI);

    connect(spinbox, &QDoubleSpinBox::valueChanged, this, [this, spinbox]()
    {
        onSomethingChanged(spinbox);    
    });

    return spinbox;
}

void LimitedDoubleWidget::onSomethingChanged(QWidget *changedWidget)
{
    if (changedWidget == sliderValueLeftToRight)
    {
        const QSignalBlocker blocker(spinboxValue);
        spinboxValue->setValue( getValueBasedOnSlider() );
    }
    else
    {
        {
            //something changed and we need to constrain spinboxValue to limits
            const QSignalBlocker blocker(spinboxValue);
            spinboxValue->setValue( currentValue().value() );
        }

        const QSignalBlocker blocker(sliderValueLeftToRight);
        setSliderValue01(sliderValueLeftToRight, getValue01BasedOnSpinboxes());
    }


    //SV_LOG(("Changed: " + currentValue().toString()).toStdString());
    emit valueChanged(currentValue());
}

LimitedDouble LimitedDoubleWidget::currentValue() const
{
    double left     = spinboxLeftLimit->value();
    double right    = spinboxRightLimit->value();
    double value    = spinboxValue->value();

    return LimitedDouble(value, left, right);
}

void LimitedDoubleWidget::setValue(const LimitedDouble &value)
{
    {
        QSignalBlocker  a(spinboxLeftLimit),
                        b(spinboxRightLimit),
                        c(spinboxValue),
                        d(sliderValueLeftToRight);

        spinboxLeftLimit->setValue(value.left());
        spinboxRightLimit->setValue(value.right());
        spinboxValue->setValue(value.value());
        setSliderValue01(sliderValueLeftToRight, getValue01BasedOnSpinboxes());
    }

    //SV_LOG(("Set: " + currentValue().toString()).toStdString());
    emit valueChanged(currentValue());
}

double LimitedDoubleWidget::getValue01BasedOnSpinboxes() const
{
    return getValue01Clamped(spinboxValue->value(),
                             spinboxLeftLimit->value(),
                             spinboxRightLimit->value());
}

double LimitedDoubleWidget::getValueBasedOnSlider() const
{
    return mix( spinboxLeftLimit->value(),
                spinboxRightLimit->value(),
                getSliderValue01(sliderValueLeftToRight) );
}
