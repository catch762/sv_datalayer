#include "XYPadWithPresetsWidget.h"
#include "BaseXYPadWidget.h"
#include "../LimitedDoubleWidget.h"
#include "../LimitedDoubleVecWidget.h"
#include <QPainter>

namespace
{
    const int PresetsWidth = 24;
    const int CurrentPresetTabHeight = 24;
    const char* ButtonPresetIsValidProperty = "PresetIsValid";
}

void setBackgroundColor(QPushButton *btn, QColor color)
{
    if (!btn || !color.isValid())
    {
        return;
    }

    const static QString styleString = "QPushButton { background-color: %1; border: none; }";

    QString styleSheet = styleString.arg(color.name(QColor::HexRgb));  // Use HexRgb for #RRGGBB format
    btn->setStyleSheet(styleSheet);
}

//****************************//
//  [class XYPadForPresets]   //
//****************************//

XYPadForPresets::XYPadForPresets(XYPadWithPresetsWidget *theParent) : BaseXYPadWidget(theParent)
{
    parent = theParent;
}

void XYPadForPresets::paintEvent(QPaintEvent *event)
{
    BaseXYPadWidget::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPen pen;
    pen.setCapStyle(Qt::RoundCap);
    
    parent->iterateValidPresetPoints([&](const LimitedDoublePair &point, int presetIndex, int presetIsSelected)
    {
        double x11 = value01To11( point.first.getValue01() );
        double y11 = value01To11( point.second.getValue01() );

        QPointF pixCoord = coord11ToPixcoord(QPointF(x11, y11));

        auto colorData = parent->colorsForPreset(presetIndex, true);
        auto color = presetIsSelected ? colorData.selectedColor : colorData.notSelectedColor;

        pen.setWidthF(presetIsSelected ? 6.0 : 5.0);
        pen.setColor(color);
        painter.setPen(pen);

        painter.drawPoint(pixCoord);

        if (presetIsSelected)
        {
            auto midPoint = coordinateArea().center();

            pen.setWidthF(1.0);
            pen.setStyle(Qt::DashLine);
            painter.setPen(pen);

            painter.drawLine(midPoint, pixCoord);
        }
    });
}

//****************************//
//  [/class XYPadForPresets]  //
//****************************//




XYPadWithPresetsWidget::XYPadWithPresetsWidget(LimitedDoubleVecWidget *theParent) : QWidget(theParent), parent(theParent)
{
    layout = new QGridLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);

    paramX = new LimitedDoubleWidget({}, this);
    paramY = new LimitedDoubleWidget({}, this);
    {
        auto onSliderRepresentationChange = [this]()
        {
            auto point = tryGetPointFromPreset(currentPresetSaveData());
            if (!point)
            {
                return;
            }

            point->first.setValue11 ( paramX->currentValue().getValue11() );
            point->second.setValue11( paramY->currentValue().getValue11() );

            onXYRepresentationChanged(*point);
        };

        connect(paramX, &LimitedDoubleWidget::valueChanged, this, onSliderRepresentationChange);
        connect(paramY, &LimitedDoubleWidget::valueChanged, this, onSliderRepresentationChange);

        layout->addWidget(paramX, 0, 0, 1, 2);
        layout->addWidget(paramY, 1, 0, 1, 2);


    }

    xyPad = new XYPadForPresets(this);
    {
        layout->addWidget(xyPad, 2, 0);

        auto onPadRepresentationChange = [this](QPointF coord11)
        {
            auto point = tryGetPointFromPreset(currentPresetSaveData());
            if (!point)
            {
                return;
            }

            point->first.setValue11 (coord11.x());
            point->second.setValue11(coord11.y());

            onXYRepresentationChanged(*point);
        };

        connect(xyPad, &BaseXYPadWidget::positionChanged, this, onPadRepresentationChange);
    }

    presetsWidget = new QWidget(this);
    {
        presetsLayout = new QVBoxLayout(presetsWidget);
        presetsLayout->setContentsMargins(0,0,0,0);
        presetsLayout->setSpacing(0);

        presetsWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
        //presetsWidget->setFixedWidth(PresetsWidth);

        for (int presetIdx = 0; presetIdx < PresetsCount; ++presetIdx)
        {
            auto presetBtn = new QPushButton(QString::number(presetIdx+1), presetsWidget);

            QString style = "QPushButton {border: none;border-radius: 0;padding: 1;margin: 0;}"
                "QPushButton:checked {border: 1px solid palette(highlight);border-radius: 0;}";

            presetBtn->setCheckable(true);
            presetBtn->setChecked(presetIdx==0);
            presetBtn->setAutoExclusive(true);

            presetBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
            presetBtn->setFixedWidth(PresetsWidth);
            presetBtn->setMinimumHeight(15);
            presetBtn->setFlat(true);

            //sets 'invalid preset' background color
            updatePresetButtonIfNeeded(presetBtn, presetIdx);

            connect(presetBtn, &QPushButton::toggled, this, [this, presetIdx](bool checked)
            {
                if (checked)
                {
                    onPresetSelected(presetIdx);
                }
            });

            presetsLayout->addWidget(presetBtn);
            presetsButtons.push_back(presetBtn);
        }

        layout->addWidget(presetsWidget, 2, 1);
    }

    currentPresetWidget = new QWidget(this);
    {
        currentPresetLayout = new QHBoxLayout(currentPresetWidget);

        currentPresetWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        currentXIndex = new QSpinBox(this);
        currentYIndex = new QSpinBox(this);
        {
            auto init = [this](auto *spinbox)
            {
                spinbox->setMinimum(-1);
                spinbox->setValue(-1);

                connect(spinbox, &QSpinBox::valueChanged, this, &XYPadWithPresetsWidget::updateEverythingToMatchParentValue);

                currentPresetLayout->addWidget(spinbox);
            };
            init(currentXIndex);
            init(currentYIndex);
        }


        layout->addWidget(currentPresetWidget, 3, 0, 1, 2);
    }

    
    // After adding all widgets to layout:
    //layout->setColumnStretch(0, 1);  // Let xyPad column expand, fixed column 1 stays narrow
    //layout->setRowStretch(0, 0);     // Row 0 minimum, row 1 expands if needed
}

