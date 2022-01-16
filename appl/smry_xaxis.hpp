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


class SmryXaxis: public QtCharts::QDateTimeAxis {

public:

    SmryXaxis(QObject *parent = nullptr);

    void set_min_value(qreal number);
    bool set_range(std::string argstr);
    bool set_range(double min, double max);

    void setMinAndMax(QDateTime min_val, QDateTime max_val);
    void setMinAndMax(qreal min_val, qreal max_val);

    void resetAxisRange();
    void print_range();

    std::tuple<double, double> get_xrange();

    void reset_range();
    bool has_xrange() {return xrange_set;}

private slots:

    void rangeChanged(QDateTime min, QDateTime max);

private:

    QDateTime next_daylight_transition(const QDateTime& dt);
    QDateTime prev_daylight_transition(const QDateTime& dt);

    bool round_hour(qint64& ms1, qint64& ms2);
    bool round_day(qint64& ms1, qint64& ms2);
    bool round_month(qint64& ms1, qint64& ms2);
    bool round_year(qint64& ms1, qint64& ms2);
    void print_time_string(qint64 ms);
    void increment_month(int& y, int& m, int increment);
    void decrement_month(int& y, int& m, int decrement);

    void split_datetime(QDateTime dt, int& y, int& m, int& d, int& h, int& mi, int& s, int& ms);
    int adjust_ticks(int& n_tick, int max_intv);
    int adjust_ticks(int& n_tick, int min_intv, int max_intv);

    bool get_datetime_from_string(std::string str_arg, QDateTime& dt);

    bool xrange_set;

    qint64 m_min_attached_series;
    int n_tick;

    int rcount;

    QDateTime m_min;
    QDateTime m_max;

    QDateTime xrange_from;
    QDateTime xrange_to;
};


