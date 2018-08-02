/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Charts module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "chartview.h"
#include <QtGui/QMouseEvent>

ChartView::ChartView(QWidget *parent) :
    QChartView(parent),
    m_isTouching(false)
{
    setRubberBand(QChartView::RectangleRubberBand);
//    setDragMode(QGraphicsView::NoDrag);
//    this->setMouseTracking(true);
}

ChartView::ChartView(QChart *chart, QWidget *parent) :
    QChartView(chart, parent),
    m_isTouching(false)
{
    setRubberBand(QChartView::RectangleRubberBand);
//    setDragMode(QGraphicsView::NoDrag);
//    this->setMouseTracking(true);
}

void ChartView::setMinMax(double minx, double maxx, double miny, double maxy)
{
    minx_ = minx;
    maxx_ = maxx;
    miny_ = miny;
    maxy_ = maxy;
}

bool ChartView::viewportEvent(QEvent *event)
{
    if (event->type() == QEvent::TouchBegin) {
        m_isTouching = true;
        chart()->setAnimationOptions(QChart::NoAnimation);
    }
    return QChartView::viewportEvent(event);
}

void ChartView::mousePressEvent(QMouseEvent *event)
{

    if(event->button() == Qt::RightButton){
        chart()->zoomOut();
        checkZoom();
    } else if(event->button() == Qt::MiddleButton){
        QApplication::setOverrideCursor(QCursor(Qt::SizeAllCursor));
        lastMousePos = event->pos();
        QRectF bounds = currentViewRect();
        viewWidth = bounds.right()-bounds.left();
        viewHeight = bounds.top()-bounds.bottom();
        chart()->setAnimationOptions(QChart::NoAnimation);
        event->accept();
    } else{
        if (m_isTouching)
            return;
    }
    QChartView::mousePressEvent(event);

}

void ChartView::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() == Qt::MiddleButton){
        QRectF bounds = currentViewRect();

        QPoint evpos = event->pos();
        QPointF p1 = chart()->mapToValue(evpos);
        QPointF p2 = chart()->mapToValue(lastMousePos);
        auto dPos = p1-p2;
        double dx = dPos.x();
        double dy = dPos.y();

        auto axh = chart()->axes(Qt::Horizontal);
        double newXmin = bounds.left()-dx;
        double newXmax = bounds.left()+viewWidth-dx;
        if(newXmin<minx_){
            newXmin = minx_;
            newXmax = minx_+viewWidth;
        }
        if(newXmax>maxx_){
            newXmax = maxx_;
            newXmin = maxx_-viewWidth;
        }
        chart()->axisX()->setRange(newXmin,newXmax);

        auto axv = chart()->axes(Qt::Vertical);
        double newYmax = bounds.top()-dy;
        double newYmin = bounds.top()-viewHeight-dy;
        if(newYmin<miny_){
            newYmin = miny_;
            newYmax = miny_+viewHeight;
        }
        if(newYmax>maxy_){
            newYmax = maxy_;
            newYmin = maxy_-viewHeight;
        }
        chart()->axisY()->setRange(newYmin,newYmax);

        lastMousePos = evpos;
        event->accept();
    }else{
        if (m_isTouching)
            return;
    }
    QChartView::mouseMoveEvent(event);
}

void ChartView::mouseReleaseEvent(QMouseEvent *event)
{

    if(event->button() == Qt::RightButton){
        chart()->zoomOut();
        checkZoom();
    } else if(event->button() == Qt::MiddleButton){
        QApplication::restoreOverrideCursor();
        chart()->setAnimationOptions(QChart::SeriesAnimations);
    } else{
        if (m_isTouching)
            m_isTouching = false;

        // Because we disabled animations when touch event was detected
        // we must put them back on.
        chart()->setAnimationOptions(QChart::SeriesAnimations);

        QChartView::mouseReleaseEvent(event);
    }
}

void ChartView::mouseDoubleClickEvent(QMouseEvent *event)
{
    chart()->zoomIn();
}

void ChartView::wheelEvent(QWheelEvent *event)
{
    if(event->delta()>0){
        chart()->zoomIn();
    }else{
        chart()->zoomOut();
        checkZoom();
    }
}

//![1]
void ChartView::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Plus:
        chart()->zoomIn();
        break;
    case Qt::Key_Minus:
        chart()->zoomOut();
        checkZoom();
        break;
//![1]
    case Qt::Key_Left:
        chart()->scroll(-10, 0);
        break;
    case Qt::Key_Right:
        chart()->scroll(10, 0);
        break;
    case Qt::Key_Up:
        chart()->scroll(0, 10);
        break;
    case Qt::Key_Down:
        chart()->scroll(0, -10);
        break;
    default:
        QGraphicsView::keyPressEvent(event);
        break;
    }
}

void ChartView::checkZoom()
{
    auto ax = chart()->axes();
    QValueAxis *axx = qobject_cast<QValueAxis *>(ax.at(0));
    double minx = axx->min();
    double maxx = axx->max();
    if(minx<minx_){
        minx = minx_;
    }
    if(maxx>maxx_){
        maxx = maxx_;
    }
    QValueAxis *axy = qobject_cast<QValueAxis *>(ax.at(1));
    double miny = axy->min();
    double maxy = axy->max();
    if(miny<miny_){
        miny = miny_;
    }
    if(maxy>maxy_){
        maxy = maxy_;
    }
    axx->setRange(minx,maxx);
    axy->setRange(miny,maxy);
}

QRectF ChartView::currentViewRect()
{
    auto ax = chart()->axes();
    QValueAxis *axx = qobject_cast<QValueAxis *>(ax.at(0));
    double minx = axx->min();
    double maxx = axx->max();
    QValueAxis *axy = qobject_cast<QValueAxis *>(ax.at(1));
    double miny = axy->min();
    double maxy = axy->max();
    return QRectF(QPointF(minx,maxy),QPointF(maxx,miny));
}
