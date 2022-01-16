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


#include <appl/smry_xaxis.hpp>

#include <random>
#include <iostream>
#include <iomanip>
#include <math.h>
#include <limits>
#include <string>


SmryXaxis::SmryXaxis(QObject *parent)
      : QDateTimeAxis(parent)
{
    m_min_attached_series = std::numeric_limits<qint64>::max();
    n_tick = 5;
    rcount = 0;
    xrange_set = false;

    connect(this, &QDateTimeAxis::rangeChanged, this, &SmryXaxis::rangeChanged);
}

bool SmryXaxis::get_datetime_from_string(std::string str_arg, QDateTime& dt)
{
    std::vector<int> tokens;

    std::string date_str;
    std::string time_str;

    int p = str_arg.find("_");

    if (p == -1){
        date_str = str_arg;
        time_str = "";
    } else {
        date_str = str_arg.substr(0, p);
        time_str = str_arg.substr(p+1);
    }

    // process date string

    int n = 0;
    p = str_arg.find("-");
    int p0 = 0;

    while ((p > -1) and (n < 10)){
        n++;

        try {
            tokens.push_back(std::stoi(date_str.substr(p0, p - p0)));
        }

        catch (...)
        {
            return false;
        }

        p0 = p + 1;
        p = date_str.find("-", p0);
    }

    tokens.push_back(std::stoi(date_str.substr(p0, p - p0)));

    QDate d;

    if (tokens.size() == 1)
        d.setDate(tokens[0], 1, 1);
    else if (tokens.size() == 2)
        d.setDate(tokens[0], tokens[1], 1);
    else if (tokens.size() == 3)
        d.setDate(tokens[0], tokens[1], tokens[2]);
    else
        return false;

    dt.setDate(d);

    // process time string if length > 0

    if (time_str.size() > 0){
        int p0 = 0;
        p = time_str.find(":", p0);
        int hr = std::stoi(time_str.substr(p0, p - p0));

        p0 = p + 1;
        p = time_str.find(":", p0);

        int min = std::stoi(time_str.substr(p0, p - p0));

        p0 = p + 1;
        p = time_str.find(":", p0);

        std::string sec_str =  time_str.substr(p0, p - p0);

        p = sec_str.find(".", 0);

        int sec = std::stoi(sec_str.substr(0, p));
        int msec = std::stoi(sec_str.substr(p+1));

        QTime t(hr, min, sec, msec);
        dt.setTime(t);
    }


    if (dt.isValid())
        return true;
    else
        return false;
}


bool SmryXaxis::set_range(double min, double max)
{
    QDateTime dt1;
    QDateTime dt2;

    dt1 = QDateTime::fromMSecsSinceEpoch(min);
    dt2 = QDateTime::fromMSecsSinceEpoch(max);

    this->setRange(dt1, dt2);

    xrange_from = dt1;
    xrange_to = dt2;


    return true;

}


bool SmryXaxis::set_range(std::string argstr)
{
    if (argstr.substr(0,7) == ":xrange")
        argstr = argstr.substr(8);

    int p1 = argstr.find(" ");

    if (p1 == -1)
        return false;

    std::string str_from = argstr.substr(0, p1);
    std::string str_to = argstr.substr(p1 + 1);

    QDateTime dt1;
    QDateTime dt2;

    if (not get_datetime_from_string(str_from, dt1))
        return false;

    if (not get_datetime_from_string(str_to, dt2))
        return false;

    if (dt1 > dt2)
        return false;

    xrange_from = dt1;
    xrange_to = dt2;

    this->setRange(xrange_from, xrange_to);
    xrange_set = true;

    return true;
}


void SmryXaxis::print_time_string(qint64 ms)
{
    QDateTime dt_test1 = QDateTime::fromMSecsSinceEpoch(ms);

    std::string test1 = dt_test1.toString("yyyy-MM-dd HH:mm:ss.zzz").toStdString();
    std::cout << "ms : " << ms << "  string: " << test1 << std::endl;

}

void SmryXaxis::increment_month(int& y, int& m, int increment)
{
    if (increment > 12)
        throw std::invalid_argument("increment_month not support increment > 12" );

    m = m + increment;

    if (m > 12) {
        m = m - 12;
        y += 1;
    }
}

