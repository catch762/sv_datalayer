#include "CameraViewport.h"

CameraViewport::CameraViewport(QWidget* parent) : QWidget(parent)
{
    setFocusPolicy(Qt::ClickFocus);

    setMinimumSize(50, 50);

    camera.setPos({ 3, 3, 3 });
    camera.lookAtWithoutRoll({ 0, 0, 0 });

    updateTimer.setSingleShot(false);
    updateTimer.setInterval(0);
    connect(&updateTimer, &QTimer::timeout, this, &CameraViewport::doUpdate);
}

int CameraViewport::getKey(QKeyEvent* event)
{
    // The problem is, when i change system language i no longer get same key code in event.
    // There are 3 ways to get key: key() or nativeScanCode() or nativeVirtualKey()

    // There is simply no good built-in-Qt cross-platform way to check
    // "is user pressing fucking W key regardless of his OS and layout language"

    // So i go with VK codes because they kinda match usual key() except for shift/control/alt.
    // This probably will break on linux and to fix this i need to reinvent a bicycle and
    // make another abstraction layer which i have no desire to do at the moment.

    auto k  = event->key();
    auto sc = event->nativeScanCode();
    auto vk = event->nativeVirtualKey();

    //SV_LOG(std::format("k {} sc {} vk {}", k, sc, vk));

    const int VK_SHIFT   = 16;
    const int VK_CONTROL = 17;
    const int VK_ALT     = 18;

    // Return code in same Qt format as typically in event->key()
    switch (vk)
    {
        case VK_SHIFT:      return Qt::Key_Shift;
        case VK_CONTROL:    return Qt::Key_Control;
        case VK_ALT:        return Qt::Key_Alt;
        default:            return vk;
    }
}

bool CameraViewport::hasPressedKeyForActionThatNeedsUpdatingWidget()
{
    for (auto pressedKey : keysPressed)
    {
        if (pressedKey != Qt::Key_Shift && pressedKey != Qt::Key_Alt)
        {
            return true;
        }
    }

    return false;
}

void CameraViewport::keyPressEvent(QKeyEvent* event)
{
    static const QSet<int> keysToTrack = {  Qt::Key_W,
                                            Qt::Key_A,
                                            Qt::Key_S,
                                            Qt::Key_D,
                                            Qt::Key_Q,
                                            Qt::Key_E,
                                            Qt::Key_Up,
                                            Qt::Key_Down,
                                            Qt::Key_Left,
                                            Qt::Key_Right,
                                            Qt::Key_Space,
                                            Qt::Key_Control,
                                            Qt::Key_Shift,
                                            Qt::Key_Alt };

    auto theKey = getKey(event);

    if (keysToTrack.contains(theKey))
    {
        keysPressed.insert(theKey);

        //so that we receive release even if we switch focus to other widget/window
        //grabKeyboard(); 

        setUpdatesEnabled(hasPressedKeyForActionThatNeedsUpdatingWidget());
    }
    else
    {
        QWidget::keyPressEvent(event);
    }
}

void CameraViewport::keyReleaseEvent(QKeyEvent* event)
{
    auto theKey = getKey(event);

    if (keysPressed.contains(theKey))
    {
        keysPressed.remove(theKey);

        if (keysPressed.isEmpty())
        {
            //releaseKeyboard();
        }

        setUpdatesEnabled(hasPressedKeyForActionThatNeedsUpdatingWidget());
    }
    else
    {
        QWidget::keyReleaseEvent(event);
    }
}

void CameraViewport::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        //because its crucial we receive release event so we can set cursor visible again
        grabMouse();
        setCursorVisible(false);

        lastClickPos = event->pos();
    }
    else QWidget::mousePressEvent(event);
}
void CameraViewport::mouseMoveEvent(QMouseEvent* event)
{
    QPoint posDelta = event->pos() - lastClickPos;

    if (!posDelta.isNull())
    {
        if (auto changed = applyMousePitchYawChange(posDelta))
        {
            emit cameraChanged(camera);
            update();
        }
    }

    QPoint lastClickPosGlobal = mapToGlobal(lastClickPos);
    QCursor::setPos(lastClickPosGlobal);

    //QWidget::mouseMoveEvent(event);
}

