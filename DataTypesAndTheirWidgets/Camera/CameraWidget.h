#pragma once
#include "sv_qtcommon.h"
#include <QTimer>

struct CameraData
{
	glm::vec3 pos		= {};
	float	  roll		= 0;
	glm::vec3 dir	    = glm::vec3(0, 0, -1);
	float	  fov		= 45;
};

SV_REGTYPENAME(CameraData);

class CameraViewport : public QWidget
{
    Q_OBJECT
public:
	CameraViewport(QWidget* parent = nullptr) : QWidget(parent)
	{
		setFixedSize(300, 300);

        camera.setPos({ 3, 3, 3 });
        camera.lookAt({ 0, 0, 0 });

        updateTimer.setSingleShot(false);
        updateTimer.setInterval(0);
        connect(&updateTimer, &QTimer::timeout, this, &CameraViewport::onUpdate);
	}

    void keyPressEvent(QKeyEvent* event) override
    {
        if (keysToTrack.contains(event->key()))
        {
            keysPressed.insert(event->key());

            setUpdatesEnabled(true);
        }
        else
        {
            QWidget::keyPressEvent(event);
        }
    }
    void keyReleaseEvent(QKeyEvent* event) override
    {
        if (keysPressed.contains(event->key()))
        {
            keysPressed.remove(event->key());

            setUpdatesEnabled(!keysPressed.isEmpty());
        }
        else
        {
            QWidget::keyReleaseEvent(event);
        }
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

    void paintEvent(QPaintEvent* event) override
    {
        QPainter p(this);

        p.fillRect(rect(), QColor(200, 200, 200));

        auto setColor = [&](QColor color)
        {
            QPen linePen = QPen(color, 3);
            p.setPen(linePen);
        };

        setColor(Qt::red);
        drawWorldLine(p, {}, { 1,0,0 });
        setColor(Qt::green);
        drawWorldLine(p, {}, { 0,1,0 });
        setColor(Qt::blue);
        drawWorldLine(p, {}, { 0,0,1 });
    }

private:
    void setUpdatesEnabled(bool enabled)
    {
        if      ( enabled && !updateTimer.isActive()) updateTimer.start();
        else if (!enabled &&  updateTimer.isActive()) updateTimer.stop();
    }

    void applyMovementTick()
    {
        float moveForward = 0;
        float moveUpward = 0;
        float moveRight = 0;

        if (keysPressed.contains(Qt::Key_W))
        {
            moveForward += 1;
        }
        if (keysPressed.contains(Qt::Key_S))
        {
            moveForward -= 1;
        }
        if (keysPressed.contains(Qt::Key_D))
        {
            moveRight += 1;
        }
        if (keysPressed.contains(Qt::Key_A))
        {
            moveRight -= 1;
        }
        if (keysPressed.contains(Qt::Key_Space))
        {
            moveUpward += 1;
        }
        if (keysPressed.contains(Qt::Key_Control))
        {
            moveUpward -= 1;
        }

        camera.moveBy(glm::vec3{ moveRight, moveUpward, moveForward } * moveSpeed);
    }

    void onUpdate()
    {
        applyMovementTick();
        update();
    }

    void drawWorldLine(QPainter& painter, glm::vec3 worldBegin, glm::vec3 worldEnd)
    {
        auto beginNdc = camera.worldToScreen11(worldBegin);
        if (!beginNdc) return;

        auto endNdc = camera.worldToScreen11(worldEnd);
        if (!endNdc) return;

        QPointF beginPix = pixCoordOfNdcCoord(rect(), *beginNdc);
        QPointF endPix   = pixCoordOfNdcCoord(rect(), *endNdc);

        painter.drawLine(beginPix, endPix);
    }

private:
    BasicCamera camera;

    QTimer updateTimer;

    QSet<int> keysPressed;

    float moveSpeed = 0.0025;

    inline static const QSet<int> keysToTrack = { Qt::Key_W, Qt::Key_A, Qt::Key_S, Qt::Key_D, Qt::Key_Space, Qt::Key_Control };
};