void XYPadWithPresetsWidget::onPresetSelected(int presetIdx)
{
    currentPresetIdx = presetIdx;

    //i want to do all the remaining work in 'updateEverythingToMatchParentValue()'
    //that would be triggered after 'setValue':
    //so blocking one of widgets just so we dont do exact same thing twice
    QSignalBlocker blockFirst(currentXIndex);
    currentXIndex->setValue(currentPresetSaveData().xIndex.value_or(-1));
    currentYIndex->setValue(currentPresetSaveData().yIndex.value_or(-1));

    update();
}

XYPadWithPresetsWidget::PresetData& XYPadWithPresetsWidget::currentPresetSaveData()
{
    SV_ASSERT(currentPresetIdx >= 0 && currentPresetIdx < PresetsCount);

    return presets[currentPresetIdx];
}

bool XYPadWithPresetsWidget::componentIndexIsValid(int componentIndex)
{
    return componentIndex >= 0 && componentIndex < parent->getValue().size();
}

bool XYPadWithPresetsWidget::presetIsValid(const PresetData &preset)
{
    return preset.hasValues() && componentIndexIsValid(*preset.xIndex) && componentIndexIsValid(*preset.yIndex);
}

LimitedDoublePairOpt XYPadWithPresetsWidget::tryGetPointFromPreset(const PresetData &preset)
{
    if (presetIsValid(preset))
    {
        const auto &valueVector = parent->getValue();
        return LimitedDoublePair{valueVector[*preset.xIndex], valueVector[*preset.yIndex]};
    }

    return {};
}

void XYPadWithPresetsWidget::setPresetButtonStylesheetAndColors(QPushButton *btn, ColorData colors)
{
    if (!btn || !colors.isValid())
    {
        SV_ERROR("setButtonStylesheetAndColors: wrong arguments");
        return;
    }

    static const QString stylePattern = 
        "QPushButton         {background-color: %1; border: none; border-radius: 0; padding: 1; margin: 0;}"
        "QPushButton:checked {background-color: %2; border: 1px solid rgb(37, 37, 37); border-radius: 0;}"
        "QPushButton:hovered {background-color: %3; border: none; border-radius: 0; padding: 1; margin: 0;}";

    QString style = stylePattern.arg(colors.notSelectedColor.name(QColor::HexArgb))
                                .arg(colors.selectedColor.name(QColor::HexArgb))
                                .arg(colors.selectedColor.name(QColor::HexArgb));
    btn->setStyleSheet(style);
}

void XYPadWithPresetsWidget::updatePresetButtonIfNeeded(QPushButton *btn, int index)
{
    bool isValidNow = presetIsValid(presets[index]);

    auto getIsValidProp = [](QPushButton* btn) -> boolOpt
    {
        auto prop = btn->property(ButtonPresetIsValidProperty);

        if (prop.isValid() && prop.canConvert<bool>())
        {
            return prop.toBool();
        }
        else return {};
    };

    boolOpt isValidSavedValue = getIsValidProp(btn);

    if (isValidSavedValue && *isValidSavedValue == isValidNow)
    {
        //nothing changed.
        return;
    }

    //SV_LOG("Updating prop");

    btn->setProperty(ButtonPresetIsValidProperty, isValidNow);

    setPresetButtonStylesheetAndColors(btn, colorsForPreset(index, isValidNow));
}

