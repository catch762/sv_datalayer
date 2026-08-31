#pragma once
#include "sv_qtcommon.h"
#include <QTimer>
#include <QTextLayout>

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
        setFocusPolicy(Qt::ClickFocus);

		setMinimumSize(50, 50);

        camera.setPos({ 3, 3, 3 });
        camera.lookAtWithoutRoll({ 0, 0, 0 });

        updateTimer.setSingleShot(false);
        updateTimer.setInterval(0);
        connect(&updateTimer, &QTimer::timeout, this, &CameraViewport::onUpdate);
	}

    using RenderFunc = std::function<void(CameraViewport& vp, QPainter& p)>;

    int getKey(QKeyEvent* event)
    {
        //or nativeScanCode() or nativeVirtualKey() ?
        return event->key();
    }

    void keyPressEvent(QKeyEvent* event) override
    {
        static const QSet<int> keysToTrack = {  Qt::Key_W,
                                                Qt::Key_A,
                                                Qt::Key_S,
                                                Qt::Key_D,
                                                Qt::Key_Up,
                                                Qt::Key_Down,
                                                Qt::Key_Left,
                                                Qt::Key_Right,
                                                Qt::Key_Space, 
                                                Qt::Key_Control };

        auto theKey = getKey(event);

        if (keysToTrack.contains(theKey))
        {
            keysPressed.insert(theKey);

            setUpdatesEnabled(true);
        }
        else
        {
            QWidget::keyPressEvent(event);
        }
    }
    void keyReleaseEvent(QKeyEvent* event) override
    {
        auto theKey = getKey(event);

        if (keysPressed.contains(theKey))
        {
            keysPressed.remove(theKey);

            setUpdatesEnabled(!keysPressed.isEmpty());
        }
        else
        {
            QWidget::keyReleaseEvent(event);
        }
    }

    void mousePressEvent(QMouseEvent* event) override
    {
        if (event->button() == Qt::LeftButton)
        {
            lastMousePos = event->pos();
        }

        QWidget::mousePressEvent(event);
    }
    void mouseMoveEvent(QMouseEvent* event) override
    {
        QPoint posDelta = event->pos() - lastMousePos;

        if (!posDelta.isNull())
        {
            applyMousePitchYawChange(posDelta);
            update();
        }

        lastMousePos = event->pos();

        //QWidget::mouseMoveEvent(event);
    }

    void mouseReleaseEvent(QMouseEvent* event) override
    {
        QWidget::mouseMoveEvent(event);
    }

    void paintEvent(QPaintEvent* event) override
    {
        QPainter p(this);

        if (renderFunc)
        {
            renderFunc(*this, p);
        }
        else
        {
            //default render: just draw world coord system
            fillWithColor(p);
            drawWorldAxes(p);
        }
    }

    BasicPlaneCamera& getCamera()
    {
        return camera;
    }
    const BasicPlaneCamera& getCamera() const
    {
        return camera;
    }

    void setRenderFunc(const RenderFunc& func)
    {
        renderFunc = func;
    }

    //Painting methods:
