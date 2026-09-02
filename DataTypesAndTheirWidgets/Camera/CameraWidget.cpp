#include "CameraWidget.h"

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

const int ControlHeight = 20;
const int LeftmostControlWidth = 50;
const int RightControlWidth = 150;
const int ControlLayoutSpacing = 8;

QPushButton* CameraWidget::makeStdLeftButton(const QString& buttonText, QWidget* parent)
{
    QPushButton* btn = new QPushButton(buttonText, parent);
    btn->setFixedSize(LeftmostControlWidth, ControlHeight);
    return btn;
}

CameraWidget::AngleControl CameraWidget::makeAngleControl(const QString& angleName)
{
    QWidget*        control     = new QWidget(this);
    QHBoxLayout*        layout  = new QHBoxLayout(control);
    initLayoutSpacing(layout, 0, ControlLayoutSpacing);

    QPushButton* resetBtn = makeStdLeftButton(angleName, control);

    QSlider* slider = new QSlider(Qt::Horizontal, control);
    slider->setFixedSize(RightControlWidth, ControlHeight);
    slider->setMinimum(-1000);
    slider->setMaximum(+1000);
    slider->setValue(0);

    QDoubleSpinBox* spinBox = new QDoubleSpinBox(control);

    //master changes the other silently
    connect(slider, &QSlider::valueChanged, slider, [slider, spinBox](int)
    {
        QSignalBlocker block(spinBox);
        float degrees = glm::degrees(ang11ToRad(getSliderValue11(slider)));
        spinBox->setValue(degrees);
    });

    //slave changes the other loudly
    connect(spinBox, &QDoubleSpinBox::valueChanged, slider, [slider](double angDeg)
    {
        setSliderValue11(slider, radTo11(glm::radians(angDeg)));
    });

    connect(resetBtn, &QPushButton::clicked, slider, [slider]()
    {
        setSliderValue11(slider, 0.0);
    });


    layout->addWidget(resetBtn);
    layout->addWidget(slider);
    layout->addWidget(spinBox);

    return { control, resetBtn, slider, spinBox };
}

void CameraWidget::initControls()
{
    pitchControl = makeAngleControl("Pitch");

    connect(pitchControl.angleSlider, &QSlider::valueChanged, this, [this](int)
    {
        float roll = pitchControl.getRadians();

        cameraViewport->changeCamera([roll](auto& camera)
        {
            camera.setRoll(roll);
        });
    });

    controlsLayout->addWidget(pitchControl.control);
}

void CameraWidget::updateUiFromCamera()
{
    const BasicPlaneCamera& camera = cameraViewport->getCamera();

    pitchControl.setRadiansSilently( camera.getRoll() );
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