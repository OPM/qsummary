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


#include <appl/xaxis_ticks.hpp>

#include <QtGui/QPainter>
#include <QtGui/QFontMetrics>
#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtGui/QMouseEvent>
#include <QtCharts/QChart>

#include <iostream>

XaxisTicks::XaxisTicks(QChart *chart):
    QGraphicsItem(chart),
    m_chart(chart)
{
    for (int n = 0; n < max_number_of_labels; n++){
        m_labels.push_back(std::make_unique<QGraphicsSimpleTextItem>(m_chart));
        m_labels.back()->setVisible(false);
    }
}

QRectF XaxisTicks::boundingRect() const
{
    QRectF rect = m_chart->rect();

    return rect;
}



void XaxisTicks::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QRectF rect1 = m_chart->rect();
    QRectF rect2 = m_chart->plotArea();

    auto bot_chart = rect1.y() + rect1.height();
    auto bot_plot = rect2.y() + rect2.height();

    auto lbl_ypos = bot_plot + (bot_chart - bot_plot)*0.05;

    for (int n = 0; n < max_number_of_labels; n++)
        m_labels[n]->setVisible(false);

    if (m_xaxis_ticks.size() > 0) {

        for (int n = 0; n <  m_xaxis_ticks.size(); n++) {

            QPainterPath path3;

            qreal xpos = rect2.x() + rect2.width()*static_cast<qreal>(std::get<1>(m_xaxis_ticks[n]));

            path3.moveTo(xpos, rect2.y());
            path3.lineTo(xpos, rect2.y() + rect2.height());

            painter->setPen(QPen(Qt::gray, 0.5, Qt::SolidLine));

            painter->drawPath(path3);


            QString lbl = QString::fromStdString(std::get<0>(m_xaxis_ticks[n]));

            m_labels[n]->setText(lbl);

            QFont font1("serifed style");  // default on Linux
            font1.setPointSize(8);

            //font1.setFamily("Arial");  // default on Windows
            //font1.setFamily("serifed style");  // default on Linux

            int left_shift;

            if (lbl.length() == 4)
                left_shift = 10;
            else if (lbl.length() == 8)
                left_shift = 20;
            else if (lbl.length() == 10)
                left_shift = 25;
            else if (lbl.length() == 11)
                left_shift = 28;
            else if (lbl.length() == 17)
                left_shift = 43;
            else if (lbl.length() == 20)
                left_shift = 50;
            else if (lbl.length() == 24)
                left_shift = 60;
            else
                left_shift = 0;

            m_labels[n]->setFont(font1);
            m_labels[n]->setPos(xpos - left_shift, lbl_ypos );
            m_labels[n]->setVisible(true);
        }
    }
}

void XaxisTicks::prepare_update()
{
    prepareGeometryChange();
}


void XaxisTicks::set_xaxis_ticks(const std::vector<std::tuple<std::string, double>>& xaxis_ticks)
{
    if (xaxis_ticks.size() > max_number_of_labels)
        throw std::runtime_error("number of xaxis ticks " + std::to_string(xaxis_ticks.size()) + " are larger than maximum " +
            std::to_string(max_number_of_labels)   );

    m_xaxis_ticks = xaxis_ticks;
}

void XaxisTicks::set_visible(bool value)
{
    this->setVisible(value);

    if (!value)
        for (int n = 0; n < max_number_of_labels; n++)
            m_labels[n]->setVisible(false);
}
