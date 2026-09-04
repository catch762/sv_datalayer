#pragma once
#include "sv_qtcommon.h"
#include <QTimer>
#include <QTextLayout>
#include <QApplication>

//todo delete this its unused, only for stub
struct CameraData
{
    glm::vec3 pos = {};
    float	  roll = 0;
    glm::vec3 dir = glm::vec3(0, 0, -1);
    float	  fov = 45;
};
SV_REGTYPENAME(CameraData);


//*************************************************************************************
//
// CameraViewport looks like rectangle with what camera sees, no any other ui elements.
// 
// It owns its BasicPlaneCamera and provides controlling via keys/mouse,
// as long as you clicked on a widget.
//
// It also provides all the drawing functions like drawLine, etc.
// 
// But its other class responsibility to decide what to actually draw, this is done
// via setting RenderFunc on CameraViewport.
// 
//*************************************************************************************
class CameraViewport : public QWidget
{
    Q_OBJECT
public:
    CameraViewport(QWidget* parent = nullptr);

    using RenderFunc = std::function<void(CameraViewport& vp, QPainter& p)>;

    static int getKey(QKeyEvent* event);

    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

    void mouseReleaseEvent(QMouseEvent* event) override;

    void paintEvent(QPaintEvent* event) override;

    BasicPlaneCamera& getCamera();
    const BasicPlaneCamera& getCamera() const;

    template<typename CameraChangerFunc>
        requires std::is_invocable_v<CameraChangerFunc, BasicPlaneCamera&>
    void changeCamera(const CameraChangerFunc& changerFunc)
    {
        changerFunc(camera);
        emit cameraChanged(camera);
        update();
    }

    void setRenderFunc(const RenderFunc& func);

    void setMoveSpeed(float newMoveSpeed)
    {
        moveSpeed = newMoveSpeed;
    }
    float getMoveSpeed() const
    {
        return moveSpeed;
    }

    void setMouseSens(float newMouseSens)
    {
        mouseSens = newMouseSens;
    }
    float getMouseSens() const
    {
        return mouseSens;
    }

    //Painting methods:
public:
    struct LineGradientInfo
    {
        QColor  colorAtZeroDist = Qt::red;
        QColor  colorAtMaxDist = Qt::blue;
        float   maxWorldDist = 1;
    };

    void drawWorldAxes(QPainter& p);

    //this draws camera forward dir as Z axis, although its -Z really
    void drawCameraAxes(QPainter& p, const BasicPlaneCamera& camera);

    template<typename LineVisitor>
        requires std::is_invocable_r_v<void, LineVisitor, glm::vec3, glm::vec3, int /*id, x=0 y=1 z=2*/>
    void traverseGrid(glm::vec3 center, float lineSpacing, int linesCount, const LineVisitor& lineVisitor);

    void drawStandardGrid(QPainter& p);

    void drawAxes(QPainter& p, glm::vec3 pos, glm::vec3 dirX, glm::vec3 dirY, glm::vec3 dirZ, qreal lineWidth = 3);

    void fillWithColor(QPainter& p, QColor color = QColor(200, 200, 200));

    void setPen(QPainter& p, QColor color, qreal width = 3);


    //uses current pen
    void drawWorldLine(QPainter& painter, glm::vec3 worldBegin, glm::vec3 worldEnd);

    void drawWorldLineWithGradient(QPainter& painter, glm::vec3 worldBegin, glm::vec3 worldEnd, const LineGradientInfo& grad);

    // Tries to print text within rectangle that is like rect() but starts at startY;
    // Prints text on multiple lines if needed.
    // returns 'endY' - coordinates of last line actually printed.
    int printText(QPainter& p, const QString& text, QColor color = Qt::white, int startY = 0);

signals:
    void cameraChanged(const BasicPlaneCamera& camera);

private:
    bool hasPressedKeyForActionThatNeedsUpdatingWidget();

    void setCursorVisible(bool visible);

    void setUpdatesEnabled(bool enabled);

    //These applySomething functions return true if camera was actually changed
    bool applyMousePitchYawChange(QPoint mouseDelta);
    bool applyMovementByKeys();
    bool applyRotationByKeys();

    float getShiftOrAltSpeedModifier();

    void doUpdate();

private:
    BasicPlaneCamera camera;

    QTimer updateTimer;

    QSet<int> keysPressed;

    float moveSpeed = 0.0025;
    float mouseSens = 1.0;

    QPoint lastClickPos;

    RenderFunc renderFunc;
};


template<typename LineVisitor>
        requires std::is_invocable_r_v<void, LineVisitor, glm::vec3, glm::vec3, int /*id, x=0 y=1 z=2*/>
void CameraViewport::traverseGrid(glm::vec3 center, float lineSpacing, int linesCount, const LineVisitor& lineVisitor)
{
    const float lineLength = lineSpacing * (linesCount) * 2.0;

    auto drawLinesAtLevel = [&](glm::vec3 midPos, glm::vec3 normalizedDir, glm::vec3 differenceDir, int axisId)
        {
            for (int i = -linesCount; i <= linesCount; ++i)
            {
                glm::vec3 thisMidpos = midPos + differenceDir * lineSpacing * float(i);
                glm::vec3 thisLineBegin = thisMidpos - normalizedDir * lineLength * 0.5f;
                glm::vec3 thisLineEnd = thisMidpos + normalizedDir * lineLength * 0.5f;

                lineVisitor(thisLineBegin, thisLineEnd, axisId);
            }
        };

    auto drawLinesAlongOtherAxis = [&](glm::vec3 masterMidPos,
        glm::vec3 masterDifferenceDir,
        glm::vec3 subNormalizedDir,
        glm::vec3 subDifferenceDir,
        int       axisId)
        {
            for (int i = -linesCount; i <= linesCount; ++i)
            {
                glm::vec3 thisMidpos = masterMidPos + masterDifferenceDir * lineSpacing * float(i);

                drawLinesAtLevel(thisMidpos, subNormalizedDir, subDifferenceDir, axisId);
            }
        };

    const glm::vec3 XAxis(1, 0, 0);
    const glm::vec3 YAxis(0, 1, 0);
    const glm::vec3 ZAxis(0, 0, 1);


    //Layers on Y, containing lines in X direction, that are spaced by Z
    drawLinesAlongOtherAxis(center, YAxis, XAxis, ZAxis, 0);

    drawLinesAlongOtherAxis(center, YAxis, ZAxis, XAxis, 2);

    drawLinesAlongOtherAxis(center, XAxis, YAxis, ZAxis, 1);
}