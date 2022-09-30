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


SmryXaxis::SmryXaxis(ChartView *chart_view, QObject *parent)
      : QDateTimeAxis(parent),
      m_chart_view(chart_view)
{
    this->setVisible(false);
    m_chart_view->show_xaxis_obj();

    m_dt_min_utc.setTimeSpec(Qt::UTC);
    m_dt_max_utc.setTimeSpec(Qt::UTC);

    QDateTime xrange_from;
    xrange_from.setTimeSpec(Qt::UTC);

    QDateTime xrange_to;
    xrange_to.setTimeSpec(Qt::UTC);

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
    dt1.setTimeSpec(Qt::UTC);

    QDateTime dt2;
    dt2.setTimeSpec(Qt::UTC);

    if (not get_datetime_from_string(str_from, dt1))
        return false;

    if (not get_datetime_from_string(str_to, dt2))
        return false;

    if (dt1 > dt2)
        return false;

    xrange_from = dt1;
    xrange_to = dt2;

    this->setRange(dt1, dt2);
    xrange_set = true;

    return true;
}


void SmryXaxis::print_time_string(qint64 ms)
{
    QDateTime dt_test1 = QDateTime::fromMSecsSinceEpoch(ms);

    std::string test1 = dt_test1.toString("yyyy-MM-dd HH:mm:ss.zzz").toStdString();
    std::cout << "ms : " << ms << "  string: " << test1 << std::endl;

}


std::string SmryXaxis::get_month_string(int ind)
{
    std::vector<std::string> month_list;
    month_list = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    if ((ind < 0) || (ind > 11))
        throw std::runtime_error("month index " + std::to_string(ind) + " out of range" );

    return month_list[ind];
}


SmryXaxis::tick_type SmryXaxis::make_raw_list(int nThick)
{
    SmryXaxis::tick_type tick_list;
    double ms1 = static_cast<double>(m_dt_min_utc.toMSecsSinceEpoch());
    double ms2 = static_cast<double>(m_dt_max_utc.toMSecsSinceEpoch());

    double diff_msecs = ms2 - ms1;

    double step_msecs = diff_msecs / nThick;

    int y1 = m_dt_min_utc.date().year();
    int m1 = m_dt_min_utc.date().month();
    int d1 = m_dt_min_utc.date().day();
    int h1 = m_dt_min_utc.time().hour();
    int mm1 = m_dt_min_utc.time().minute();
    int s1 = m_dt_min_utc.time().second();
    int msec1 = m_dt_min_utc.time().msec();

    QDate d;
    d.setDate(y1, m1, d1);

    QDateTime dt;

    dt.setDate(d);

    dt.setTime({ h1, mm1, s1, msec1 });
    dt.setTimeSpec(Qt::UTC);
    dt = dt.addMSecs(step_msecs);

    int n = 0;

    while (dt < m_dt_max_utc){

        double frac = (static_cast<double>(dt.toMSecsSinceEpoch()) - ms1) / ( ms2 - ms1);
        int ms = dt.time().msec();
        int s = dt.time().second();
        int mi = dt.time().minute();
        int h = dt.time().hour();
        int d = dt.date().day();
        int m = dt.date().month() - 1;
        int y = dt.date().year();

        std::ostringstream str;
        str << std::setw(2) << std::setfill('0') << d << " " << get_month_string(m) << " " << y;
        str << " " << std::setw(2) << std::setfill('0') << h << ":";
        str << std::setw(2) << std::setfill('0') << mi << ":";
        str << std::setw(2) << std::setfill('0') << s;
        str << "." << std::setw(3) << std::setfill('0') << ms;

        std::string lbl_str = str.str();

        tick_list.push_back( {lbl_str, frac});
        dt = dt.addMSecs(step_msecs);

        n++;
        if (n > 100){
            std::cout << "n > 100 , ups \n";
            std::cout << "step_msecs: " << step_msecs << "\n\n";
            exit(1);
        }
    }

    return tick_list;
}