void SmryXaxis::decrement_month(int& y, int& m, int decrement)
{
    if (decrement > 12)
        throw std::invalid_argument("decrement_month not support decrement > 12" );

    m = m - decrement;

    if (m < 1) {
        m = m + 12 - decrement;
        y -= 1;
    }
}


void SmryXaxis::set_min_value(qreal number)
{
    qint64 tmp = static_cast<qint64>(number);

    if (tmp < m_min_attached_series)
        m_min_attached_series = tmp;
};


void SmryXaxis::split_datetime(QDateTime dt, int& y, int& m, int& d, int& h, int& mi, int& s, int& ms)
{
    y = dt.date().year();
    m = dt.date().month();
    d = dt.date().day();

    h = dt.time().hour();
    mi = dt.time().minute();
    s = dt.time().second();
    ms = dt.time().msec();
}


int SmryXaxis::adjust_ticks(int& n_tick, int min_intv, int max_intv)
{
    int n = max_intv;

    while (n >= min_intv){
        if ((n_tick % n) == 0){
            n_tick = n;
            return 0;
        }

        n--;
    }


    auto intv = std::div(n_tick, max_intv);


    if (intv.rem > 0)
        intv.quot += 1;

    int step = intv.quot;

    intv = std::div(n_tick, step);

    n_tick = intv.quot;

    if (intv.rem > 0)
        n_tick++;

    return step - intv.rem;
}

int SmryXaxis::adjust_ticks(int& n_tick, int max_intv)
{
    auto intv = std::div(n_tick, max_intv);

    if (intv.rem > 0)
        intv.quot += 1;

    int step = intv.quot;

    intv = std::div(n_tick, step);

    n_tick = intv.quot;

    if (intv.rem > 0)
        n_tick++;

    return step - intv.rem;
}


bool SmryXaxis::round_year(qint64& ms1, qint64& ms2)
{
    QDateTime dt1 = QDateTime::fromMSecsSinceEpoch(ms1);
    QDateTime dt2 = QDateTime::fromMSecsSinceEpoch(ms2);

    int shift_d = 2;
    int min_intv = 6;
    int max_intv = 18;

    int y1, m1, d1, h1, mi1, s1, mis1;
    int y2, m2, d2, h2, mi2, s2, mis2;

    split_datetime(dt1, y1, m1, d1, h1, mi1, s1, mis1);
    split_datetime(dt2, y2, m2, d2, h2, mi2, s2, mis2);

    if ((m1==1) && (d1==1) && (mi1 == 0) && (s1 == 0))
        d1 = shift_d;

    if ((m2==1) && (d2==1) && (mi2 == 0) && (s2 == 0))
        d2 = shift_d;

    bool ticks_ok = ((y2-y1) % n_tick) > 0 ? false : true;

    qInfo() << "Round years, number of thicks  " << ticks_ok << ", count: " << rcount;

    if ((m1==1) && (d1 == shift_d) && (h1 == 0) && (mi1 == 0) && (s1 == 0) && (mis1 == 0) &&
          (m2==1) && (d2 == shift_d) && (h2 == 0) && (mi2 == 0) && (s2 == 0) && (mis2 == 0) && ticks_ok) {

        rcount = 0;

        return false;

    } else {

        rcount++;

        if (rcount > 10)
            qFatal("not able to round axis, number of attempts > 10, ");

        dt1 = QDateTime(QDate(y1,1,shift_d), QTime(0,0,0,0));

        if ((m1==1) &&  (d1 == 1) && (mi1 == 0) ){
            y1 -= 1;
            dt1 = QDateTime(QDate(y1,1,shift_d), QTime(0,0,0,0));
        }

        if ((m2 != 1) || (d2 != shift_d)  || (mi2 != 0))
            y2++;

        n_tick = y2 - y1;

        if (n_tick > max_intv) {
            y2 = y2 + adjust_ticks(n_tick, min_intv, max_intv);
        }

        dt2 = QDateTime(QDate(y2, 1, shift_d), QTime(0,0,0,0));

        ms1 = dt1.toMSecsSinceEpoch();
        ms2 = dt2.toMSecsSinceEpoch();

        return true;
    }
}


