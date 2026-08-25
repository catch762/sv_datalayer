#pragma once
#include "../LimitedValueWidget.h"
#include "DataTypesAndTheirWidgets/DataTypesAndTheirWidgets.h"
#include <QStackedLayout>
#include "WidgetLogic/WidgetDefs.h"

class LimitedValueVecSlidersWidget;
class ILimitedValueVecAtomicWidget;
class XYPadWithPresetsWidget;
class NodeWidget;

//********************************************************
//
// THE SITUATION:
//
// There are various "essentially just sequence of numbers" types and different user widgets
// for them, like plain QSliders, or a color picker widget, or a XY pad, etc. Big variety.
// 
// Also, it would be great if changing widget could work as easy as possible without recreating
// the data. 
// 
// So it kinda screams "use vector of numbers as the model, and different widgets as easily changeable views".
// 
// 
//*********************************************************


class LimitedValueVecMultiWidget : public QFrame
{
    Q_OBJECT
public:
    enum Mode
    {
        ShowJustLimitedValueWidgets,
        ShowXYPad
    };

    LimitedValueVecMultiWidget(const LimitedIntOrDoubleVec& vec,
                           const WidgetOptionsJsonOpt& options = {},
                           QWidget *parent = nullptr);

    const LimitedIntOrDoubleVec& getValue() const;

    //may add or remove widgets based on difference between current value and new value
    void setValue(const LimitedIntOrDoubleVec& newValue);

    WidgetOptionsJsonOpt makeOptions() const;
    void setupButtonsOnWrapperParent(NodeWidget* wrapper, const WidgetOptionsJsonOpt& options = {} );

    void setMode(Mode mode);
    Mode getMode() const;

signals:
    void valueChanged       (const LimitedIntOrDoubleVec &val);
    void intValueChanged    (const LimitedIntVec& val);
    void doubleValueChanged (const LimitedDoubleVec& val);

private:
    //no signals will be emitted from views
    void setViewsStateFromValue(const LimitedIntOrDoubleVec& value);
    
    using AtomicWidgetVisitor       = std::function<void(      ILimitedValueVecAtomicWidget* atomicWidget)>;
    using AtomicWidgetVisitorConst  = std::function<void(const ILimitedValueVecAtomicWidget* atomicWidget)>;
    void visitAllAtomicWidgets                   (const AtomicWidgetVisitor&      visitor);
    void visitAllAtomicWidgetsConst              (const AtomicWidgetVisitorConst& visitor) const;

    void setOnlyThisAtomicWidgetVisible(ILimitedValueVecAtomicWidget* atomicWidgetToMakeVisible);

private:
    LimitedIntOrDoubleVec value;

    QVBoxLayout*                    layout          = nullptr;
    ILimitedValueVecAtomicWidget*       slidersView = nullptr;
    ILimitedValueVecAtomicWidget*       xyPadView   = nullptr;

    // Button to select views, it will be created on a NodeWidget, but stored here.
    // default state: 'checked = false' which means slidersView is visible
    QPointer<QPushButton> viewSelectorWrapperButton;

    static inline const QString modeIsXYKey = "modeIsXY";
};

Q_DECLARE_METATYPE(LimitedValueVecMultiWidget*);