SmryXaxis::tick_type SmryXaxis::make_second_list()
{
    SmryXaxis::tick_type tick_list;

    double ms1 = static_cast<double>(m_dt_min_utc.toMSecsSinceEpoch());
    double ms2 = static_cast<double>(m_dt_max_utc.toMSecsSinceEpoch());

    double diff_hrs = (ms2 - ms1) / (1000*3600);
    double diff_second = (ms2 - ms1) / 1000;

    int y1 = m_dt_min_utc.date().year();
    int m1 = m_dt_min_utc.date().month();
    int d1 = m_dt_min_utc.date().day();
    int h1 = m_dt_min_utc.time().hour();
    int mi1 = m_dt_min_utc.time().minute();
    int s1 = m_dt_min_utc.time().second();

    int step_seconds;

    if (diff_second > 90){
        step_seconds = 30;
        s1 = std::floor(s1/30)*30;
    } else if (diff_second > 45){
        step_seconds = 15;
        s1 = std::floor(s1/15)*15;
    } else if (diff_second > 30){
        step_seconds = 10;
        s1 = std::floor(s1/10)*10;
    } else if (diff_second > 15){
        step_seconds = 5;
        s1 = std::floor(s1/5)*5;
    } else if (diff_second > 2){
        step_seconds = 1;
        s1 = std::floor(s1/1)*1;
    } else {
        std::cout << " less that 2 seconds in make second list, should not be here\n";
        exit(1);
    }

    QDate d;
    d.setDate(y1, m1, d1);

    QDateTime dt;

    dt.setDate(d);

    dt.setTime({ h1, mi1, s1, 0 });
    dt.setTimeSpec(Qt::UTC);
    dt = dt.addSecs(step_seconds);

    while (dt < m_dt_max_utc){

        double frac = (static_cast<double>(dt.toMSecsSinceEpoch()) - ms1) / ( ms2 - ms1);
        int s = dt.time().second();
        int mi = dt.time().minute();
        int h = dt.time().hour();
        int d = dt.date().day();
        int m = dt.date().month() - 1;
        int y = dt.date().year();

        std::ostringstream str;
        str << std::setw(2) << std::setfill('0') << d << " " << get_month_string(m) << " " << y;
        str << " " << std::setw(2) << std::setfill('0') << h << ":";
        str << std::setw(2) << std::setfill('0') << mi << ":";
        str << std::setw(2) << std::setfill('0') << s;

        std::string lbl_str = str.str();

        tick_list.push_back( {lbl_str, frac});
        dt = dt.addSecs(step_seconds);
    }

    return tick_list;
}


SmryXaxis::tick_type SmryXaxis::make_minute_list()
{
    SmryXaxis::tick_type tick_list;

    double ms1 = static_cast<double>(m_dt_min_utc.toMSecsSinceEpoch());
    double ms2 = static_cast<double>(m_dt_max_utc.toMSecsSinceEpoch());

    double diff_hrs = (ms2 - ms1) / (1000*3600);
    double diff_minute = (ms2 - ms1) / (1000*60);

    int y1 = m_dt_min_utc.date().year();
    int m1 = m_dt_min_utc.date().month();
    int d1 = m_dt_min_utc.date().day();
    int h1 = m_dt_min_utc.time().hour();
    int mi1 = m_dt_min_utc.time().minute();

    int step_minutes;

    if (diff_minute > 90){
        step_minutes = 30;
        mi1 = std::floor(mi1/30)*30;
    } else if (diff_minute > 45){
        step_minutes = 15;
        mi1 = std::floor(mi1/15)*15;
    } else if (diff_minute > 30){
        step_minutes = 10;
        mi1 = std::floor(mi1/10)*10;
    } else if (diff_minute > 15){
        step_minutes = 5;
        mi1 = std::floor(mi1/5)*5;
    } else if (diff_minute > 2){
        step_minutes = 1;
        mi1 = std::floor(mi1/1)*1;
    } else {
        std::cout << " less than 2 minutes in make minute list, should not be here.\n";
        exit(1);
    }

    QDate d;
    d.setDate(y1, m1, d1);

    QDateTime dt;

    dt.setDate(d);

    dt.setTime({ h1, mi1, 0, 0 });
    dt.setTimeSpec(Qt::UTC);
    dt = dt.addSecs(step_minutes*60);

    while (dt < m_dt_max_utc){

        double frac = (static_cast<double>(dt.toMSecsSinceEpoch()) - ms1) / ( ms2 - ms1);
        int mi = dt.time().minute();
        int h = dt.time().hour();
        int d = dt.date().day();
        int m = dt.date().month() - 1;
        int y = dt.date().year();

        std::ostringstream str;
        str << std::setw(2) << std::setfill('0') << d << " " << get_month_string(m) << " " << y;
        str << " " << std::setw(2) << std::setfill('0') << h << ":";
        str << std::setw(2) << std::setfill('0') << mi;

        std::string lbl_str = str.str();

        tick_list.push_back( {lbl_str, frac});
        dt = dt.addSecs(step_minutes*60);
    }

    return tick_list;
}