bool SmryXaxis::round_month(qint64& ms1, qint64& ms2)
{
    QDateTime dt1 = QDateTime::fromMSecsSinceEpoch(ms1);
    QDateTime dt2 = QDateTime::fromMSecsSinceEpoch(ms2);

    int shift_d = 3;

    int min_intv = 6;
    int max_intv = 11;

    int y1, m1, d1, h1, mi1, s1, mis1;
    int y2, m2, d2, h2, mi2, s2, mis2;

    split_datetime(dt1, y1, m1, d1, h1, mi1, s1, mis1);
    split_datetime(dt2, y2, m2, d2, h2, mi2, s2, mis2);

    if ((m1==1) && (d1==1) && (mi1 == 0) && (s1 == 0) && (mis1 == 0))
        d1 = shift_d;

    if ((m2==1) && (d2==1) && (mi2 == 0) && (s2 == 0) && (mis2 == 0))
        d2 = shift_d;

    qInfo() << "Round month, count: " << rcount;

    if ((d1 == shift_d) && (h1 == 0) && (mi1 == 0) && (s1 == 0) && (mis1 == 0) &&
          (d2 == shift_d) && (h2 == 0) && (mi2 == 0) && (s2 == 0) && (mis2 == 0)) {

        rcount = 0;

        return false;

    } else {

        rcount++;

        if (rcount > 10)
            qFatal("not able to round axis, number of attempts > 10, ");

        dt1 = QDateTime(QDate(y1, m1, shift_d), QTime(0,0,0,0));

        if (dt1.toMSecsSinceEpoch() > m_min_attached_series){
            decrement_month(y1, m1, 1);
            dt1 = QDateTime(QDate(y1, m1, shift_d), QTime(0,0,0,0));
        }

        increment_month(y2, m2, 1);

        n_tick = (y2 - y1) * 12 + m2 - m1;

        if (n_tick > max_intv) {
            int add_month = adjust_ticks(n_tick, min_intv, max_intv);
            increment_month(y2, m2, add_month);
        }

        dt2 = QDateTime(QDate(y2, m2, shift_d), QTime(0,0,0,0));

        ms1 = dt1.toMSecsSinceEpoch();
        ms2 = dt2.toMSecsSinceEpoch();

        return true;
    }
}


bool SmryXaxis::round_day(qint64& ms1, qint64& ms2)
{
    QDateTime dt1 = QDateTime::fromMSecsSinceEpoch(ms1);
    QDateTime dt2 = QDateTime::fromMSecsSinceEpoch(ms2);

    int min_intv = 5;
    int max_intv = 8;

    int y1, m1, d1, h1, mi1, s1, mis1;
    int y2, m2, d2, h2, mi2, s2, mis2;

    split_datetime(dt1, y1, m1, d1, h1, mi1, s1, mis1);
    split_datetime(dt2, y2, m2, d2, h2, mi2, s2, mis2);

    int h1_rounded = dt1.isDaylightTime() ? 1 : 0;
    int h2_rounded = dt2.isDaylightTime() ? 1 : 0;

    bool ticks_ok = (((ms2-ms1)/(3600000*24)) % n_tick) > 0 ? false : true;

    qInfo() << "Round day: number of thicks  " << ticks_ok << ", count: " << rcount;

    if ( (h1 == h1_rounded) && (mi1 == 0) && (s1 == 0) &&
          (h2 == h2_rounded) && (mi2 == 0) && (s2 == 0) && ticks_ok ) {

        rcount = 0;

        return false;

    } else {

        rcount++;

        if (rcount > 10)
            qFatal("not able to round axis, number of attempts > 10, ");

        dt1 = QDateTime(QDate(y1, m1, d1), QTime(0,0,0,0));

        if (dt1.isDaylightTime())
            dt1 = QDateTime(QDate(y1, m1, d1), QTime(1,0,0,0));

        dt2 = QDateTime(QDate(y2, m2, d2), QTime(0,0,0,0));

        if ((h2 != h2_rounded) || (mi2 > 0) || (s2 > 0)) {
            dt2 = dt2.addDays(1.0);
            split_datetime(dt2, y2, m2, d2, h2, mi2, s2, mis2);
        }

        if (dt2.isDaylightTime())
            dt2 = QDateTime(QDate(y2, m2, d2), QTime(1,0,0,0));

        ms1 = dt1.toMSecsSinceEpoch();
        ms2 = dt2.toMSecsSinceEpoch();

        float f_days = static_cast<float>(ms2 - ms1);
        f_days = f_days / (1000.0 * 3600.0 * 24.0);

        auto s1 = dt1.toSecsSinceEpoch();
        auto s2 = dt2.toSecsSinceEpoch();

        auto ndays = (s2 - s1) / (3600*24);

        n_tick = static_cast<int>(ndays);

        if (n_tick > max_intv) {
            int add_days = adjust_ticks(n_tick, min_intv, max_intv);
            ms2 += add_days*24*60*60*1000;
        }

        return true;
    }
}


