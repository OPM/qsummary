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


#include <QtCharts/QValueAxis>

#include <random>
#include <iostream>
#include <math.h>

#include <appl/smry_series.hpp>

enum class AxisMultiplierType { one, thousand, million, billion };

class SmryYaxis: public QValueAxis {

public:

    SmryYaxis(AxisMultiplierType mult_type, float mult, QObject *parent = nullptr);

    float multiplier() { return axis_multiplier; };

    AxisMultiplierType multiplier_type() { return axis_multiplier_type; };

    void update_series_data(AxisMultiplierType mult_type, float mult, const std::vector<SmrySeries*>& series);
    void update_axis_multiplier(const std::vector<SmrySeries*>& series);
    void view_title();

    void remove_last_unit();

    bool set_range(double min_val, double max_val);
    void print_axis_range();

    void add_title(std::string title);
    void setMinAndMax(qreal min_val, qreal max_val);

    void resetAxisRange();

private slots:

    void rangeChanged(qreal min, qreal max);

private:

   std::vector<std::string> m_titles;

   float calc_p90(const std::vector<float>& data);

   float axis_multiplier;

   AxisMultiplierType axis_multiplier_type;

   double m_min;
   double m_max;
};