SmryXaxis::tick_type SmryXaxis::make_hr_list()
{
    SmryXaxis::tick_type tick_list;

    double ms1 = static_cast<double>(m_dt_min_utc.toMSecsSinceEpoch());
    double ms2 = static_cast<double>(m_dt_max_utc.toMSecsSinceEpoch());

    double diff_hrs = (ms2 - ms1) / (1000*3600);

    int y1 = m_dt_min_utc.date().year();
    int m1 = m_dt_min_utc.date().month();
    int d1 = m_dt_min_utc.date().day();
    int h1 = m_dt_min_utc.time().hour();

    QDate d;
    d.setDate(y1, m1, d1);

    QDateTime dt;

    dt.setDate(d);

    dt.setTime({ h1, 0, 0, 0 });
    dt.setTimeSpec(Qt::UTC);
    dt = dt.addSecs(3600);

    while (dt < m_dt_max_utc){

        double frac = (static_cast<double>(dt.toMSecsSinceEpoch()) - ms1) / ( ms2 - ms1);
        int h = dt.time().hour();
        int d = dt.date().day();
        int m = dt.date().month() - 1;
        int y = dt.date().year();

        std::ostringstream str;
        str << std::setw(2) << std::setfill('0') << d << " " << get_month_string(m) << " " << y;
        str << " " << std::setw(2) << std::setfill('0') << h << ":00";

        std::string lbl_str = str.str();

        tick_list.push_back( {lbl_str, frac});
        dt = dt.addSecs(3600);
    }

    return tick_list;
}


SmryXaxis::tick_type SmryXaxis::make_day_list()
{
    SmryXaxis::tick_type tick_list;

    double ms1 = static_cast<double>(m_dt_min_utc.toMSecsSinceEpoch());
    double ms2 = static_cast<double>(m_dt_max_utc.toMSecsSinceEpoch());

    int y1 = m_dt_min_utc.date().year();
    int m1 = m_dt_min_utc.date().month();
    int d1 = m_dt_min_utc.date().day();

    QDate d;
    d.setDate(y1, m1, d1);

    QDateTime dt;

    dt.setDate(d);

    dt.setTime({ 0, 0, 0, 0 });
    dt.setTimeSpec(Qt::UTC);
    dt = dt.addDays(1);

    while (dt < m_dt_max_utc){

        double frac = (static_cast<double>(dt.toMSecsSinceEpoch()) - ms1) / ( ms2 - ms1);
        int d = dt.date().day();
        int m = dt.date().month() - 1;
        int y = dt.date().year();

        std::ostringstream str;
        str << std::setw(2) << std::setfill('0') << d << " " << get_month_string(m) << " " << y;

        std::string lbl_str = str.str();

        tick_list.push_back( {lbl_str, frac});
        dt = dt.addDays(1);
    }

    return tick_list;
}


SmryXaxis::tick_type SmryXaxis::make_month_list()
{
    SmryXaxis::tick_type tick_list;

    double ms1 = static_cast<double>(m_dt_min_utc.toMSecsSinceEpoch());
    double ms2 = static_cast<double>(m_dt_max_utc.toMSecsSinceEpoch());

    int y1 = m_dt_min_utc.date().year();
    int m1 = m_dt_min_utc.date().month();

    QDate d;

    if (m1 == 12)
       d.setDate(y1+1, 1, 1);
    else
       d.setDate(y1, m1 + 1, 1);

    QDateTime dt;

    dt.setDate(d);
    dt.setTime({ 0, 0, 0, 0 });
    dt.setTimeSpec(Qt::UTC);

    while (dt < m_dt_max_utc){

        double frac = (static_cast<double>(dt.toMSecsSinceEpoch()) - ms1) / ( ms2 - ms1);
        int m = dt.date().month() - 1;
        int y = dt.date().year();
        std::string lbl_str = get_month_string(m) + " " + std::to_string(y);
        tick_list.push_back( {lbl_str, frac});
        dt = dt.addMonths(1);
    }

    return tick_list;
}

SmryXaxis::tick_type SmryXaxis::make_year_list()
{
    SmryXaxis::tick_type tick_list;

    double ms1 = static_cast<double>(m_dt_min_utc.toMSecsSinceEpoch());
    double ms2 = static_cast<double>(m_dt_max_utc.toMSecsSinceEpoch());

    int y1 = m_dt_min_utc.date().year();

    QDate d;
    d.setDate(y1+1, 1, 1);

    QDateTime dt;

    dt.setDate(d);
    dt.setTime({ 0, 0, 0, 0 });
    dt.setTimeSpec(Qt::UTC);

    while (dt < m_dt_max_utc){

        double frac = (static_cast<double>(dt.toMSecsSinceEpoch()) - ms1) / ( ms2 - ms1);
        int y = dt.date().year();
        std::string lbl_str = std::to_string(y);
        tick_list.push_back( {lbl_str, frac});
        dt = dt.addYears(1);
    }

    return tick_list;
}

