#pragma once
#include "../LimitedValue.h"

// Read LimitedValueVecMultiWidget.h, it explains everything

class ILimitedValueVecAtomicWidget : public QWidget
{
    Q_OBJECT
public:
    using QWidget::QWidget;

    virtual const LimitedIntOrDoubleVec& getValue() const = 0;
    virtual void setValue(const LimitedIntOrDoubleVec& newValue) = 0;

signals:
    void valueChanged(const LimitedIntOrDoubleVec& value);
};