void CameraViewport::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        releaseMouse();
        setCursorVisible(true);
    }
    else QWidget::mouseMoveEvent(event);
}

void CameraViewport::paintEvent(QPaintEvent* event)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    float curAspect = float(rect().width()) / float(rect().height());
    camera.setAspect(curAspect);

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

BasicPlaneCamera& CameraViewport::getCamera()
{
    return camera;
}
const BasicPlaneCamera& CameraViewport::getCamera() const
{
    return camera;
}

void CameraViewport::setRenderFunc(const RenderFunc& func)
{
    renderFunc = func;
}

void CameraViewport::drawWorldAxes(QPainter& p)
{
    drawAxes(p, { 0,0,0 }, { 1,0,0 }, { 0,1,0 }, { 0,0,1 });
}

void CameraViewport::drawCameraAxes(QPainter& p, const BasicPlaneCamera& camera)
{
    drawAxes(p, camera.getPos(), camera.getDirRight(), camera.getDirUp(), camera.getDir());
}


void CameraViewport::drawStandardGrid(QPainter& p)
{
    const float cellsize = 0.5f;
    const int   linesCount = 2;
    const float gradMaxDist = linesCount * cellsize;

    auto makeGrad = [&](QColor beginColor)
        {
            beginColor.setAlpha(200);

            QColor endColor = beginColor;
            endColor.setAlpha(0);

            return LineGradientInfo{
                beginColor,
                endColor,
                gradMaxDist
            };
        };

    static auto xGrad = makeGrad(QColor(255, 25, 0));
    static auto yGrad = makeGrad(QColor(15, 225, 0));
    static auto zGrad = makeGrad(QColor(25, 25, 255));


    //drawWorldLineWithGradient(p, { -1, 1, 0 }, { 1,1,0 }, xGrad);
    //return;

    auto drawLine = [&](glm::vec3 worldA, glm::vec3 worldB, int axisId)
        {
            drawWorldLineWithGradient(p, worldA, worldB, axisId == 0 ? xGrad : (axisId == 1 ? yGrad : zGrad));
        };


    glm::vec3 camInCellSpace = camera.getPos() / cellsize;
    glm::vec3 closestCellCoordInCellSpace = glm::round(camInCellSpace);
    glm::vec3 closestCell = closestCellCoordInCellSpace * cellsize;

    traverseGrid(closestCell, cellsize, linesCount, drawLine);
}

void CameraViewport::drawAxes(QPainter& p, glm::vec3 pos, glm::vec3 dirX, glm::vec3 dirY, glm::vec3 dirZ, qreal lineWidth)
{
    setPen(p, Qt::red, lineWidth);
    drawWorldLine(p, pos, pos + dirX);
    setPen(p, Qt::green, lineWidth);
    drawWorldLine(p, pos, pos + dirY);
    setPen(p, Qt::blue, lineWidth);
    drawWorldLine(p, pos, pos + dirZ);
}

void CameraViewport::fillWithColor(QPainter& p, QColor color)
{
    p.fillRect(rect(), color);
}

void CameraViewport::setPen(QPainter& p, QColor color, qreal width)
{
    QPen linePen = QPen(color, width);
    p.setPen(linePen);
}

void CameraViewport::drawWorldLine(QPainter& painter, glm::vec3 worldBegin, glm::vec3 worldEnd)
{
    auto clippedLine = worldLineToScreen(camera.getViewProjection(), worldBegin, worldEnd);
    if (!clippedLine) return;

    auto beginNdc = clippedLine->first;
    auto endNdc = clippedLine->second;

    QPointF beginPix = pixCoordOfNdcCoord(rect(), beginNdc);
    QPointF endPix = pixCoordOfNdcCoord(rect(), endNdc);

    painter.drawLine(beginPix, endPix);
}