QDateTime SmryXaxis::next_daylight_transition(const QDateTime& dt)
{
    QTimeZone tzone = dt.timeZone();

    QTimeZone::OffsetData  tz_next_offset_data = tzone.nextTransition(dt);

    QDateTime next_trans_dt = tz_next_offset_data.atUtc;
    next_trans_dt = next_trans_dt.addSecs(tz_next_offset_data.offsetFromUtc);

    return next_trans_dt;
}


QDateTime SmryXaxis::prev_daylight_transition(const QDateTime& dt)
{
    QTimeZone tzone = dt.timeZone();

    QTimeZone::OffsetData  tz_prev_offset_data = tzone.previousTransition(dt);

    QDateTime prev_trans_dt = tz_prev_offset_data.atUtc;
    prev_trans_dt = prev_trans_dt.addSecs(tz_prev_offset_data.offsetFromUtc);

    return prev_trans_dt;
}


bool SmryXaxis::round_hour(qint64& ms1, qint64& ms2)
{
    QDateTime dt1 = QDateTime::fromMSecsSinceEpoch(ms1);
    QDateTime dt2 = QDateTime::fromMSecsSinceEpoch(ms2);

    int min_intv = 5;
    int max_intv = 8;

    int y1, m1, d1, h1, mi1, s1, mis1;
    int y2, m2, d2, h2, mi2, s2, mis2;

    split_datetime(dt1, y1, m1, d1, h1, mi1, s1, mis1);
    split_datetime(dt2, y2, m2, d2, h2, mi2, s2, mis2);

    bool ticks_ok = (((ms2-ms1)/3600000) % n_tick) > 0 ? false : true;

    qInfo() << "Round hours, number of thicks  " << ticks_ok << ", count: " << rcount;

    if ( (mi1 == 0) && (s1 == 0)  &&
          (mi2 == 0) && (s2 == 0)  && (ticks_ok) ) {

        rcount = 0;

        return false;

    } else {

        rcount++;

        if (rcount > 10)
            qFatal("not able to round axis, number of attempts > 10, ");

        dt1 = QDateTime(QDate(y1, m1, d1), QTime(h1,0,0,0));

        bool add_day = false;

        if (mi2 > 0){
            h2++;

            if (h2 == 24){
                h2 = 0;
                add_day = true;
            }
        }

        dt2 = QDateTime(QDate(y2, m2, d2), QTime(h2,0,0,0));

        if (add_day)
            dt2 = dt2.addDays(1);

        ms1 = dt1.toMSecsSinceEpoch();
        ms2 = dt2.toMSecsSinceEpoch();

        qint64 s1 = round(ms1 / 1000.0);
        qint64 s2 = round(ms2 / 1000.0);

        qint64 nhours = round(s2-s1) / 3600.0;

        n_tick = static_cast<int>(nhours);

        if (n_tick > max_intv) {
            int add_hrs = adjust_ticks(n_tick, min_intv, max_intv);
            ms2 += add_hrs*3600000;
            s2 += add_hrs*3600;
        }

        nhours = round((s2 - s1) / 3600);

        return true;
    }
}


