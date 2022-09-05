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

#ifndef SMRY_APPL_SERIES_HPP
#define SMRY_APPL_SERIES_HPP

#include <appl/point_info.hpp>

#include <QtCharts>
#include <QtCharts/QLineSeries>

#include <random>
#include <iostream>
#include <math.h>


class PointInfo;


class SmrySeries: public QtCharts::QLineSeries {

public:

    SmrySeries(QChart *qtchart, QObject *parent = nullptr);
    void print_data();

    std::tuple<double,double> get_min_max_value() { return std::make_tuple(m_glob_min, m_glob_max); };
    std::tuple<double,double> get_min_max_value(double xfrom, double xto, bool ignore_zero = false);
    bool all_values_zero();
    void calcMinAndMax();

private slots:

    void onHovered(const QPointF &point, bool state);
    void onPressed(const QPointF &point);


private:
     PointInfo *m_tooltip;
     QChart *m_chart;

     QPointF calculate_closest(const QPointF point);

     double m_glob_min;
     double m_glob_max;
};


#endif // SMRY_APPL_SERIES_HPP
