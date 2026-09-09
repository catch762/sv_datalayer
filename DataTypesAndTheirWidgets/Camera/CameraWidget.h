#pragma once
#include "CameraViewport.h"
#include "CameraData.h"


// 1 or 0.
// This adds second viewport, which renders same as first one, but with second camera,
// and you also can see first camera in that view. Helps debugging camera rotation code.
#define CAMERAWIDGET_ENABLE_DEBUGVIEWPORT 0

class ValueControl;
class PositionControl;
class ScaleConstraintControl;

class CameraWidget : public QWidget
{
    Q_OBJECT
public:
    CameraWidget(const CameraDataOpt& camOpt = {}, QWidget* parent = nullptr);

    CameraData getValue() const;
    void setValue(const CameraData& cam);

signals:
    //first camera viewport emits cameraChanged signal, then we apply some filtering logic
    //(for example, constraints) and then we emit this final signal.
    void filteredCameraChanged(const BasicFPSCamera& camera);

    void valueChanged(const CameraData& cam);

private:
    void onViewportCameraChanged(const BasicFPSCamera& camera);

    void initControls();
    void initControlsShowHideButtons();

    void updateUiFromCamera();

    static void renderScene(CameraViewport& vp, QPainter& p, const CameraViewport* additionalCameraToRender = nullptr);

    

private:
    QVBoxLayout*    layout                  = nullptr;
    CameraViewport*     cameraViewport      = nullptr;
#if CAMERAWIDGET_ENABLE_DEBUGVIEWPORT
    CameraViewport*     cameraViewportDbg  = nullptr;
#endif

    

    QHBoxLayout*        controlsShowHideButtonsLayout   = nullptr;
    QToolButton*            posControlShow              = nullptr;
    QToolButton*            lookatControlShow           = nullptr;
    QToolButton*            yfovControlShow             = nullptr;
    QToolButton*            pitchControlShow            = nullptr;
    QToolButton*            yawControlShow              = nullptr;
    QToolButton*            rollControlShow             = nullptr;
    QToolButton*            speedControlShow            = nullptr;
    QToolButton*            msensControlShow            = nullptr;
    QToolButton*            scaleConstraintControlShow  = nullptr;
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