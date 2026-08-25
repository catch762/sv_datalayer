#pragma once
#include "sv_qtcommon.h"
#include "../../LimitedValue.h"
#include <QGridLayout>
#include "Internal/BaseXYPadWidget.h"
#include "WidgetLogic/WidgetDefs.h"
#include "../ILimitedValueVecAtomicWidget.h"

class LimitedValueWidget;
class LimitedValueVecMultiWidget;
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


class XYPadWithPresetsWidget : public ILimitedValueVecAtomicWidget
{
    Q_OBJECT

private:
    static inline const int PresetsCount = 9;

public:
    XYPadWithPresetsWidget(LimitedValueVecMultiWidget* parent = nullptr);

    //************************************************
    // [ ILimitedValueVecAtomicWidget implementation ]
    //
    // Note that this specific widget isnt exactly
    // what ILimitedValueVecAtomicWidget implies,
    // because it doesnt hold its own value, it
    // pulls it directly from LimitedValueVecMultiWidget.
    // 
    // So getValue/setValue functions being overriden kind 
    // of do more or less what you expect, but not exactly.
    //
    //************************************************

    // This widget doesnt hold a value, so this will pull it from parent.
    // If you call this upon receiving valueChanged(actualNewValue) signal (before parent is updated),
    // you ll still get old value
    const LimitedIntOrDoubleVec& getValue() const override;
    // This widget doesnt hold a value, but this will update everything visually
    void setValue(const LimitedIntOrDoubleVec& newValue) override;

    WidgetOptionsJsonOpt makeOptions() const override;
    void applyOptions(const WidgetOptionsJson& options) override;

    //************************************************
    // [/ILimitedValueVecAtomicWidget implementation ]
    //************************************************

    struct PresetData
    {
        QJsonArray toJson() const;
        static std::optional<PresetData> fromJson(const QJsonValue& json);
        bool hasValues() const;

        intOpt xIndex;
        intOpt yIndex;
    };

    void iterateValidPresetPoints(std::function<void(const LimitedIntOrDoublePair& xy,
                                                     int presetIndex,
                                                     bool presetIsSelected)> visitor);
    struct ColorData
    {
        QColor selectedColor;
        QColor notSelectedColor;

        bool isValid() { return selectedColor.isValid() && notSelectedColor.isValid(); }
    };
    static ColorData colorsForPreset(int presetIdx, bool isValid);

    


    const std::array<PresetData, PresetsCount>& getPresets() const;
    int getCurrentPresetIdx() const;

public slots:
    void updateEverythingToMatchParentValue(); //will not emit any signals
                                                     
private slots:
    void onPresetSelected(int presetIdx);
    void onAnyRepresentationChanged(const LimitedIntOrDoublePair& point);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    
private:
    PresetData& currentPresetSaveData();
    bool componentIndexIsValid(int componentIndex);
    //Returns whether both indexes in PresetData are valid indexes for 'parent->getValue()' vector
    bool presetIsValid(const PresetData& data);
    //if !presetIsValid(), returns {}
    LimitedIntOrDoublePairOpt tryGetPointFromPreset (const PresetData& data);
    LimitedIntOrDoublePairOpt tryGetPointFromSliders();

    //silently means no signals will be emitted:
    void silentlySetPointOnSliderRepresentation(const LimitedIntOrDoublePair& point);

    void setPresetButtonStylesheetAndColors (QPushButton* btn, ColorData colors);
    void updatePresetButtonIfNeeded         (QPushButton* btn, int index);

    //pass paramX or paramY
    void    setupSliderWidgetForIndex               (LimitedValueWidget* sliderParam, intOpt indexOpt);
    //pass currentXIndex or currentYIndex. Any invalid value on spinbox will change to -1
    intOpt  getIndexOptAndHandleInvalid             (QSpinBox* indexSpinbox); 
    void    updateCurrentIndexesUIToMatchPresetData ();

private:

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
    LimitedValueVecMultiWidget* parent = nullptr;

    std::array<PresetData, PresetsCount> presets;
    int currentPresetIdx = 0; //index in 'presets' array
};