void SmryXaxis::rangeChanged(QDateTime min, QDateTime max)
{
    qint64 ms1 = min.toMSecsSinceEpoch();
    qint64 ms2 = max.toMSecsSinceEpoch();

    QDateTime dt1 = QDateTime::fromMSecsSinceEpoch(ms1);
    QDateTime dt2 = QDateTime::fromMSecsSinceEpoch(ms2);

    QString dt1_qstr = dt1.toString("yyyy-MM-dd HH:mm:ss.zzz");
    QString dt2_qstr = dt2.toString("yyyy-MM-dd HH:mm:ss.zzz");

    qInfo() << "xaxis range changed, min: " << dt1_qstr << ", " <<  dt2_qstr;

    double diff_days;

    if (ms2 == ms1)
        diff_days = 1000.0;
    else
        diff_days = static_cast<double>(ms2 - ms1) / static_cast<double>(1000*60*60*24);

    if (diff_days > 2000) {

        if (round_year(ms1, ms2)) {

            this->setRange(QDateTime::fromMSecsSinceEpoch(ms1),
                             QDateTime::fromMSecsSinceEpoch(ms2));

            this->setTickCount(n_tick + 1);

        } else {

            this->setTickCount(n_tick + 1);
            this->setFormat("yyyy");
            this->setTitleText("Date");

            qInfo() << "round years, complete all good ?";
        }

    } else if (diff_days > 60) {

        if (round_month(ms1, ms2)) {

            this->setRange(QDateTime::fromMSecsSinceEpoch(ms1),
                             QDateTime::fromMSecsSinceEpoch(ms2));

            this->setTickCount(n_tick + 1);

        } else {

            this->setTickCount(n_tick + 1);

            this->setFormat("MMM yyyy");
            this->setTitleText("Date");

            qInfo() << "round months, complete all good ?";
        }

    } else if (diff_days > 3) {


        if (round_day(ms1, ms2)) {

            try {

                this->setRange(QDateTime::fromMSecsSinceEpoch(ms1),
                             QDateTime::fromMSecsSinceEpoch(ms2));

                this->setTickCount(n_tick + 1);
            }

            catch (const char* message) {
                std::cout << "in round days, catch exception ";
                std::cout << message << std::endl;
            }


        } else {

            this->setFormat("dd MMM yyyy");
            this->setTitleText("Date");

            qInfo() << "round days, complete all good ?";
        }

    } else if (diff_days > 0.125) {

        if (round_hour(ms1, ms2)) {

            this->setRange(QDateTime::fromMSecsSinceEpoch(ms1),
                             QDateTime::fromMSecsSinceEpoch(ms2));

            this->setTickCount(n_tick + 1);

        } else {

            this->setFormat("dd MMM yyyy hh:mm");
            this->setTitleText("Date");

            qInfo() << "round hours, complete all good ?";
        }

    } else {

        qInfo() << "Default, no rounding";

        this->setTickCount(5);
        this->setFormat("dd MMM yyyy hh:mm");
        this->setTitleText("Date");
    }
}

void SmryXaxis::setMinAndMax(QDateTime min_val, QDateTime max_val)
{
    m_min = min_val;
    m_max = max_val;
}

void SmryXaxis::setMinAndMax(qreal min_val, qreal max_val)
{
    m_min = QDateTime::fromMSecsSinceEpoch(min_val);
    m_max = QDateTime::fromMSecsSinceEpoch(max_val);
}

void SmryXaxis::resetAxisRange()
{
    this->setRange(m_min, m_max);
}

void SmryXaxis::print_range()
{
    std::cout << "\nfull range: " << std::endl;
    std::cout << "min: " << m_min.toMSecsSinceEpoch();
    std::cout << "   max: " << m_max.toMSecsSinceEpoch();
    std::cout << std::endl;

    std::cout << "\nxrange: " << std::endl;

    std::cout << "min: " << xrange_from.toMSecsSinceEpoch();
    std::cout << "   max: " << xrange_to.toMSecsSinceEpoch();
    std::cout << std::endl;

}

void SmryXaxis::reset_range()
{
    xrange_from = m_min;
    xrange_to = m_max;

    xrange_set = false;

}

std::tuple<double, double> SmryXaxis::get_xrange()
{
    return std::make_tuple(static_cast<double>(xrange_from.toMSecsSinceEpoch()),
                           static_cast<double>(xrange_to.toMSecsSinceEpoch()));
}

