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
    void onSomethingChanged();
    LimitedDouble currentValue();

private:
    QDoubleSpinBox* spinboxLeftLimit    = nullptr;
    QDoubleSpinBox* spinboxRightLimit   = nullptr;
    QSlider*        slider              = nullptr;
};