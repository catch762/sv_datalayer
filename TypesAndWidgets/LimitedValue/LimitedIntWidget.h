#pragma once
#include "sv_qtcommon.h"
#include "LimitedValue.h"

class LimitedIntWidget : public QFrame
{
    Q_OBJECT
public:
    LimitedIntWidget(const LimitedInt &initialValue = {}, QWidget *parent = nullptr);

signals:
    void valueChanged(const LimitedInt& value);

private:
    QSpinBox* createSpinbox();

private slots:
    void onSomethingChanged();
    LimitedInt currentValue();

private:
    QSpinBox* spinboxLeftLimit    = nullptr;
    QSpinBox* spinboxRightLimit   = nullptr;
    QSlider*  slider              = nullptr;
};