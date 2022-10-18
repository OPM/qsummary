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

#include <QtCharts>
#include <QtCharts/QDateTimeAxis>

#include <random>
#include <iostream>
#include <math.h>

#include <appl/chartview.hpp>

class SmryXaxis: public QtCharts::QDateTimeAxis {

public:

    using tick_type = std::vector<std::tuple<std::string, double>>;

    SmryXaxis(ChartView *chart_view, QObject *parent = nullptr);

    bool set_range(std::string argstr);
    void set_full_range(double min, double max);
    bool has_xrange() { return xrange_set; }
    void set_xrange(bool value) { xrange_set = value; }

    bool has_full_range();

    void resetAxisRange();
    void print_ranges();

    std::tuple<double, double> get_xrange();
    std::tuple<QDateTime, QDateTime> get_current_xrange();
    std::vector<QDateTime> get_xrange_state();
    void set_xrange_state(const std::vector<QDateTime>& xrange_state);
    void print_xrange_state();


private slots:

    void rangeChanged(QDateTime min, QDateTime max);

private:

    void print_time_string(qint64 ms);

    tick_type make_raw_list(int nThick);

    tick_type make_second_list();
    tick_type make_minute_list();
    tick_type make_hr_list();
    tick_type make_day_list();
    tick_type make_month_list();
    tick_type make_year_list();

    std::string get_month_string(int ind);

    bool get_datetime_from_string(std::string str_arg, QDateTime& dt);

    QDateTime m_dt_min_utc;
    QDateTime m_dt_max_utc;

    bool xrange_set;

    QDateTime xrange_from;
    QDateTime xrange_to;

    QDateTime full_xrange_from;
    QDateTime full_xrange_to;

    ChartView *m_chart_view;

};