public:
    void drawWorldAxes(QPainter& p)
    {
        drawAxes(p, { 0,0,0 }, { 1,0,0 }, { 0,1,0 }, { 0,0,1 });
    }

    //this draws camera forward dir as Z axis, although its -Z really
    void drawCameraAxes(QPainter& p, const BasicPlaneCamera& camera)
    {
        drawAxes(p, camera.getPos(), camera.getDirRight(), camera.getDirUp(), camera.getDir());
    }

    void drawGrid(QPainter& p, glm::vec3 center, float lineSpacing, int linesCount, QPen xPen, QPen yPen, QPen zPen)
    {
        const float lineLength = lineSpacing * (linesCount) * 2.0;

        auto drawLinesAtLevel = [&](glm::vec3 midPos, glm::vec3 normalizedDir, glm::vec3 differenceDir)
        {
            for (int i = -linesCount; i <= linesCount; ++i)
            {
                glm::vec3 thisMidpos    = midPos + differenceDir * lineSpacing * float(i);
                glm::vec3 thisLineBegin = thisMidpos - normalizedDir * lineLength * 0.5f;
                glm::vec3 thisLineEnd   = thisMidpos + normalizedDir * lineLength * 0.5f;

                drawWorldLine(p, thisLineBegin, thisLineEnd);
            }
        };

        auto drawLinesAlongOtherAxis = [&]( glm::vec3 masterMidPos,
                                            glm::vec3 masterDifferenceDir,
                                            glm::vec3 subNormalizedDir,     glm::vec3 subDifferenceDir)
        {
            for (int i = -linesCount; i <= linesCount; ++i)
            {
                glm::vec3 thisMidpos = masterMidPos + masterDifferenceDir * lineSpacing * float(i);

                drawLinesAtLevel(thisMidpos, subNormalizedDir, subDifferenceDir);
            }
        };

        const glm::vec3 XAxis(1, 0, 0);
        const glm::vec3 YAxis(0, 1, 0);
        const glm::vec3 ZAxis(0, 0, 1);

      
        //Layers on Y, containing lines in X direction, that are spaced by Z
        p.setPen(xPen);
        drawLinesAlongOtherAxis(center, YAxis, XAxis, ZAxis);

        p.setPen(zPen);
        drawLinesAlongOtherAxis(center, YAxis, ZAxis, XAxis);

        p.setPen(yPen);
        drawLinesAlongOtherAxis(center, XAxis, YAxis, ZAxis);
    }

    void drawStandardGrid(QPainter& p)
    {
        drawGrid(p, {}, 1, 1, QPen(Qt::cyan), QPen(Qt::magenta), QPen(Qt::yellow));
    }

    void drawAxes(QPainter& p, glm::vec3 pos, glm::vec3 dirX, glm::vec3 dirY, glm::vec3 dirZ, qreal lineWidth = 3)
    {
        setPen(p, Qt::red, lineWidth);
        drawWorldLine(p, pos, pos + dirX);
        setPen(p, Qt::green, lineWidth);
        drawWorldLine(p, pos, pos + dirY);
        setPen(p, Qt::blue, lineWidth);
        drawWorldLine(p, pos, pos + dirZ);
    }

    void fillWithColor(QPainter& p, QColor color = QColor(200, 200, 200))
    {
        p.fillRect(rect(), color);
    }

    void setPen(QPainter& p, QColor color, qreal width = 3)
    {
        QPen linePen = QPen(color, width);
        p.setPen(linePen);
    }

    //uses current pen
    void drawWorldLine(QPainter& painter, glm::vec3 worldBegin, glm::vec3 worldEnd)
    {
        auto clippedLine = worldLineToScreen(camera.getViewProjection(), worldBegin, worldEnd);
        if (!clippedLine) return;

        auto beginNdc    = clippedLine->first;
        auto endNdc      = clippedLine->second;

        QPointF beginPix = pixCoordOfNdcCoord(rect(), beginNdc);
        QPointF endPix   = pixCoordOfNdcCoord(rect(), endNdc);

        painter.drawLine(beginPix, endPix);
    }

    // Tries to print text within rectangle that is like rect() but starts at startY;
    // Prints text on multiple lines if needed.
    // returns 'endY' - coordinates of last line actually printed.
    int printText(QPainter& p, const QString& text, int startY = 0)
    {
        static const QFont fixedFont("Arial", 12);
        p.setFont(fixedFont);

        QRect requestedRect = rect();
        requestedRect.setTop(startY);

        // To ensure we use same flags for metrics and painter:
        int flags = Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap;

        QRect actualTextBoundingRect = p.boundingRect(requestedRect, flags, text);

        p.drawText(requestedRect, flags, text);

        return actualTextBoundingRect.bottom();
    }

signals:
    void cameraChanged(const BasicPlaneCamera& camera);

