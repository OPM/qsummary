/*
   Copyright 2022 Equinor ASA.

   This file is part of the Open Porous Media project (OPM).

   OPM is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   OPM is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with OPM.  If not, see <http://www.gnu.org/licenses/>.
   */

#include <appl/chartview.hpp>

#include <QtGui/QMouseEvent>
#include <QApplication>

#include <iostream>

ChartView::ChartView(QChart *chart, QWidget *parent) :
    QChartView(chart, parent),
    m_chart(chart),
    m_isTouching(false),
    m_parent(parent)
{
    setRubberBand(QChartView::RectangleRubberBand);

    m_xaxis_obj = new XaxisTicks(m_chart);
}


void ChartView::keyPressEvent(QKeyEvent *event)
{

    switch (event->key()) {
    case Qt::Key_Plus:
        chart()->zoomIn();
        break;
    case Qt::Key_Minus:
        chart()->zoomOut();
        break;
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
        QApplication::sendEvent(m_parent, event);
        break;
    }
}

void ChartView::resizeEvent(QResizeEvent *event)
{
    if (scene()) {
         scene()->setSceneRect(QRect(QPoint(0, 0), event->size()));
         m_chart->resize(event->size());
    }

    QGraphicsView::resizeEvent(event);
}


void ChartView::set_xaxis_ticks(const std::vector<std::tuple<std::string, double>>& xaxis_ticks)
{
    m_xaxis_ticks = xaxis_ticks;
    m_xaxis_obj->set_xaxis_ticks(xaxis_ticks);
}

void ChartView::update_geometry()
{
    m_xaxis_obj->prepare_update();
}


void ChartView::update_graphics()
{
    // repaint is not working
    //this->repaint();

    // this works but this is not pretty
    // will keep this for now

    QRect cv_rect = this->rect();

    this->resize(cv_rect.width()+1, cv_rect.height());
    this->resize(cv_rect.width()-1, cv_rect.height());

}

void ChartView::hide_xaxis_obj()
{
    m_xaxis_obj->set_visible(false);
}

void ChartView::show_xaxis_obj()
{
    m_xaxis_obj->set_visible(true);
}