void SmryXaxis::rangeChanged(QDateTime min, QDateTime max)
{
    qint64 ms_min = min.toMSecsSinceEpoch();
    qint64 ms_max = max.toMSecsSinceEpoch();

    m_dt_min_utc.setDate({1970, 1, 1});
    m_dt_min_utc.setTime({0, 0, 0});
    m_dt_min_utc = m_dt_min_utc.addMSecs(static_cast<qint64>(ms_min));

    m_dt_max_utc.setDate({1970, 1, 1});
    m_dt_max_utc.setTime({0, 0, 0});
    m_dt_max_utc = m_dt_max_utc.addMSecs(static_cast<qint64>(ms_max));

    if ((ms_max - ms_min) < 10){
        std::cout << "\n!Warning, trying to set xaxis range to less than 10 millisecond";
        std::cout << ", which is not allowed\n";

        m_dt_max_utc = m_dt_max_utc.addMSecs(10);

        QString dt_min_qstr = m_dt_min_utc.toString("yyyy-MM-dd HH:mm:ss.zzz");
        QString dt_max_qstr = m_dt_max_utc.toString("yyyy-MM-dd HH:mm:ss.zzz");

        std::cout << "Re-setting to :" << dt_min_qstr.toStdString();
        std::cout << " -> " << dt_max_qstr.toStdString() << "\n\n";

        this->setRange(m_dt_min_utc, m_dt_max_utc);
    }

    double diff = static_cast<double>(ms_max - ms_min);
    diff = diff / (1000*3600*24);


    QString dt1_qstr = m_dt_min_utc.toString("yyyy-MM-dd HH:mm:ss.zzz");
    QString dt2_qstr = m_dt_max_utc.toString("yyyy-MM-dd HH:mm:ss.zzz");

    double  diff_days = static_cast<double>(ms_max - ms_min) / static_cast<double>(1000*60*60*24);

    std::vector<std::tuple<std::string, double>> raw_ticks_vect;

    int max_ticks;

    if (diff_days > 2000) {
        raw_ticks_vect = make_year_list();
        max_ticks = 20;
    } else if (diff_days > 60) {
        raw_ticks_vect = make_month_list();
        max_ticks = 10;
    } else if (diff_days > 3) {
        raw_ticks_vect = make_day_list();
        max_ticks = 8;
    } else if (diff_days > 0.1) {
        raw_ticks_vect = make_hr_list();
        max_ticks = 8;
    } else if (diff_days > 0.0015) {
        raw_ticks_vect = make_minute_list();
        max_ticks = 6;

    } else if (diff_days > 0.000035) {
        raw_ticks_vect = make_second_list();
        max_ticks = 6;
    } else {
        max_ticks = 4;
        raw_ticks_vect = make_raw_list(max_ticks);
    }

    std::vector<std::tuple<std::string, double>> ticks_vect;

    if (raw_ticks_vect.size() >  max_ticks){
        int num_ticks = static_cast<int>(raw_ticks_vect.size());

        int step = static_cast<int>(num_ticks / max_ticks);
        int rest = num_ticks % max_ticks;

        if (rest > 0)
            step++;

        int i = 0;

        while (i < num_ticks){
            ticks_vect.push_back(raw_ticks_vect[i]);
            i = i + step;
        }

    } else {

        ticks_vect = raw_ticks_vect;
    }

    m_chart_view->set_xaxis_ticks(ticks_vect);
    m_chart_view->update_geometry();
}


std::tuple<double, double> SmryXaxis::get_xrange(bool full_range)
{
    if ((!xrange_set) || (full_range))
        return std::make_tuple(static_cast<double>(m_dt_min_utc.toMSecsSinceEpoch()),
                               static_cast<double>(m_dt_max_utc.toMSecsSinceEpoch()));
    else
        return std::make_tuple(static_cast<double>(xrange_from.toMSecsSinceEpoch()),
                               static_cast<double>(xrange_to.toMSecsSinceEpoch()));


}

void SmryXaxis::resetAxisRange()
{
    this->setRange(m_dt_min_utc, m_dt_max_utc);
}

void SmryXaxis::reset_range()
{
    xrange_from = m_dt_min_utc;
    xrange_to = m_dt_max_utc;

    xrange_set = false;

}
