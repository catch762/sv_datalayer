#include "LimitedDoubleVecWidget.h"
#include "Internal/LimitedDoublesWidget.h"
#include "Internal/XYPadWithPresetsWidget.h"



LimitedDoubleVecWidget::LimitedDoubleVecWidget(const LimitedDoubleVec& initialValue, QWidget *parent)
    : QFrame(parent)
{
    value = initialValue;

    auto tempWrapperLayout = new QVBoxLayout(this);

    layout = new QVBoxLayout();
    tempWrapperLayout->addLayout(layout);

    {
        auto *tempSwapModesButton = new QPushButton("swap", this);
        connect(tempSwapModesButton, &QPushButton::clicked, [this]()
        {
            if (slidersView->isVisible())
            {
                slidersView->setVisible(false);
                xyPadView->setVisible(true);
            }
            else
            {
                xyPadView->setVisible(false);
                slidersView->setVisible(true);
            }
        });
        tempWrapperLayout->addWidget(tempSwapModesButton);
    }

    slidersView = new LimitedDoublesWidget(value, this);
    {
        connect(slidersView, &LimitedDoublesWidget::valueChanged, this, [this](auto &val)
        {
            setValue(val);
        });

        layout->addWidget(slidersView);
    }

    xyPadView = new XYPadWithPresetsWidget(this);
    {
        xyPadView->setVisible(false);
        layout->addWidget(xyPadView);
    }    
}

const LimitedDoubleVec& LimitedDoubleVecWidget::getValue() const
{
    return value;
}

void LimitedDoubleVecWidget::setValue(const LimitedDoubleVec& newValue)
{
    //static int i = 0;
    //SV_LOG("Master: setValue " + std::to_string(i++));

    value = newValue;

    setViewsStateFromValue(value);

    emit valueChanged(value);
}



void LimitedDoubleVecWidget::setViewsStateFromValue(const LimitedDoubleVec& value)
{
    QSignalBlocker blockSliders(slidersView);
    slidersView->setValue(value);

    xyPadView->updateEverythingToMatchParentValue();
}