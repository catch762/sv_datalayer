#pragma once
#include "CameraViewport.h"



// 1 or 0.
// This adds second viewport, which renders same as first one, but with second camera,
// and you also can see first camera in that view. Helps debugging camera rotation code.
#define CAMERAWIDGET_ENABLE_DEBUGVIEWPORT 1

class CameraWidget : public QWidget
{
    Q_OBJECT
public:
    CameraWidget(QWidget* parent = nullptr);

private:
    void initControls();

    struct AngleControl
    {
        QWidget*        control                 = nullptr;
        QPushButton*        angleResetButton    = nullptr;
        QSlider*            angleSlider         = nullptr; //master representation
        QDoubleSpinBox*     angleSpinbox        = nullptr; //slave

        float getRadians() const
        {
            return ang11ToRad(getSliderValue11(angleSlider));
        }
        void setRadiansSilently(float radians)
        {
            QSignalBlocker block1(angleSlider);
            QSignalBlocker block2(angleSpinbox);

            setSliderValue11(angleSlider, radTo11(radians));
            angleSpinbox->setValue(glm::degrees(radians));
        }
    };
    QPushButton* makeStdLeftButton(const QString& buttonText, QWidget* parent = nullptr);
    AngleControl makeAngleControl(const QString& angleName);

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
    AngleControl            pitchControl;

};