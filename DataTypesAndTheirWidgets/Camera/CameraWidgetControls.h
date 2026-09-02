#pragma once
#include "sv_qtcommon.h"

class CWControls
{
public:
    static constexpr int ControlHeight = 20;
    static constexpr int LeftmostControlWidth = 50;
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
        btn->setFixedSize(LeftmostControlWidth, ControlHeight);
        return btn;
    }
};

class AngleControl : public QWidget
{
    Q_OBJECT
public:
    AngleControl(const QString& angleName, QWidget* parent = nullptr) : QWidget(parent)
    {
        QHBoxLayout* layout = CWControls::makeStdLayout(this);

        angleResetButton = CWControls::makeStdLeftButton(angleName, this);
        angleWidget = new SliderAndSpinbox(this, -180.0, 180.0, 0.0);

        connect(angleWidget, &SliderAndSpinbox::valueChanged, this, [this](double degrees)
            {
                emit angleChanged(glm::radians(degrees));
            });

        connect(angleResetButton, &QPushButton::clicked, this, [this]()
            {
                angleWidget->setValue(0);
            });

        layout->addWidget(angleResetButton);
        layout->addWidget(angleWidget);
    }

    float getDegrees() const
    {
        return angleWidget->getValue();
    }
    void setDegrees(float degrees)
    {
        angleWidget->setValue(degrees);
    }
    float getRadians() const
    {
        return glm::radians(getDegrees());
    }
    void setRadians(float radians)
    {
        setDegrees(glm::degrees(radians));
    }

signals:
    void angleChanged(double angleRadians);

private:
    QPushButton* angleResetButton = nullptr;
    SliderAndSpinbox* angleWidget = nullptr;
};