XYPadWithPresetsWidget::ColorData XYPadWithPresetsWidget::colorsForPreset(int presetIdx, bool isValid)
{
    auto makeColorData = [](QColor selectedColor)
    {
        QColor notSelectedColor = selectedColor;
        notSelectedColor.setAlphaF(0.3);

        return ColorData{selectedColor, notSelectedColor};
    };

    static const ColorData presetColors[PresetsCount] = {
        makeColorData(QColor(227,  52,  47)),  // red
        makeColorData(QColor( 52, 144, 220)),  // blue
        makeColorData(QColor( 56, 193, 114)),  // green
        makeColorData(QColor(149,  97, 228)),  // purple
        makeColorData(QColor(246, 153,  63)),  // orange
        makeColorData(QColor(255, 206,  74).darker(100)),  // yellow
        makeColorData(QColor( 74, 204, 204)),  // cyan
        makeColorData(QColor(101, 116, 205)),  // indigo
        makeColorData(QColor(246, 109, 155))   // pink
    };
    static const ColorData unavailableColor = makeColorData(QColor(205, 205, 205));

    if (isValidIndex(presetIdx, PresetsCount) && isValid)
    {
        return presetColors[presetIdx];
    }
    else return unavailableColor;
}

void XYPadWithPresetsWidget::iterateValidPresetPoints(std::function<void(const LimitedDoublePair &xy,
                                                                         int presetIndex,
                                                                         bool presetIsSelected)> visitor)
{
    for (int presetIndex = 0; presetIndex < presets.size(); ++presetIndex)
    {
        const auto &preset = presets[presetIndex];

        if (auto point = tryGetPointFromPreset(preset))
        {
            visitor(*point, presetIndex, presetIndex == currentPresetIdx);
        }
    }
}


void XYPadWithPresetsWidget::updateEverythingToMatchParentValue()
{
    const auto &vectorValue = parent->getValue();

    auto getIndexOptAndHandleInvalid = [this](QSpinBox* indexSpinbox) -> intOpt
    {
        int index = indexSpinbox->value();
        if (componentIndexIsValid(index))
        {
            return index;
        }
        else
        {
            // Currently this is not really needed for anything. I just feel like it:
            const int DefaultInvalidValue = -1;
            if (index != DefaultInvalidValue)
            {
                QSignalBlocker blocker(indexSpinbox);
                indexSpinbox->setValue(DefaultInvalidValue);
            }

            return {};
        }
    };

    intOpt xIndexOpt = getIndexOptAndHandleInvalid(currentXIndex);
    intOpt yIndexOpt = getIndexOptAndHandleInvalid(currentYIndex);

    currentPresetSaveData() = {xIndexOpt, yIndexOpt};

    auto setupSliderWidgetForIndex = [&](LimitedDoubleWidget* sliderParam, intOpt indexOpt)
    {
        auto isValidIdx = indexOpt && componentIndexIsValid(*indexOpt);

        sliderParam->setEnabled(isValidIdx);

        QSignalBlocker block(sliderParam);
        sliderParam->setValue(isValidIdx ? vectorValue[*indexOpt] : LimitedDouble{});
    };

    setupSliderWidgetForIndex(paramX, xIndexOpt);
    setupSliderWidgetForIndex(paramY, yIndexOpt);

    for (int i = 0; i < PresetsCount; ++i)
    {
        auto button = presetsButtons[i];
        updatePresetButtonIfNeeded(button, i);
    }

    //for xypad and preset buttons to repaint themselves
    update();
}

void XYPadWithPresetsWidget::onXYRepresentationChanged(const LimitedDoublePair &point)
{
    auto getNewValueOpt = [&]() -> LimitedDoubleVecOpt
    {
        const auto &preset = currentPresetSaveData();

        LimitedDoubleVec value = parent->getValue();

        if (!presetIsValid(preset))
        {
            SV_WARN("current preset not valid, but representation got changed somehow, this shouldnt happen");
            return {};
        }

        value[*preset.xIndex] = point.first;
        value[*preset.yIndex] = point.second;

        return value;
    };

    LimitedDoubleVecOpt newValue = getNewValueOpt();
    if (!newValue) return;

    //note that we are not changing 'other representation' of this widget:
    //we just set it on parent, and parent will synchronize everything
    parent->setValue(*newValue);
}
