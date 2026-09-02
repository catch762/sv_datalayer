#include "CameraWidget.h"
#include "CameraWidgetControls.h"


CameraWidget::CameraWidget(QWidget* parent) : QWidget(parent)
{
    layout = new QGridLayout(this);

    //viewport on the left
    cameraViewport = new CameraViewport(this);
    cameraViewport->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    cameraViewport->setRenderFunc(std::bind(&CameraWidget::renderScene, std::placeholders::_1, std::placeholders::_2, nullptr));
    connect(cameraViewport, &CameraViewport::cameraChanged, this, &CameraWidget::updateUiFromCamera);
    layout->addWidget(cameraViewport, 0, 0);

    //optional debug viewport in the middle
#if CAMERAWIDGET_ENABLE_DEBUGVIEWPORT
    cameraViewportDbg = new CameraViewport(this);
    cameraViewportDbg->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    cameraViewportDbg->setRenderFunc(std::bind(&CameraWidget::renderScene, std::placeholders::_1, std::placeholders::_2, cameraViewport));

    connect(cameraViewport, &CameraViewport::cameraChanged, this,
        [this]()
        {
            cameraViewportDbg->update();
        });

    layout->addWidget(cameraViewportDbg, 0, 1);
#endif

    int controlsLayoutColumn = 1 + int(CAMERAWIDGET_ENABLE_DEBUGVIEWPORT);

    //layout for all controls on the right
    controlsLayout = new QVBoxLayout();
    layout->addLayout(controlsLayout, 0, controlsLayoutColumn);

    initControls();

    resize(1300, 400);
}



void CameraWidget::initControls()
{
    pitchControl = new AngleControl("Pitch", this);
    {
        connect(pitchControl, &AngleControl::angleChanged, this, [this](double angleRadians)
            {
                cameraViewport->changeCamera([angleRadians](auto& camera)
                    {
                        camera.setRoll(angleRadians);
                    });
            });

        controlsLayout->addWidget(pitchControl);
    }

    posControl = new PositionControl("Pos", this);
    {
        controlsLayout->addWidget(posControl);
    }
    lookatControl = new PositionControl("LookAt", this);
    {
        controlsLayout->addWidget(lookatControl);
    }
}

void CameraWidget::updateUiFromCamera()
{
    const BasicPlaneCamera& camera = cameraViewport->getCamera();

    QSignalBlocker block(pitchControl);
    pitchControl->setRadians( camera.getRoll() );
}

void CameraWidget::renderScene(CameraViewport& vp, QPainter& p, const CameraViewport* additionalCameraToRender)
{
    vp.fillWithColor(p, additionalCameraToRender ? QColor(0, 0, 0) : QColor(55, 55, 55));

    vp.drawStandardGrid(p);

    vp.drawWorldAxes(p);

    if (additionalCameraToRender)
    {
        vp.drawCameraAxes(p, additionalCameraToRender->getCamera());
    }

    int textY = 0;
    textY = vp.printText(p, QString::fromStdString(vp.getCamera().toString()), Qt::white, textY);
}