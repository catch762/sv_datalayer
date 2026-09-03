#pragma once
#include "CameraViewport.h"



// 1 or 0.
// This adds second viewport, which renders same as first one, but with second camera,
// and you also can see first camera in that view. Helps debugging camera rotation code.
#define CAMERAWIDGET_ENABLE_DEBUGVIEWPORT 1

class ValueControl;
class PositionControl;
class ScaleConstraintControl;

class CameraWidget : public QWidget
{
public:
    CameraWidget(QWidget* parent = nullptr);

private:
    void initControls();

    void updateUiFromCamera();

    static void renderScene(CameraViewport& vp, QPainter& p, const CameraViewport* additionalCameraToRender = nullptr);


private:
    QGridLayout*    layout                  = nullptr;
    CameraViewport*     cameraViewport      = nullptr;
#if CAMERAWIDGET_ENABLE_DEBUGVIEWPORT
    CameraViewport*     cameraViewportDbg  = nullptr;
#endif

    

    QHBoxLayout*        controlsShowHideButtonsLayout   = nullptr;
    QVBoxLayout*        controlsLayout                  = nullptr;
    PositionControl*        posControl                  = nullptr; 
    PositionControl*        lookatControl               = nullptr; 
    ValueControl*           yfovControl                 = nullptr;
    ValueControl*           pitchControl                = nullptr;
    ValueControl*           yawControl                  = nullptr;
    ValueControl*           rollControl                 = nullptr;
    ValueControl*           speedControl                = nullptr;
    ValueControl*           msensControl                = nullptr;
    ScaleConstraintControl* scaleConstraintControl      = nullptr;

};