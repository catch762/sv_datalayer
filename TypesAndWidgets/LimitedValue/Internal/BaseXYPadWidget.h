#pragma once
#include "sv_qtcommon.h"
#include <QMouseEvent>

// The most base class for creating XY pad widgets:
// it doesnt really have anything, just provides mapping to normalized space
// and draws the area.

class BaseXYPadWidget : public QWidget
{
    Q_OBJECT
public:
    BaseXYPadWidget(QWidget *parent = nullptr);

    //bottom left corner will map to (-1,-1), top right to (1,1)
    QPointF pixcoordToCoord11(QPointF localPixcoord);

    QPointF coord11ToPixcoord(QPointF coord11);

protected:
    void paintEvent(QPaintEvent *event) override;

    int heightForWidth(int width) const;
    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    QRect coordinateArea();

    void mousePressEvent(QMouseEvent *event) override
    {
        auto x = event->pos().x();
        auto y = event->pos().y();
        auto z = pixcoordToCoord11(QPoint(x,y)).x();
        auto w = pixcoordToCoord11(QPoint(x,y)).y();

        SV_LOG( QString("%1 %2 --- %3 %4").arg(x).arg(y).arg(z).arg(w).toStdString() );
    }

private:
    //actual coordinateArea() is between [offset.x, width - 2*offset.x], same for height
    QPoint offset = QPoint(10,10);
    bool constrainAreaToSquared = true;
};