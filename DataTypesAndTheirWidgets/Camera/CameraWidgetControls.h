#pragma once
#include "sv_qtcommon.h"
#include <QToolButton>

class CWControls
{
public:
    static constexpr int ControlHeight = 20;
    static constexpr int LeftmostControlWidth = 80;
    static constexpr int RightControlWidth = 150;
    static constexpr int ControlLayoutSpacing = 8;

public:
    static QHBoxLayout* makeStdLayout(QWidget* control)
    {
        QHBoxLayout* layout = new QHBoxLayout(control);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(ControlLayoutSpacing);
        return layout;
    }

    static QPushButton* makeStdLeftButton(const QString& buttonText, QWidget* parent)
    {
        QPushButton* btn = new QPushButton(buttonText, parent);

        btn->setStyleSheet(myDefaultPushButtonStyle());

        btn->setFixedSize(LeftmostControlWidth, ControlHeight);
        return btn;
    }

    static QPushButton* makeStdLeftToggleButton(const QString& buttonText, QWidget* parent)
    {
        QPushButton* btn = makeStdLeftButton(buttonText, parent);

        btn->setCheckable(true);
        btn->setChecked(false);

        return btn;
    }

    static QToolButton* makeEnableControlButton(const QString& text, QWidget* controlWidget, QWidget* parent)
    {
        auto* button = new QToolButton(parent);
        button->setText(text);
        button->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextOnly);
        button->setArrowType(Qt::ArrowType::NoArrow);
        button->setCheckable(true);
        
        QObject::connect(button, &QToolButton::toggled, controlWidget, &QWidget::setVisible);

        return button;
    }

public:
    static constexpr float maxMoveSpeed = 0.2;
    static float moveSpeedFromSpeed01(float speed01)
    {
        speed01 = std::clamp(speed01, 0.0f, 1.0f);
        return (speed01 * speed01) * maxMoveSpeed;
    }
    static float speed01FromMoveSpeed(float moveSpeed)
    {
        moveSpeed = std::clamp(moveSpeed, 0.0f, maxMoveSpeed);
        return sqrtf(moveSpeed / maxMoveSpeed);
    }
};

// Control for single 'double'.
// So far 'activateButton' on the left just resets
// value to 'defaultValue', i think its all i need
class ValueControl : public QWidget
{
    Q_OBJECT
public:
    ValueControl(const QString& buttonName, double min, double max, double defaultValue, QWidget* parent = nullptr) : QWidget(parent)
    {
        setFixedHeight(CWControls::ControlHeight);

        QHBoxLayout* layout = CWControls::makeStdLayout(this);

        activateButton = CWControls::makeStdLeftButton(buttonName, this);
        valueWidget = new SliderAndSpinbox(this, min, max, defaultValue);

        connect(valueWidget, &SliderAndSpinbox::valueChanged, this, &ValueControl::valueChanged);

        connect(activateButton, &QPushButton::clicked, this, [this, defaultValue]()
        {
            setValue(defaultValue);
        });

        layout->addWidget(activateButton);
        layout->addWidget(valueWidget);
    }

    float getValue() const
    {
        return valueWidget->getValue();
    }
    void setValue(float value)
    {
        valueWidget->setValue(value);
    }
signals:
    void valueChanged(double value);

private:
    QPushButton*        activateButton  = nullptr;
    SliderAndSpinbox*   valueWidget     = nullptr;
};

class PositionControl : public QWidget
{
    Q_OBJECT
public:
    PositionControl(const QString& buttonText, QWidget* parent = nullptr) : QWidget(parent)
    {
        setFixedHeight(CWControls::ControlHeight);

        QHBoxLayout* layout = CWControls::makeStdLayout(this);

        activateButton = CWControls::makeStdLeftButton(buttonText, this);
        posWidget = new SpinboxesVec3({}, this);

        //SV_LOG(widgetInfo(posWidget));

        connect(posWidget, &SpinboxesVec3::valueChanged, this, &PositionControl::valueChanged);

        connect(activateButton, &QPushButton::clicked, this, [this]()
        {
            emit activatePressed(getValue());
        });

        layout->addWidget(activateButton);
        layout->addWidget(posWidget);
    }

