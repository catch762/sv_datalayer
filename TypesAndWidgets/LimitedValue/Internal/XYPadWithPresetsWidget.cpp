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

    const QString presetsKey = "presets";
    const QString currentPresetKey = "curPreset";
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

//***********************//
//  [class PresetData]   //
//***********************//

QJsonArray XYPadWithPresetsWidget::PresetData::toJson() const
{
    QJsonArray arr;
    arr.append(xIndex.value_or(-1));
    arr.append(yIndex.value_or(-1));
    return arr;
}
std::optional<XYPadWithPresetsWidget::PresetData> XYPadWithPresetsWidget::PresetData::fromJson(const QJsonValue& json)
{
    PresetData res;

    const QString err("Deserializing XY PresetData error");
    auto arr = convertJsonAndLogError<QJsonArray>(json, err);
    if (!arr) return {};

    if (arr->size() != 2)
    {
        SV_ERROR(std::format("{}: array size {} instead of 2", err, arr->size()));
        return {};
    }

    //returns success;
    auto putValueTo = [&](const QJsonValue& val, intOpt& intOptDestination)
    {
        doubleOpt index = convertJsonAndLogError<double>(val, err + " in array value");
        if (!index) return false;

        intOptDestination = int(*index);
        return true;
    };

    if (!putValueTo((*arr)[0], res.xIndex)) return {};
    if (!putValueTo((*arr)[1], res.yIndex)) return {};

    return res;
}

bool XYPadWithPresetsWidget::PresetData::hasValues() const
{
    return xIndex && yIndex;
}

//***********************//
//  [/class PresetData]  //
//***********************//

XYPadWithPresetsWidget::XYPadWithPresetsWidget(LimitedDoubleVecWidget *theParent) : QWidget(theParent), parent(theParent)
{
    setFocusPolicy(Qt::ClickFocus);

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

        layout->addWidget(paramX, 0, 2);
        layout->addWidget(paramY, 1, 2);
    }

    {
        QSpacerItem* spacer = new QSpacerItem(0, 4, QSizePolicy::Minimum, QSizePolicy::Fixed);
        layout->addItem(spacer, 2, 0, 1, 2);
    }
    {
        QSpacerItem* spacer = new QSpacerItem(4, 0, QSizePolicy::Fixed, QSizePolicy::Minimum);
        layout->addItem(spacer, 0, 1, 3, 1);
    }


    xyPad = new XYPadForPresets(this);
    {
        layout->addWidget(xyPad, 3, 2, 1, 1);

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
        presetsLayout->setContentsMargins(0,3,0,3);
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

        layout->addWidget(presetsWidget, 3, 0);
    }

    {
        auto makeIndexSpinbox = [this]()
        {
            QSpinBox *spinbox = new QSpinBox(this);

            spinbox->setFixedWidth(PresetsWidth);
            spinbox->setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);

            spinbox->setMinimum(-1);
            spinbox->setValue(-1);

            connect(spinbox, &QSpinBox::valueChanged, this, &XYPadWithPresetsWidget::updateEverythingToMatchParentValue);

            return spinbox;
        };

        currentXIndex = makeIndexSpinbox();
        currentYIndex = makeIndexSpinbox();

        layout->addWidget(currentXIndex, 0, 0);
        layout->addWidget(currentYIndex, 1, 0);
    }

    //now that everything else is created, we update paramX and paramY
    {
        setupSliderWidgetForIndex(paramX, getIndexOptAndHandleInvalid(currentXIndex));
        setupSliderWidgetForIndex(paramY, getIndexOptAndHandleInvalid(currentYIndex));
    }
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

intOpt XYPadWithPresetsWidget::getIndexOptAndHandleInvalid(QSpinBox *indexSpinbox)
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
}

void XYPadWithPresetsWidget::updateCurrentIndexesUIToMatchPresetData()
{
    const auto &curPreset = currentPresetSaveData();
    int xIndex = curPreset.xIndex.value_or(-1);
    int yIndex = curPreset.yIndex.value_or(-1);
    currentXIndex->setValue(xIndex);
    currentYIndex->setValue(yIndex);

    SV_LOG(std::format("Set indexes {} {}", xIndex, yIndex));
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


QJsonObjectWithWidgetOptionsOpt XYPadWithPresetsWidget::makeOptions()
{
    QJsonObjectWithWidgetOptions res;

    auto getPresetsJson = [&]() -> QJsonObjectOpt
    {
        QJsonObject obj;

        for (int presetIdx = 0; presetIdx < PresetsCount; ++presetIdx)
        {
            const auto &preset = presets[presetIdx];

            if (preset.hasValues() && *preset.xIndex >= 0 && *preset.yIndex >= 0)
            {
                obj[QString::number(presetIdx)] = preset.toJson();
            }
        }

        return !obj.empty() ? obj : QJsonObjectOpt{};
    };

    if (auto presetsObj = getPresetsJson())
    {
        res[presetsKey] = *presetsObj;
    }

    if (currentPresetIdx != 0)
    {
        res[currentPresetKey] = currentPresetIdx;
    }

    return !res.empty() ? res : QJsonObjectWithWidgetOptionsOpt{};
}

void XYPadWithPresetsWidget::restoreFromOptions(const QJsonObjectWithWidgetOptions &options)
{
    auto restorePresets = [&](const QJsonObject& presetsJson)
    {
        for (auto [keyPresetIndexString, presetDataJson] : presetsJson.asKeyValueRange())
        {
            bool keyIsInteger       = false;
            int  presetIndex        = keyPresetIndexString.toString().toInt(&keyIsInteger);
            bool presetIndexIsValid = keyIsInteger && presetIndex >= 0 && presetIndex < PresetsCount;

            if (!presetIndexIsValid)
            {
                continue;
            }

            if (auto presetData = PresetData::fromJson(presetDataJson))
            {
                presets[presetIndex] = *presetData;
            }
        }

        updateCurrentIndexesUIToMatchPresetData();

        for (int i = 0; i < presetsButtons.size(); ++i)
        {
            updatePresetButtonIfNeeded(presetsButtons[i], i);
        }

        update(); //to repaint preset buttons mainly
    };

    if (doubleOpt currentIdx = getFromJson<double>(options, currentPresetKey))
    {
        auto idx = int(*currentIdx);
        if (idx >= 0 && idx < presetsButtons.size())
        {
            presetsButtons[idx]->setChecked(true);
        }
    }

    if (auto presetsObj = getFromJson<QJsonObject>(options, presetsKey))
    {
        restorePresets(*presetsObj);
    }
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

void XYPadWithPresetsWidget::setupSliderWidgetForIndex(LimitedDoubleWidget* sliderParam, intOpt indexOpt)
{
    auto isValidIdx = indexOpt && componentIndexIsValid(*indexOpt);

    sliderParam->setEnabled(isValidIdx);

    QSignalBlocker block(sliderParam);
    sliderParam->setValue(isValidIdx ? parent->getValue()[*indexOpt] : LimitedDouble{});
}

void XYPadWithPresetsWidget::updateEverythingToMatchParentValue()
{
    const auto &vectorValue = parent->getValue();

    intOpt xIndexOpt = getIndexOptAndHandleInvalid(currentXIndex);
    intOpt yIndexOpt = getIndexOptAndHandleInvalid(currentYIndex);

    currentPresetSaveData() = {xIndexOpt, yIndexOpt};

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
