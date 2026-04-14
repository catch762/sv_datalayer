#pragma once
#include "../LimitedValueWidget.h"
#include "DataTypesAndTheirWidgets/DataTypesAndTheirWidgets.h"

class BaseXYPadWidget;
class XYPadWithPresetsWidget;

// Simplest widget for LimitedDoubleVec: just creates N LimitedValueWidget's in a layout 
class LimitedDoublesWidget : public QFrame
{
    Q_OBJECT
public:
    LimitedDoublesWidget(const LimitedDoubleVec& vec, QWidget *parent = nullptr);

    const LimitedDoubleVec& getValue() const;

    //may add or remove widgets based on difference between current value and new value
    void setValue(const LimitedDoubleVec& newValue);

signals:
    void valueChanged(const LimitedDoubleVec &val);

private slots:
    void onSomethingChanged();

private:
    //May resize value, if widgets count is different from value size
    void setCurrentValueFromWidgetsState();

    //Simply makes sure there are N basicWidgets now: deletes unneeded widgets or adds new ones, if needed.
    //Added basicWidgets remain with their default value, but their visibility
    //in regards to 'curMode' is set appropriately.
    void setBasicWidgetsCount(int requiredBasicWidgetsCount);

    //After this call, widgets state will exactly match the argument.
    //Therefore widgets count may change.
    //All signals from widgets are blocked during this operation.
    void setWidgetsStateFromValue(const LimitedDoubleVec& value);

private:
    LimitedDoubleVec value;

    QVBoxLayout*                        basicWidgetsLayout = nullptr;
    std::vector<LimitedValueWidget*>   basicWidgets;
};