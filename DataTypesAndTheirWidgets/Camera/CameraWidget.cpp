#include "CameraWidget.h"
#include "CameraWidgetControls.h"


CameraWidget::CameraWidget(QWidget* parent) : QWidget(parent)
{
    layout = new QVBoxLayout(this);
    initLayoutSpacing(layout, 2, 2);

    //viewport on the left
    cameraViewport = new CameraViewport(this);
    cameraViewport->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    cameraViewport->setRenderFunc(std::bind(&CameraWidget::renderScene, std::placeholders::_1, std::placeholders::_2, nullptr));
    connect(cameraViewport, &CameraViewport::cameraChanged, this, &CameraWidget::updateUiFromCamera);
    layout->addWidget(cameraViewport, 1);

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

    layout->addWidget(cameraViewportDbg, 1);
#endif

    //layout for all controls
    controlsLayout = new QVBoxLayout();
    initLayoutSpacing(controlsLayout, 0, 2);
    {
        //layout->addLayout(controlsLayout);

        initControls();
        updateUiFromCamera();
    }


    controlsShowHideButtonsLayout = new QHBoxLayout();
    initLayoutSpacing(controlsShowHideButtonsLayout, 0, 2);
    {
        initControlsShowHideButtons();
    }

    layout->addLayout(controlsShowHideButtonsLayout);
    layout->addLayout(controlsLayout);

    //Initial controls visibility:
    posControlShow->toggle();
    lookatControlShow->toggle();
    yfovControlShow->toggle();
    pitchControlShow->toggle();
    yawControlShow->toggle();
    rollControlShow->toggle();
}



void CameraWidget::initControls()
{
    posControl = new PositionControl("Pos", this);
    {
        connect(posControl, &PositionControl::valueChanged, this, [this](glm::vec3 pos)
        {
            cameraViewport->changeCamera([pos](auto& camera)
            {
                camera.setPos(pos);
            });
        });

        connect(posControl, &PositionControl::activatePressed, this, [this](glm::vec3)
        {
            cameraViewport->changeCamera([](auto& camera)
            {
                camera.setPos({1,1,1});
                camera.lookAtWithoutRoll({ 0,0,0 });
            });
        });

        controlsLayout->addWidget(posControl);
    }

    lookatControl = new PositionControl("Look at", this);
    {
        connect(lookatControl, &PositionControl::activatePressed, this, [this](glm::vec3 pos)
        {
            cameraViewport->changeCamera([pos](auto& camera)
            {
                camera.lookAtWithoutRoll(pos);
            });
        });

        controlsLayout->addWidget(lookatControl);
    }

    yfovControl = new ValueControl("Y FOV", 0.1, 170.0, 45.0, this);
    {
        connect(yfovControl, &ValueControl::valueChanged, this, [this](double degrees)
        {
                posControl->log();
            cameraViewport->changeCamera([degrees](auto& camera)
            {
                camera.setYFov(glm::radians(degrees));
            });
        });

        controlsLayout->addWidget(yfovControl);
    }

    pitchControl = new ValueControl("Pitch", -180, 180, 0, this);
    {
        connect(pitchControl, &ValueControl::valueChanged, this, [this](double degrees)
        {
            cameraViewport->changeCamera([degrees](auto& camera)
            {
                camera.setPitch(glm::radians(degrees));
            });
        });

        controlsLayout->addWidget(pitchControl);
    }
    yawControl = new ValueControl("Yaw", -180, 180, 0, this);
    {
        connect(yawControl, &ValueControl::valueChanged, this, [this](double degrees)
        {
            cameraViewport->changeCamera([degrees](auto& camera)
            {
                camera.setYaw(glm::radians(degrees));
            });
        });

        controlsLayout->addWidget(yawControl);
    }
    rollControl = new ValueControl("Roll", -180, 180, 0, this);
    {
        connect(rollControl, &ValueControl::valueChanged, this, [this](double degrees)
        {
            cameraViewport->changeCamera([degrees](auto& camera)
            {
                camera.setRoll(glm::radians(degrees));
            });
        });

        controlsLayout->addWidget(rollControl);
    }


    speedControl = new ValueControl("Speed", 0, 1, 0.2, this);
    {
        connect(speedControl, &ValueControl::valueChanged, this, [this](double speed01)
        {
            cameraViewport->setMoveSpeed(CWControls::moveSpeedFromSpeed01(speed01));
        });

        controlsLayout->addWidget(speedControl);
    }

    msensControl = new ValueControl("Mouse sens", 0.1, 2, 1, this);
    {
        connect(msensControl, &ValueControl::valueChanged, this, [this](double sens)
        {
            cameraViewport->setMouseSens(sens);
        });

        controlsLayout->addWidget(msensControl);
    }

    scaleConstraintControl = new ScaleConstraintControl(this);
    {
        controlsLayout->addWidget(scaleConstraintControl);
    }

    //hide all controls by default
    iterateImmediateWidgets(controlsLayout, [](QWidget* w) { w->hide(); });

    controlsLayout->addStretch();
}

void CameraWidget::updateUiFromCamera()
{
    const BasicPlaneCamera& camera = cameraViewport->getCamera();

    {
        QSignalBlocker block(posControl);
        posControl->setValue(camera.getPos());
    }

    {
        QSignalBlocker block(yfovControl);
        pitchControl->setValue(glm::degrees(camera.getYFov()));
    }

    {
        QSignalBlocker block(pitchControl);
        pitchControl->setValue(glm::degrees(camera.getPitch()));
    }
    {
        QSignalBlocker block(yawControl);
        yawControl->setValue(glm::degrees(camera.getYaw()));
    }
    {
        QSignalBlocker block(rollControl);
        rollControl->setValue(glm::degrees(camera.getRoll()));
    }

    {
        QSignalBlocker block(speedControl);
        speedControl->setValue(CWControls::speed01FromMoveSpeed(cameraViewport->getMoveSpeed()));
    }

    {
        QSignalBlocker block(msensControl);
        msensControl->setValue(cameraViewport->getMouseSens());
    }
}

void CameraWidget::initControlsShowHideButtons()
{
    auto add = [this](QToolButton* w)
    {
        controlsShowHideButtonsLayout->addWidget(w);
        return w;
    };

    
    posControlShow              = add(CWControls::makeEnableControlButton("Pos",    posControl,             this));
    lookatControlShow           = add(CWControls::makeEnableControlButton("LookAt", lookatControl,          this));
    yfovControlShow             = add(CWControls::makeEnableControlButton("Fov",    yfovControl,            this));
    pitchControlShow            = add(CWControls::makeEnableControlButton("P°",     pitchControl,           this));
    yawControlShow              = add(CWControls::makeEnableControlButton("Y°",     yawControl,             this));
    rollControlShow             = add(CWControls::makeEnableControlButton("R°",     rollControl,            this));
    speedControlShow            = add(CWControls::makeEnableControlButton("Speed",  speedControl,           this));
    msensControlShow            = add(CWControls::makeEnableControlButton("Sens",   msensControl,           this));
    scaleConstraintControlShow  = add(CWControls::makeEnableControlButton("SLim",   scaleConstraintControl, this));

    controlsShowHideButtonsLayout->addStretch();
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