private:
    void setUpdatesEnabled(bool enabled)
    {
        if      ( enabled && !updateTimer.isActive()) updateTimer.start();
        else if (!enabled &&  updateTimer.isActive()) updateTimer.stop();
    }

    void applyMousePitchYawChange(QPoint mouseDelta)
    {
        glm::vec3 pitchYawRoll = glm::vec3(-mouseDelta.y(), -mouseDelta.x(), 0);
        //glm::vec3 pitchYawRoll = glm::vec3(-mouseDelta.y(), 0, mouseDelta.x());

        pitchYawRoll *= 0.008;

        if (glm::length(pitchYawRoll) > 0.000001)
        {
            camera.addAngles(pitchYawRoll);
            emit cameraChanged(camera);
        }
    }

    //returns true if camera changed
    bool applyMovementByKeys()
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

        auto moveVec = glm::vec3{ moveRight, moveUpward, moveForward } * moveSpeed;

        if (glm::length(moveVec) > 0.000001)
        {
            camera.moveBy(moveVec);
            return true;
        }
        else return false;
    }

    //returns true if camera changed
    bool applyRotationByKeys()
    {
        float deltaPitch = 0;
        float deltaYaw = 0;
        float deltaRoll = 0;

        if (keysPressed.contains(Qt::Key_Up))
        {
            deltaPitch += 1;
        }
        if (keysPressed.contains(Qt::Key_Down))
        {
            deltaPitch -= 1;
        }
        if (keysPressed.contains(Qt::Key_Left))
        {
            deltaYaw += 1;
        }
        if (keysPressed.contains(Qt::Key_Right))
        {
            deltaYaw -= 1;
        }

        glm::vec3 pitchYawRollDeltasRadians = { deltaPitch, deltaYaw, deltaRoll };

        pitchYawRollDeltasRadians *= 0.003;

        if (glm::length(pitchYawRollDeltasRadians) > 0.00001)
        {
            camera.addAngles(pitchYawRollDeltasRadians);
            return true;
        }
        else return false;
    }

    void onUpdate()
    {
        bool changed1 = applyMovementByKeys();
        bool changed2 = applyRotationByKeys();

        if (changed1 || changed2)
        {
            emit cameraChanged(camera);
        }

        update();
    }

private:
    BasicPlaneCamera camera;

    QTimer updateTimer;

    QSet<int> keysPressed;

    float moveSpeed = 0.0025;

    QPoint lastMousePos;

    RenderFunc renderFunc;
};

// 1 or 0.
// This adds second viewport, which renders same as first one, but with external camera,
// and you also can see first camera.
#define CAMERAWIDGET_ENABLE_DEBUGVIEWPORT 1

class CameraWidget : public QWidget
{
    Q_OBJECT
public:
    CameraWidget(QWidget* parent = nullptr) : QWidget(parent)
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

        //all controls on the right
        controlsLayout = new QGridLayout();
        layout->addLayout(controlsLayout, 0, controlsLayoutColumn);
    }

private:
    static void renderScene(CameraViewport& vp, QPainter& p, const CameraViewport* additionalCameraToRender = nullptr)
    {
        vp.fillWithColor(p, additionalCameraToRender ? QColor(0, 0, 0) : QColor(55, 55, 55));

        vp.drawStandardGrid(p);

        vp.drawWorldAxes(p);

        if (additionalCameraToRender)
        {
            vp.drawCameraAxes(p, additionalCameraToRender->getCamera());
        }

        int textY = 0;
        textY = vp.printText(p, QString::fromStdString(vp.getCamera().toString()), textY);
    }

private:
    QGridLayout*    layout                  = nullptr;
    CameraViewport*     cameraViewport      = nullptr;
#if CAMERAWIDGET_ENABLE_DEBUGVIEWPORT
    CameraViewport*     cameraViewportDbg  = nullptr;
#endif
    QGridLayout*        controlsLayout      = nullptr;
};