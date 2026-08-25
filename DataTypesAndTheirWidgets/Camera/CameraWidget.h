#pragma once
#include "sv_qtcommon.h"

struct CameraData
{
	glm::vec3 pos		= {};
	float	  roll		= 0;
	glm::vec3 lookat	= glm::vec3(0, -1, 0);
	float	  fov		= 45;
};

SV_REGTYPENAME(CameraData);

class CameraViewport : public QWidget
{
public:
	CameraViewport(QWidget* parent = nullptr) : QWidget(parent)
	{
		setFixedSize(300, 300);
	}

    void keyPressEvent(QKeyEvent* event) override
    {
        QWidget::keyPressEvent(event);
    }

    void mousePressEvent(QMouseEvent* event) override
    {
        //if (event->button() == Qt::LeftButton)

        QWidget::mousePressEvent(event);
    }
    void mouseMoveEvent(QMouseEvent* event) override
    {
        QWidget::mouseMoveEvent(event);
    }

    void mouseReleaseEvent(QMouseEvent* event) override
    {
        QWidget::mouseMoveEvent(event);
    }

private:
    CameraData camera;
};