#include "BaseXYPadWidget.h"
#include <QPainter>

BaseXYPadWidget::BaseXYPadWidget(QWidget *parent) : QWidget(parent)
{
    
    QSizePolicy policy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    //policy.setHeightForWidth(true);
    setSizePolicy(policy);
    
    setMouseTracking(true);
}

QPointF BaseXYPadWidget::pixcoordToCoord11(QPointF localPixcoord)
{
    auto area   = coordinateArea();
    auto x11    = getValue11Clamped(localPixcoord.x(), area.left(), area.right  ());
    auto y11    = getValue11Clamped(localPixcoord.y(), area.top (), area.bottom ()) * -1.0;
    return QPointF(x11, y11);
}

QPointF BaseXYPadWidget::coord11ToPixcoord(QPointF coord11)
{
    auto areaF   = QRectF(coordinateArea());

    QPointF offset = QPointF(
        areaF.width () * value11To01(coord11.x()),
        areaF.height() * value11To01(coord11.y())
    );

    return areaF.bottomLeft() + QPointF(offset.x(), -offset.y());
}

void BaseXYPadWidget::paintEvent(QPaintEvent *event)
{
    QPainter p(this);

    QPen pen(palette().color(QPalette::Mid).lighter(135));
    pen.setWidth(1);
    p.setPen(pen);

    p.setBrush(palette().color(QPalette::AlternateBase).lighter(155));

    auto coordArea = coordinateArea();
    auto fixedCoordArea = coordArea.adjusted(0, 0, -1, -1);

    p.drawRect(fixedCoordArea);

    int midX = coordArea.center().x();
    int midY = coordArea.center().y();

    QLine verticalMidline = QLine(midX, coordArea.top(), midX, coordArea.bottom());
    QLine horizontalMidline = QLine(coordArea.left(), midY, coordArea.right(), midY);

    p.drawLine(verticalMidline);
    p.drawLine(horizontalMidline);
}

/*
int BaseXYPadWidget::heightForWidth(int width) const
{
    return width;
}
*/


QSize BaseXYPadWidget::sizeHint() const
{
    const int side = 200;
    return QSize(side, side);
}


QSize BaseXYPadWidget::minimumSizeHint() const
{
    // Allow shrinking as much as you want (or set something tiny like 10x10)
    return QSize(40, 40);
}

QRect BaseXYPadWidget::coordinateArea()
{
    const auto availableCoordArea = QRect(rect().topLeft() + offset, rect().bottomRight() - offset);

    if (constrainAreaToSquared)
    {
        auto minSize = std::min(availableCoordArea.width(), availableCoordArea.height());
        QRect squaredArea;
        squaredArea.setSize(QSize(minSize, minSize));
        squaredArea.moveCenter(availableCoordArea.center());

        return squaredArea;
    }
    else
    {
        return availableCoordArea;
    }
}

void BaseXYPadWidget::mousePressEvent(QMouseEvent *event)
{
    emit positionChanged( pixcoordToCoord11(event->pos()) );
    
    QWidget::mousePressEvent(event);
}

void BaseXYPadWidget::mouseMoveEvent(QMouseEvent *event)
{
    auto buttonIsPressed = event->buttons() & Qt::LeftButton;

    if (buttonIsPressed)
    {
        emit positionChanged( pixcoordToCoord11(event->pos()) );
    }

    QWidget::mouseMoveEvent(event);
}