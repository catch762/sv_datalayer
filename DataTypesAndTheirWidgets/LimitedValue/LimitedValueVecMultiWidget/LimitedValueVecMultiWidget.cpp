#include "LimitedValueVecMultiWidget.h"
#include "LimitedValueVecAtomicWidgets/LimitedValueVecSlidersWidget.h"
#include "LimitedValueVecAtomicWidgets/XYPadWithPresetsWidget.h"
#include "WidgetLogic/NodeWidget.h"
#include <QToolButton>


LimitedValueVecMultiWidget::LimitedValueVecMultiWidget(const LimitedIntOrDoubleVec& initialValue,
    const WidgetOptionsJsonOpt& options, QWidget *parent)
    : QFrame(parent)
{
    value = initialValue;

    layout = new QVBoxLayout(this);
    initLayoutSpacing(layout, 2, 0);

    slidersView = new LimitedValueVecSlidersWidget(value, this);
    xyPadView   = new XYPadWithPresetsWidget(this);

    visitAllAtomicWidgets([&](ILimitedValueVecAtomicWidget* atomicWidget)
    {
        layout->addWidget(atomicWidget);

        if (options)
        {
            atomicWidget->applyOptions(*options);
        }

        connect(atomicWidget, &ILimitedValueVecAtomicWidget::valueChanged, this, &LimitedValueVecMultiWidget::setValue);

        bool initialVisible = atomicWidget == slidersView;
        atomicWidget->setVisible(initialVisible);
    });
}

const LimitedIntOrDoubleVec& LimitedValueVecMultiWidget::getValue() const
{
    return value;
}

void LimitedValueVecMultiWidget::setValue(const LimitedIntOrDoubleVec& newValue)
{
    //static int i = 0;
    //SV_LOG("Master: setValue " + std::to_string(i++));

    value = newValue;

    setViewsStateFromValue(value);

    emit valueChanged(value);

    if (std::holds_alternative<LimitedIntVec>(value))
    {
        emit intValueChanged(std::get<LimitedIntVec>(value));
    }
    else if (std::holds_alternative<LimitedDoubleVec>(value))
    {
        emit doubleValueChanged(std::get<LimitedDoubleVec>(value));
    }
    else
    {
        SV_UNREACHABLE();
    }
}

WidgetOptionsJsonOpt LimitedValueVecMultiWidget::makeOptions() const
{
    //this is controversial: merging all unknown widget type options into one flat object...
    //but i dont want to further complicate it

    WidgetOptionsJson options = {};
    visitAllAtomicWidgetsConst([&](const auto* atomicWidget)
    {
        if (auto duplicateKeyErr = mergeJsonObjectsWithUniqueKeys(options, atomicWidget->makeOptions()))
        {
            SV_ERROR(*duplicateKeyErr);
        }
    });

    if (viewSelectorWrapperButton)
    {
        options[modeIsXYKey] = viewSelectorWrapperButton->isChecked();
    }

    return options;
}

void LimitedValueVecMultiWidget::setupButtonsOnWrapperParent(NodeWidget *wrapper, const WidgetOptionsJsonOpt& options)
{
    viewSelectorWrapperButton = makeTopStripeCheckableButtonWithIcon(QIcon::ThemeIcon::MediaPlaybackStop,
                                                                     QIcon::ThemeIcon::FormatJustifyLeft);
    viewSelectorWrapperButton->setChecked(false);

    connect(viewSelectorWrapperButton, &QPushButton::toggled, this, [this](bool checked)
    {
        setMode(checked ? Mode::ShowXYPad : Mode::ShowJustLimitedValueWidgets);
    });

    wrapper->getStripeButtonsLayout()->addWidget(viewSelectorWrapperButton);

    if (options)
    {
        if (auto modeIsXY = getFromJson<bool>(*options, modeIsXYKey))
        {
            viewSelectorWrapperButton->setChecked(*modeIsXY);
        }
    }
}

void LimitedValueVecMultiWidget::setViewsStateFromValue(const LimitedIntOrDoubleVec& value)
{
    visitAllAtomicWidgets([&](ILimitedValueVecAtomicWidget* atomicWidget)
    {
        QSignalBlocker blocker(atomicWidget);
        atomicWidget->setValue(value);
    });
}

void LimitedValueVecMultiWidget::setMode(Mode mode)
{
    auto setWrapperButtonChecked = [this](bool checked)
    {
        if (viewSelectorWrapperButton)
        {
            QSignalBlocker blocker(viewSelectorWrapperButton);
            viewSelectorWrapperButton->setChecked(checked);
        }
    };

    if (mode == Mode::ShowJustLimitedValueWidgets)
    {
        setOnlyThisAtomicWidgetVisible(slidersView);

        setWrapperButtonChecked(false);
    }
    else
    {
        setOnlyThisAtomicWidgetVisible(xyPadView);

        setWrapperButtonChecked(true);
    }
}

void LimitedValueVecMultiWidget::setOnlyThisAtomicWidgetVisible(ILimitedValueVecAtomicWidget* atomicWidgetToMakeVisible)
{
    visitAllAtomicWidgets([&](auto* atomicWidget)
    {
        bool shouldBeVisible = atomicWidget == atomicWidgetToMakeVisible;
        atomicWidget->setVisible(shouldBeVisible);
    });
}

LimitedValueVecMultiWidget::Mode LimitedValueVecMultiWidget::getMode() const
{
    SV_ASSERT(viewSelectorWrapperButton);
    return viewSelectorWrapperButton->isChecked() ? Mode::ShowXYPad : Mode::ShowJustLimitedValueWidgets;
}


void LimitedValueVecMultiWidget::visitAllAtomicWidgets(const AtomicWidgetVisitor& visitor)
{
    visitor(slidersView);
    visitor(xyPadView);
}

void LimitedValueVecMultiWidget::visitAllAtomicWidgetsConst(const AtomicWidgetVisitorConst& visitor) const
{
    visitor(slidersView);
    visitor(xyPadView);
}