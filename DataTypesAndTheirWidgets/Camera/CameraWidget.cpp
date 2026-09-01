#include "CameraWidget.h"

CameraWidget::CameraWidget(QWidget* parent) : QWidget(parent)
{
    layout = new QGridLayout(this);

    //viewport on the left
    cameraViewport = new CameraViewport(this);
    cameraViewport->setRenderFunc(std::bind(&CameraWidget::renderScene, std::placeholders::_1, std::placeholders::_2, nullptr));
    layout->addWidget(cameraViewport, 0, 0);

    //optional debug viewport in the middle
#if CAMERAWIDGET_ENABLE_DEBUGVIEWPORT
    cameraViewportDbg = new CameraViewport(this);
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
    controlsLayout = new QGridLayout();
    layout->addLayout(controlsLayout, 0, controlsLayoutColumn);

    //the controls:
    {

    }

    setMinimumSize(400, 200);
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