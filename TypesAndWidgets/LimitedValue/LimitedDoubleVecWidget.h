#pragma once
#include "LimitedDoubleWidget.h"
#include "TypesAndWidgets/TypesAndWidgets.h"
#include <QStackedLayout>

class LimitedDoublesWidget;
class XYPadWithPresetsWidget;

//todo on value change check validation

class LimitedDoubleVecWidget : public QFrame
{
    Q_OBJECT
public:
    enum Mode
    {
        ShowJustLimitedDoubleWidgets,
        ShowXYPad
    };

    LimitedDoubleVecWidget(const LimitedDoubleVec& vec, QWidget *parent = nullptr);

    const LimitedDoubleVec& getValue() const;

    //may add or remove widgets based on difference between current value and new value
    void setValue(const LimitedDoubleVec& newValue);

signals:
    void valueChanged(const LimitedDoubleVec &val);

private:
    //no signals will be emitted from views
    void setViewsStateFromValue(const LimitedDoubleVec& value);

private:
    LimitedDoubleVec value;
    QVBoxLayout*            layout          = nullptr;
    LimitedDoublesWidget*       slidersView = nullptr;
    XYPadWithPresetsWidget*     xyPadView   = nullptr;
};