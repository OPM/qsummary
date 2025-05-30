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

#ifndef XAXISTICKS_H
#define XAXISTICKS_H

#include <QtCharts/QChartGlobal>
#include <QtWidgets/QGraphicsItem>
#include <QtGui/QFont>

#include <memory>

QT_BEGIN_NAMESPACE
class QGraphicsSceneMouseEvent;
class QChart;


class XaxisTicks : public QGraphicsItem
{
public:
    XaxisTicks(QChart *parent);

    void set_xaxis_ticks(const std::vector<std::tuple<std::string, double>>& xaxis_ticks);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *widget);

    void prepare_update();
    void set_visible(bool value);


private:

    const int max_number_of_labels = 25;

    QChart *m_chart;
    std::vector<std::tuple<std::string, double>> m_xaxis_ticks;

    std::vector<std::unique_ptr<QGraphicsSimpleTextItem>> m_labels;
};

#endif // XAXISTICKS_H
