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
    static constexpr float maxMoveSpeed = 0.075;
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
        auto [newPhase01u, constrainedCamPos] = constrainPos(camera.getPos(), getPhase(), getRadius(), getScale());

        phase01u = newPhase01u;
        camera.setPos(constrainedCamPos);
    }

    static std::pair<double /*newphase01u*/, glm::vec3 /*scaledpoint*/> constrainPos(   const glm::vec3 pos,
                                                                                        const double    oldphase01u,
                                                                                        const double    baseRadius,
                                                                                        const double    scale)
    {
        SV_ASSERT(scale >= 1.0);

        //********************************************************************
        // 
        // The following defines three zones:
        // 
        //  [zone +1]: [radiusPrev, radiusMin]
        //  [zone  0]: [radiusMin,  radiusMax]
        //  [zone -1]: [radiusMax,  radiusNext]
        //
        // We have to:
        //      - calculate new phase01u (which is zone idx integer + ratio to next biggest zone from 0 to 1)
        //      - return camera to zone 0.
        // 
        //********************************************************************
        const double radiusPrev = baseRadius / scale;
        const double radiusMin  = baseRadius;
        const double radiusMax  = baseRadius * scale;
        const double radiusNext = baseRadius * scale * scale;

        //yes, we only calculate "horizontal plane radius" for now
        const double radiusCur  = glm::length(glm::vec2(pos.x, pos.z));

        //Lets limit it by "zone -1" to "zone +1"
        const double radiusCurClamped = std::max(std::min(radiusCur, radiusNext), radiusPrev);

        if (radiusCurClamped >= radiusMin && radiusCurClamped <= radiusMax)
        {
            //All in range.

            const double newPhase01u = std::floor(oldphase01u) + getValue01Clamped(radiusCurClamped, radiusMax, radiusMin);

            return { newPhase01u, pos };
        }

        //For smoother movement, we scale the distance travelled past the border?
        //It kinda makes sense but im not sure tbh:

        if (radiusCurClamped < radiusMin)
        {
            const double scaledDiff         = (radiusCurClamped - radiusMin) / scale;
            const double radiusCurCorrected = radiusMin + scaledDiff;

            const double phaseInZoneMinusOne = getValue01Clamped(radiusCurCorrected, radiusMin, radiusPrev);

            const double newPhase01u = std::floor(oldphase01u) + 1.0 + phaseInZoneMinusOne;

            return { newPhase01u, pos * float(scale) };
        }
        else // radiusCurClamped > radiusMax
        {
            const double scaledDiff         = (radiusCurClamped - radiusMax) * scale;
            const double radiusCurCorrected = radiusMax + scaledDiff;

            const double phaseInZonePlusOne = getValue01Clamped(radiusCurCorrected, radiusNext, radiusMax);

            const double newPhase01u = std::floor(oldphase01u) - 1.0 + phaseInZonePlusOne;

            return { newPhase01u, pos / float(scale) };
        }
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
    double           phase01u = 0;
};