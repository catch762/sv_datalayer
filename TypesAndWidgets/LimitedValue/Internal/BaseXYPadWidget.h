#pragma once
#include "sv_qtcommon.h"
#include <QMouseEvent>

//***********************************************************************
//
// The most base class for creating XY pad widgets.
// Pretty much just sets up 'workingArea' mapping and draws it.
//
// - The 'workingArea': pixelwise its some rectangle within a widget,
//   that has a 'Coord11' mapping to and from it: 'Coord11' space is
//   from -1 to 1 on both axis. 
//
//   Bottom left pixel of workingArea is always (-1, -1)
//   Top right pixel of workingArea is always (1, 1)
//
//   You may map this to any range in user classes, but for this class's
//   perspective, coords are never outside [-1, 1].
//
//   Working area may or may not be a square.
//
//***********************************************************************

class BaseXYPadWidget : public QWidget
{
    Q_OBJECT
public:
    BaseXYPadWidget(QWidget *parent = nullptr);

    //bottom left corner will map to (-1,-1), top right to (1,1)
    QPointF pixcoordToCoord11(QPointF localPixcoord);

    QPointF coord11ToPixcoord(QPointF coord11);

signals:
    void positionChanged(QPointF coord11);

protected:
    void paintEvent(QPaintEvent *event) override;

    //int heightForWidth(int width) const;
    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    QRect coordinateArea();

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    //actual coordinateArea() is between [offset.x, width - 2*offset.x], same for height
    QPoint offset = QPoint(3,3);
    bool constrainAreaToSquared = false;
};