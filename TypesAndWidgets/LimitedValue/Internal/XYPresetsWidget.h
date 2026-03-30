#pragma once
#include "sv_qtcommon.h"
#include <QScrollArea>

class XYPresetsWidget : public QWidget
{
public:
    XYPresetsWidget(QWidget* parent = nullptr);

private:

private:
    QHBoxLayout*                layout                      = nullptr;
    HorizontalScrollAreaWidget*     presetsScrollAreaWidget = nullptr;
    QPushButton*                    addPresetButton         = nullptr;
};