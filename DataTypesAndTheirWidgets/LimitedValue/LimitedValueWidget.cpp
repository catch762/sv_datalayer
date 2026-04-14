#include "LimitedValueWidget.h"
#include "DataLayerUtils.h"

LimitedValueWidget::LimitedValueWidget(const LimitedIntOrDouble &initialValue, QWidget *parent)
    //todo: figure out why doesnt compile without 'spinboxes{Spinboxes<double>()}' 
 : QWidget(parent), isDouble(std::holds_alternative<LimitedDouble>(initialValue)), spinboxes{Spinboxes<double>()}
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setSpacing(2);
    layout->setContentsMargins(1,1,1,1);
    
    sliderValueLeftToRight = new QSlider(Qt::Horizontal, this);
    {
        sliderValueLeftToRight->setMinimumWidth(30);
        sliderValueLeftToRight->setTickInterval(1);

        //this is only for double:
        sliderValueLeftToRight->setMinimum(0);
        sliderValueLeftToRight->setMaximum(10'000);

        connect(sliderValueLeftToRight, &QSlider::valueChanged, this, [this]()
        {
            onSomethingChanged(sliderValueLeftToRight);    
        });
    }

    spinboxes = createSpinboxes();

    std::visit([&](auto&& spinboxes)
    {
        layout->addWidget(spinboxes.spinboxValue);
        layout->addWidget(sliderValueLeftToRight);
        layout->addWidget(spinboxes.spinboxLeftLimit);
        layout->addWidget(spinboxes.spinboxRightLimit);
    },
    spinboxes);

    setValue(initialValue);
}

QDoubleSpinBox *LimitedValueWidget::createDoubleSpinbox()
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

QSpinBox *LimitedValueWidget::createIntSpinbox()
{
    auto* spinbox = new QSpinBox(this);
    spinbox->setKeyboardTracking(false);
    spinbox->setMaximumWidth(60);
    spinbox->setButtonSymbols(QAbstractSpinBox::NoButtons);

    spinbox->setSingleStep(1);
    spinbox->setRange(MinIntInUI, MaxIntInUI);

    connect(spinbox, &QSpinBox::valueChanged, this, [this, spinbox]()
    {
        onSomethingChanged(spinbox); 
    });

    return spinbox;
}

LimitedValueWidget::DoubleOrIntSpinboxes LimitedValueWidget::createSpinboxes()
{
    if (isDouble)
    {
        DoubleSpinboxes spinboxes{
            createDoubleSpinbox(),
            createDoubleSpinbox(),
            createDoubleSpinbox()
        };

        return spinboxes;
    }
    else
    {
        IntSpinboxes spinboxes{
            createIntSpinbox(),
            createIntSpinbox(),
            createIntSpinbox()
        };

        return spinboxes;
    }
}

const LimitedValueWidget::DoubleSpinboxes& LimitedValueWidget::getDoubleSpinboxes() const
{
    return std::get<DoubleSpinboxes>(spinboxes);
}

const LimitedValueWidget::IntSpinboxes& LimitedValueWidget::getIntSpinboxes() const
{
    return std::get<IntSpinboxes>(spinboxes);
}

LimitedValueWidget::DoubleSpinboxes& LimitedValueWidget::getDoubleSpinboxes() 
{
    return const_cast<DoubleSpinboxes&>(
        static_cast<const LimitedValueWidget*>(this)->getDoubleSpinboxes()
    );
}

LimitedValueWidget::IntSpinboxes& LimitedValueWidget::getIntSpinboxes() 
{
    return const_cast<IntSpinboxes&>(
        static_cast<const LimitedValueWidget*>(this)->getIntSpinboxes()
    );
}

void LimitedValueWidget::updateLeftToRightSliderBasedOnSpinboxes()
{
    const QSignalBlocker blocker(sliderValueLeftToRight);

    if(!isDouble)
    {
        LimitedInt limInt = currentIntValue();

        //this should work?
        sliderValueLeftToRight->setMinimum(limInt.min());
        sliderValueLeftToRight->setMaximum(limInt.max());
    }

    setSliderValue01(sliderValueLeftToRight, getValue01BasedOnSpinboxes());
}

void LimitedValueWidget::onSomethingChanged(QWidget *changedWidget)
{
    if (changedWidget == sliderValueLeftToRight)
    {
        std::visit([&](auto &&sliderBasedValue)
        {
            using SpinboxesT = Spinboxes<std::decay_t<decltype(sliderBasedValue)>>;

            std::get<SpinboxesT>(spinboxes).setValueSpinbox(sliderBasedValue);
        },
        getValueBasedOnSlider());
    }
    else
    {
        // - read sliders values
        // - construct LimitedValue from them, which keeps LimitedValue.value constrained - thats the whole point
        // - write sliders value from that (potentially fixed) LimitedValue
        std::visit([this](auto&& limitedIntOrDouble)
        {
            using SpinboxesT = Spinboxes<typename std::decay_t<decltype(limitedIntOrDouble)>::UnderlyingType>;

            std::get<SpinboxesT>(spinboxes).setValue(limitedIntOrDouble);
        },
        currentValueVariant());

        updateLeftToRightSliderBasedOnSpinboxes();
    }

    if (isDouble) emit doubleValueChanged(currentDoubleValue());
    else          emit intValueChanged   (currentIntValue   ());
}

LimitedIntOrDouble LimitedValueWidget::currentValueVariant() const
{
    if (isDouble) return currentDoubleValue();
    else          return currentIntValue();
}

LimitedDouble LimitedValueWidget::currentDoubleValue() const
{
    if(!isDouble)
    {
        SV_ERROR("Calling LimitedValueWidget::currentDoubleValue() while its holding LimitedInt");
        return LimitedDouble{};
    }

    return getDoubleSpinboxes().getLimitedValue();
}

LimitedInt LimitedValueWidget::currentIntValue() const
{
    if(isDouble)
    {
        SV_ERROR("Calling LimitedValueWidget::currentIntValue() while its holding LimitedDouble");
        return LimitedInt{};
    }

    return getIntSpinboxes().getLimitedValue();
}

void LimitedValueWidget::setValue(const LimitedIntOrDouble &value)
{
    if (std::holds_alternative<LimitedDouble>(value) != isDouble)
    {
        SV_ERROR("LimitedValueWidget::setValue mismatch");
        return;
    }

    {
        //this also blocks signals, nothing emitted from spinboxes
        if(isDouble) getDoubleSpinboxes ().setValue(std::get<LimitedDouble> (value));
        else         getIntSpinboxes    ().setValue(std::get<LimitedInt>    (value));

        updateLeftToRightSliderBasedOnSpinboxes();
    }

    if (isDouble) emit doubleValueChanged(currentDoubleValue());
    else          emit intValueChanged   (currentIntValue   ());
}

double LimitedValueWidget::getValue01BasedOnSpinboxes() const
{
    return std::visit([](auto &&spinboxes)
    {
        return spinboxes.getValue01();
    },
    spinboxes);
}


doubleOrInt LimitedValueWidget::getValueBasedOnSlider() const
{
    return std::visit([this](auto &&spinboxes)
    {
        double value01 = getSliderValue01(sliderValueLeftToRight);
        return doubleOrInt(spinboxes.getValueFromValue01(value01));
    },
    spinboxes);
}
