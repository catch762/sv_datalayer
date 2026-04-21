#pragma once
#include "sv_qtcommon.h"
#include "DataTypesAndTheirWidgets/DataTypesAndTheirWidgets.h"

template<typename T>
concept StrictlyIntOrDouble = std::same_as<T, int> || 
                              std::same_as<T, double>;
class LimitedValueWidget : public QWidget
{
    Q_OBJECT
public:
    LimitedValueWidget(const LimitedIntOrDouble &initialValue, QWidget *parent = nullptr);

    void setValue(const LimitedIntOrDouble &value);

    const LimitedIntOrDouble&      getValue() const;

    double getValue11();

signals:
    void doubleValueChanged(const LimitedDouble& value);
    void intValueChanged(const LimitedInt& value);
    void valueChanged(const LimitedIntOrDouble& value);

private slots:
    void onSomethingChanged(QWidget *changedWidget);

private:
    //e.g.  if spinboxValue is at spinboxLeftLimit  -> 0.0 
    //      if spinboxValue is at spinboxRightLimit -> 1.0 
    double getValue01BasedOnSpinboxes() const;

    //e.g   if slider is in leftmost position       -> spinboxLeftLimit->value()
    //      if slider is in rightmost position      -> spinboxRightLimit->value()
    intOrDouble getValueBasedOnSlider() const;

    void emitValueChangedSignals();
//here's everything regarding difference 'do we hold int or double'    
private:
    template<StrictlyIntOrDouble UnderlyingType>
    struct Spinboxes
    {
        using SpinboxType = std::conditional_t<
            std::is_same_v<UnderlyingType, int>,
                QSpinBox,       //for int
                QDoubleSpinBox  //for double
        >;
        using LimitedType = LimitedValue<UnderlyingType>;

        //Spinboxes() = default;

        SpinboxType* spinboxValue      = nullptr; // this is the widget holding current Value.
        SpinboxType* spinboxLeftLimit  = nullptr;
        SpinboxType* spinboxRightLimit = nullptr;

        LimitedType getLimitedValue() const
        {
            //LimitedType is LimitedDouble or LimitedInt
            return LimitedType(spinboxValue->value(), spinboxLeftLimit->value(), spinboxRightLimit->value());
        }

        LimitedIntOrDouble getLimitedValueVariant() const
        {
            return LimitedIntOrDouble( getLimitedValue() );
        }

        void setValue(const LimitedType& value)
        {
            QSignalBlocker block0(spinboxValue), block1(spinboxLeftLimit), block2(spinboxRightLimit);
            spinboxLeftLimit->setValue  (value.left ());
            spinboxRightLimit->setValue (value.right());
            spinboxValue->setValue      (value.value());
        }

        double getValue01() const
        {
            return getValue01Clamped(spinboxValue->value(),
                                     spinboxLeftLimit->value(),
                                     spinboxRightLimit->value());
        }

        UnderlyingType getValueFromValue01(double value01) const
        {
            return mix( spinboxLeftLimit->value(),
                spinboxRightLimit->value(),
                value01 );
        }

        void setValueSpinbox(UnderlyingType val)
        {
            QSignalBlocker blocker(spinboxValue);
            spinboxValue->setValue(val);
        }
    };
    using DoubleSpinboxes       = Spinboxes<double>;
    using IntSpinboxes          = Spinboxes<int>;
    using DoubleOrIntSpinboxes  = std::variant<DoubleSpinboxes, IntSpinboxes>;

    QDoubleSpinBox*         createDoubleSpinbox();
    QSpinBox*               createIntSpinbox();
    DoubleOrIntSpinboxes    createSpinboxes();

    //these two will assert on mismatch, check 'isDouble' first
    const DoubleSpinboxes&  getDoubleSpinboxes() const;
    const IntSpinboxes&     getIntSpinboxes() const;
    DoubleSpinboxes&        getDoubleSpinboxes();
    IntSpinboxes&           getIntSpinboxes();

    LimitedIntOrDouble      currentValueVariantFromSpinboxes() const;
    //calling wrong version will return {} and log error, thats all.
    LimitedDouble           currentDoubleValueFromSpinboxes() const;
    LimitedInt              currentIntValueFromSpinboxes() const;
    
    void updateLeftToRightSliderBasedOnSpinboxes();
private:
    const bool isDouble;
    
    LimitedIntOrDouble   currentValue;

    DoubleOrIntSpinboxes spinboxes;
    QSlider*             sliderValueLeftToRight  = nullptr; // this is just representation, how much is Value between Left and Right.
    
};

Q_DECLARE_METATYPE(LimitedValueWidget*);