#pragma once
#include "LimitedDoubleWidget.h"
#include "TypesAndWidgets/TypesAndWidgets.h"

class BaseXYPadWidget;
class XYPresetsWidget;
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

    void setMode(Mode newMode);

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

    //void iterateAllWidgets

    bool basicWidgetShouldBeVisible(int index);

private:
    LimitedDoubleVec value;
    QVBoxLayout*                        layout = nullptr;
    QPushButton*                            tempSwapModesButton = nullptr;
    QVBoxLayout*                            basicWidgetsLayout = nullptr;
    std::vector<LimitedDoubleWidget*>           basicWidgets;
    BaseXYPadWidget*                        xyPad = nullptr;
    XYPresetsWidget*                        xyPadPresets = nullptr;
    
    Mode curMode = Mode::ShowJustLimitedDoubleWidgets;
};