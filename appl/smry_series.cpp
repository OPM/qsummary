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


#include <appl/smry_series.hpp>

#include <random>
#include <iostream>
#include <iomanip>
#include <math.h>


SmrySeries::SmrySeries(QChart *qtchart, QObject *parent)
      : QLineSeries(parent),
        m_chart(qtchart),
        m_tooltip(nullptr)

{
    connect(this, &QXYSeries::hovered, this, &SmrySeries::onHovered);
    connect(this, &QXYSeries::pressed, this, &SmrySeries::onPressed);

    m_glob_min = std::numeric_limits<double>::max();
    m_glob_max = -1.0*std::numeric_limits<double>::max();
}

void  SmrySeries::onPressed(const QPointF &point)
{

    QPointF p_closest = calculate_closest(point);

    QDateTime dt = QDateTime::fromMSecsSinceEpoch(p_closest.x());

    qreal yval = p_closest.y();

    QString dt_qstr = dt.toString("yyyy-MM-dd HH:mm:ss");

    std::cout << this->objectName().toStdString() << "  ";
    std::cout <<  dt_qstr.toStdString() << "  " << yval;
    std::cout << std::endl;


 }

void SmrySeries::onHovered(const QPointF &point, bool state)
{

    if (m_tooltip == 0)
        m_tooltip = new PointInfo(m_chart, this);

    bool use_bottom_right = false;

    if (state) {

        auto map_p = m_chart->mapToPosition(point, this);

        QRectF rec = m_chart->rect();

        if ((map_p.x() / rec.right()) > 0.75)
            use_bottom_right = true;

        QPointF p_closest = calculate_closest(point);

        QDateTime dt = QDateTime::fromMSecsSinceEpoch(p_closest.x());

        int ms = dt.time().msec();

        double ms_frac = static_cast<double>(ms) / 1000.0;
        double ms_correction = round(ms_frac);

        dt = dt.addMSecs(ms_correction);

        QString qstr = dt.toString("yyyy-MM-dd HH:mm:ss"); // = dt.toString("yyyy-MM-dd HH:mm:ss");

        if (dt.isDaylightTime())
            qstr = qstr + " ( daylight time ) ";

        qreal yval = p_closest.y();

        qstr = qstr + "\n" + this->objectName();
        qstr = qstr + " = " + QString::number(yval);

        m_tooltip->set_right_below(use_bottom_right);

        m_tooltip->setText(qstr);
        m_tooltip->setAnchor(p_closest);
        m_tooltip->setZValue(11);
        m_tooltip->updateGeometry();
        m_tooltip->show();


    } else {
        m_tooltip->hide();
    }


}


QPointF SmrySeries::calculate_closest(const QPointF point)
{
    auto vect = this->pointsVector();

    qreal ref_x = vect[0].x();
    qreal ref_y = vect[0].y();

    qreal dist = pow((ref_x - point.x()), 2.0) + pow((ref_y - point.y()), 2.0) ;
    dist = pow(dist, 0.5);

    for (size_t n = 1; n < vect.size(); n++) {

        qreal dist_test = pow((vect[n].x() - point.x()), 2.0) + pow((vect[n].y() - point.y()), 2.0) ;
        dist_test = pow(dist_test, 0.5);

        if (dist_test < dist) {
            dist = dist_test;
            ref_x = vect[n].x();
            ref_y = vect[n].y();
        }
    }

    QPointF new_point;

    new_point.setX(ref_x);
    new_point.setY(ref_y);

    return new_point;
}

void SmrySeries::print_data()
{
    auto data = this->pointsVector();

    for (size_t n = 0; n < data.size(); n++){
        std::cout << std::fixed << std::setw(15) << std::setprecision(0) << data[n].x();
        std::cout << "  " << std::scientific << std::setw(15) << std::setprecision(5) << data[n].y();
        std::cout << std::endl;
    }

    std::cout << "\nsize: " << data.size() << "\n\n";
}

std::tuple<double,double> SmrySeries::get_min_max_value(double xfrom, double xto)
{
    double min_y = std::numeric_limits<double>::max();
    double max_y = std::numeric_limits<double>::min();

    auto data = this->pointsVector();

    for (size_t n = 0; n < data.size(); n++){
        if ((static_cast<double>(data[n].x()) > xfrom) && (static_cast<double>(data[n].x()) < xto)){
            if ( static_cast<double>(data[n].y()) < min_y)
                min_y = static_cast<double>(data[n].y());

            if ( data[n].y() > max_y)
                max_y = static_cast<double>(data[n].y());
        }
    }

    if (abs(min_y) < 1e-100)
        min_y =0.0;

    if (abs(max_y) < 1e-100)
        max_y =0.0;

    return std::make_tuple(min_y, max_y);
}

void SmrySeries::calcMinAndMax(){

    auto data = this->pointsVector();

    for (size_t n = 0; n < data.size(); n++) {

        if ( static_cast<double>(data[n].y()) < m_glob_min)
            m_glob_min = static_cast<double>(data[n].y());

        if ( static_cast<double>(data[n].y()) > m_glob_max)
            m_glob_max = static_cast<double>(data[n].y());
    }
}

bool SmrySeries::all_values_zero()
{
    auto data = this->pointsVector();

    for (size_t n = 0; n < data.size(); n++)
        if (data[n].y() != 0.0)
            return false;

    return true;
}

