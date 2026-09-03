#pragma once
#include "sv_qtcommon.h"

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

        btn->setStyleSheet(defaultPushButtonStyleWithoutMargins());

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

    static QPushButton* makeToggleControl

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

signals:
    //emitted when its changed on posWidget, regardless of activateButton
    void valueChanged(glm::vec3 val);

    //emitted when button clicked
    void activatePressed(glm::vec3 curVal);

private:
    QPushButton* activateButton = nullptr;
    SpinboxesVec3* posWidget = nullptr;
};

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

        radiusSpinbox = makeStandardSpinbox(this, 0.0001, 1000.0, 1.0);
        scaleSpinbox  = makeStandardSpinbox(this, 1.0, 100.0, 4.0);

        layout->addWidget(enabledBtn);
        layout->addWidget(radiusLabel);
        layout->addWidget(radiusSpinbox);
        layout->addWidget(scaleLabel);
        layout->addWidget(scaleSpinbox);

        auto emitChanged = [this]()
        {
            emit settingsChanged(enabledBtn->isChecked(),
                                 radiusSpinbox->value(),
                                 scaleSpinbox->value());
        };

        connect(radiusSpinbox,  &QDoubleSpinBox::valueChanged,  this, emitChanged);
        connect(scaleSpinbox,   &QDoubleSpinBox::valueChanged,  this, emitChanged);
        connect(enabledBtn,     &QPushButton::toggled,          this, emitChanged);
    }

signals:
    void settingsChanged(bool enabled, double radius, double scale);

private:
    QPushButton*    enabledBtn      = nullptr;
    QLabel*         radiusLabel     = nullptr;
    QDoubleSpinBox* radiusSpinbox   = nullptr;
    QLabel*         scaleLabel      = nullptr;
    QDoubleSpinBox* scaleSpinbox    = nullptr;
};