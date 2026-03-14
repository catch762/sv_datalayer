#pragma once
#include "sv_qtcommon.h"
#include "LimitedValue.h"

class LimitedDoubleWidget : public QFrame
{
    Q_OBJECT
public:
    LimitedDoubleWidget(const LimitedDouble &initialValue = {}, QWidget *parent = nullptr);

signals:
    void valueChanged(const LimitedDouble& value);

private:
    QDoubleSpinBox* createSpinbox();

private slots:
    void onSomethingChanged(QWidget *changedWidget);
    LimitedDouble currentValue();
    void setValue(const LimitedDouble &value);

    //e.g.  if spinboxValue is at spinboxLeftLimit  -> 0.0 
    //      if spinboxValue is at spinboxRightLimit -> 1.0 
    double getValue01BasedOnSpinboxes();

    //e.g   if slider is in leftmost position       -> spinboxLeftLimit->value()
    //      if slider is in rightmost position      -> spinboxRightLimit->value()
    double getValueBasedOnSlider();

private:
    QDoubleSpinBox* spinboxValue            = nullptr; // this is the widget holding current Value.
    QSlider*        sliderValueLeftToRight  = nullptr; // this is just representation, how much is Value between Left and Right.
    QDoubleSpinBox* spinboxLeftLimit        = nullptr;
    QDoubleSpinBox* spinboxRightLimit       = nullptr;
};