void CameraViewport::drawWorldLineWithGradient(QPainter& painter, glm::vec3 worldBegin, glm::vec3 worldEnd, const LineGradientInfo& grad)
{
    auto lineRes = worldLineToScreenAndWorldClip(camera.getViewProjection(),
        camera.getInvertedViewProjection(),
        worldBegin,
        worldEnd);
    if (!lineRes) return;

    Vec2Pair clippedLineNdc = lineRes->first;
    Vec3Pair clippedLineWorld = lineRes->second;

    QPointF beginPix = pixCoordOfNdcCoord(rect(), clippedLineNdc.first);
    QPointF endPix = pixCoordOfNdcCoord(rect(), clippedLineNdc.second);

    float   distToBegin = glm::distance(camera.getPos(), clippedLineWorld.first);
    float   distToEnd = glm::distance(camera.getPos(), clippedLineWorld.second);

    QColor  colorAtBegin = mixColors(grad.colorAtZeroDist, grad.colorAtMaxDist, distToBegin / grad.maxWorldDist);
    QColor  colorAtEnd = mixColors(grad.colorAtZeroDist, grad.colorAtMaxDist, distToEnd / grad.maxWorldDist);

    auto    gradient = QLinearGradient(beginPix, endPix);
    gradient.setColorAt(0.0, colorAtBegin);
    gradient.setColorAt(1.0, colorAtEnd);

    auto pen = painter.pen();
    pen.setBrush(QBrush(gradient));
    painter.setPen(pen);

    // 3. Draw the line
    painter.drawLine(beginPix, endPix);
}

int CameraViewport::printText(QPainter& p, const QString& text, QColor color, int startY)
{
    static const QFont fixedFont("Arial", 12);
    p.setFont(fixedFont);
    p.setPen(color);

    QRect requestedRect = rect();
    requestedRect.setTop(startY);

    // To ensure we use same flags for metrics and painter:
    int flags = Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap;

    QRect actualTextBoundingRect = p.boundingRect(requestedRect, flags, text);

    p.drawText(requestedRect, flags, text);

    return actualTextBoundingRect.bottom();
}


void CameraViewport::setCursorVisible(bool visible)
{
    if (visible)
    {
        QApplication::restoreOverrideCursor();
    }
    else
    {
        QApplication::setOverrideCursor(QCursor(Qt::BlankCursor));
    }
}

void CameraViewport::setUpdatesEnabled(bool enabled)
{
    if (enabled && !updateTimer.isActive()) updateTimer.start();
    else if (!enabled && updateTimer.isActive()) updateTimer.stop();
}

bool CameraViewport::applyMousePitchYawChange(QPoint mouseDelta)
{
    glm::vec3 pitchYawRoll = glm::vec3(-mouseDelta.y(), -mouseDelta.x(), 0);
    //glm::vec3 pitchYawRoll = glm::vec3(-mouseDelta.y(), 0, mouseDelta.x());

    pitchYawRoll *= 0.008 * mouseSens;

    if (glm::length(pitchYawRoll) > 0.000001)
    {
        camera.addAngles(pitchYawRoll);
        return true;
    }
    else return false;
}

//returns true if camera changed
bool CameraViewport::applyMovementByKeys()
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

    auto moveVec = glm::vec3{ moveRight, moveUpward, moveForward } * moveSpeed * getShiftOrAltSpeedModifier();

    if (glm::length(moveVec) > 0.000001)
    {
        camera.moveBy(moveVec);
        return true;
    }
    else return false;
}

bool CameraViewport::applyRotationByKeys()
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
    if (keysPressed.contains(Qt::Key_Q))
    {
        deltaRoll -= 1;
    }
    if (keysPressed.contains(Qt::Key_E))
    {
        deltaRoll += 1;
    }

    glm::vec3 pitchYawRollDeltasRadians = { deltaPitch, deltaYaw, deltaRoll };

    pitchYawRollDeltasRadians *= 0.008;

    if (glm::length(pitchYawRollDeltasRadians) > 0.00001)
    {
        camera.addAngles(pitchYawRollDeltasRadians);
        return true;
    }
    else return false;
}

float CameraViewport::getShiftOrAltSpeedModifier()
{
    float modifier = 1.0;

    if (keysPressed.contains(Qt::Key_Shift)) modifier *= 8;
    if (keysPressed.contains(Qt::Key_Alt  )) modifier /= 8;

    return modifier;
}

void CameraViewport::doUpdate()
{
    bool changed1 = applyMovementByKeys();
    bool changed2 = applyRotationByKeys();

    if (changed1 || changed2)
    {
        emit cameraChanged(camera);
    }

    update();
}