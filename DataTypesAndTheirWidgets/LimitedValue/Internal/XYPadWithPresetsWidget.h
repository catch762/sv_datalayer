#pragma once
#include "sv_qtcommon.h"
#include "../LimitedValue.h"
#include <QGridLayout>
#include "BaseXYPadWidget.h"
#include "WidgetLogic/WidgetDefs.h"

class LimitedValueWidget;
class LimitedDoubleVecWidget;
class XYPadWithPresetsWidget;

// The actual XY Pad space.
//
// This small class only exists to redefine painting, taking 2d points
// from 'XYPadWithPresetsWidget' and drawing them on top.
class XYPadForPresets : public BaseXYPadWidget
{
    Q_OBJECT
public:
    XYPadForPresets(XYPadWithPresetsWidget* theParent);
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    XYPadWithPresetsWidget* parent = nullptr;
};


class XYPadWithPresetsWidget : public QWidget
{
    Q_OBJECT
public:
    XYPadWithPresetsWidget(LimitedDoubleVecWidget* parent = nullptr);

    struct PresetData
    {
        QJsonArray toJson() const;
        static std::optional<PresetData> fromJson(const QJsonValue& json);
        bool hasValues() const;

        intOpt xIndex;
        intOpt yIndex;
    };

    void iterateValidPresetPoints(std::function<void(const LimitedDoublePair& xy,
                                                     int presetIndex,
                                                     bool presetIsSelected)> visitor);
    struct ColorData
    {
        QColor selectedColor;
        QColor notSelectedColor;

        bool isValid() { return selectedColor.isValid() && notSelectedColor.isValid(); }
    };
    static ColorData colorsForPreset(int presetIdx, bool isValid);

    QJsonObjectWithWidgetOptionsOpt makeOptions();
    void restoreFromOptions(const QJsonObjectWithWidgetOptions& options);

public slots:
    void updateEverythingToMatchParentValue(); //will not emit any signals
                                                     
private slots:
    void onPresetSelected(int presetIdx);
    void onXYRepresentationChanged(const LimitedDoublePair& point);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    
private:
    PresetData& currentPresetSaveData();
    bool componentIndexIsValid(int componentIndex);
    //Returns whether both indexes in PresetData are valid indexes for 'parent->getValue()' vector
    bool presetIsValid(const PresetData& data);
    //if !presetIsValid(), returns {}
    LimitedDoublePairOpt tryGetPointFromPreset(const PresetData& data);

    void setPresetButtonStylesheetAndColors(QPushButton* btn, ColorData colors);
    void updatePresetButtonIfNeeded(QPushButton* btn, int index);

    //pass paramX or paramY
    void setupSliderWidgetForIndex(LimitedValueWidget* sliderParam, intOpt indexOpt);
    //pass currentXIndex or currentYIndex. Any invalid value on spinbox will change to -1
    intOpt getIndexOptAndHandleInvalid(QSpinBox* indexSpinbox); 
    void updateCurrentIndexesUIToMatchPresetData();

private:
    QGridLayout*            layout                      = nullptr;
    //Current XY: Slider Representation. Sync'ed to the other.
    LimitedValueWidget*        paramX                  = nullptr;
    LimitedValueWidget*        paramY                  = nullptr;
    //Current XY: XY pad Representation. Sync'ed to the other.
    BaseXYPadWidget*            xyPad                   = nullptr;
    QWidget*                    presetsWidget           = nullptr;
    QVBoxLayout*                    presetsLayout       = nullptr;
    std::vector<QPushButton*>       presetsButtons;

    QSpinBox*                           currentXIndex   = nullptr;
    QSpinBox*                           currentYIndex   = nullptr;

private:
    LimitedDoubleVecWidget* parent = nullptr;

    static inline const int PresetsCount = 9;
    std::array<PresetData, PresetsCount> presets;
    int currentPresetIdx = 0; //index in 'presets' array
};