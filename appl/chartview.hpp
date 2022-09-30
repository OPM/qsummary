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

#ifndef CHARTVIEW_H
#define CHARTVIEW_H

#include <QtCharts/QChartView>
#include <QtWidgets/QRubberBand>

#include<appl/xaxis_ticks.hpp>

QT_CHARTS_USE_NAMESPACE

class ChartView : public QChartView
{
public:

    ChartView(QChart *chart, QWidget *parent = 0);

    void set_xaxis_ticks(const std::vector<std::tuple<std::string, double>>& xaxis_ticks);
    void update_geometry();

    void update_graphics();
    void hide_xaxis_obj();
    void show_xaxis_obj();

protected:

    void keyPressEvent(QKeyEvent *event);
    void resizeEvent(QResizeEvent *event);


private:
    bool m_isTouching;
    QWidget *m_parent;

    QChart *m_chart;

    XaxisTicks *m_xaxis_obj;
    std::vector<std::tuple<std::string, double>> m_xaxis_ticks;
};

#endif