    glm::vec3 getValue() const
    {
        return posWidget->getValue();
    }
    void setValue(glm::vec3 val)
    {
        posWidget->setValue(val);
    }

    void log()
    {
        SV_LOG(widgetInfo(this));
        SV_LOG(widgetInfo(activateButton));
        SV_LOG(widgetInfo(posWidget));
    }

signals:
    //emitted when its changed on posWidget, regardless of activateButton
    void valueChanged(glm::vec3 val);

    //emitted when button clicked
    void activatePressed(glm::vec3 curVal);

private:
    QPushButton* activateButton = nullptr;
    SpinboxesVec3* posWidget = nullptr;
};

//This control is highly experimental adhoc-for-nest-project shit.
class ScaleConstraintControl : public QWidget
{
    Q_OBJECT
public:
    ScaleConstraintControl(QWidget* parent)
    {
        setFixedHeight(CWControls::ControlHeight);

        QHBoxLayout* layout = CWControls::makeStdLayout(this);

        enabledBtn = CWControls::makeStdLeftToggleButton("Scale limit", this);

        radiusLabel = new QLabel("Radius", this);
        radiusLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        scaleLabel  = new QLabel("Scale", this);
        scaleLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        radiusSpinbox = makeStandardSpinbox(this, 0.0001, 1000.0, 0.1666666, 7);
        radiusSpinbox->setFixedWidth(65);

        scaleSpinbox  = makeStandardSpinbox(this, 1.0, 100.0, 3.0);

        resetPhaseBtn = CWControls::makeStdLeftButton("Reset Ø", this);
        resetPhaseBtn->setFixedWidth(55);

        layout->addWidget(enabledBtn);
        layout->addWidget(radiusLabel);
        layout->addWidget(radiusSpinbox);
        layout->addWidget(scaleLabel);
        layout->addWidget(scaleSpinbox);
        layout->addWidget(resetPhaseBtn);

        auto emitChanged = [this]()
        {
            emit settingsChanged(enabledBtn->isChecked(),
                                 radiusSpinbox->value(),
                                 scaleSpinbox->value(),
                                 getPhase());
        };

        connect(radiusSpinbox,  &QDoubleSpinBox::valueChanged,  this, emitChanged);
        connect(scaleSpinbox,   &QDoubleSpinBox::valueChanged,  this, emitChanged);
        connect(enabledBtn,     &QPushButton::toggled,          this, emitChanged);
        connect(resetPhaseBtn,  &QPushButton::clicked,          this, [this, emitChanged]()
        {
            phase01u = 0;
            emitChanged();
        });
    }

    void constrainCameraAndUpdatePhase(BasicFPSCamera& camera)
    {
        auto [newPhase01u, constrainedCamPos] = CameraScaleConstraint::constrainPos(camera.getPos(), getPhase(), getRadius(), getScale());

        phase01u = newPhase01u;
        camera.setPos(constrainedCamPos);
    }

    double getPhase() const
    {
        return phase01u;
    }

    double getRadius() const
    {
        return radiusSpinbox->value();
    }
    double getScale() const
    {
        return scaleSpinbox->value();
    }

    bool scalingEnabled() const
    {
        return enabledBtn->isChecked();
    }
    
    glm::vec4 getModePhase() const
    {
        return glm::vec4{
            scalingEnabled() ? 1.0f : 0.0f,
            float(phase01u),
            radiusSpinbox->value(),
            scaleSpinbox->value()
        };
    }

    void setModePhase(glm::vec4 ModePhase)
    {
        enabledBtn->setChecked(ModePhase.x > 0.5);
        phase01u = ModePhase.y;
        radiusSpinbox->setValue(ModePhase.z);
        scaleSpinbox->setValue(ModePhase.w);
    }

signals:
    void settingsChanged(bool enabled, double radius, double scale, double phase01u);

private:
    QPushButton*    enabledBtn      = nullptr;
    QLabel*         radiusLabel     = nullptr;
    QDoubleSpinBox* radiusSpinbox   = nullptr;
    QLabel*         scaleLabel      = nullptr;
    QDoubleSpinBox* scaleSpinbox    = nullptr;
    QPushButton*    resetPhaseBtn   = nullptr;
    double          phase01u = 0;
};