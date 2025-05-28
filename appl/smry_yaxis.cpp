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

#include <appl/smry_yaxis.hpp>

#include <random>
#include <iostream>
#include <math.h>
#include <limits>


SmryYaxis::SmryYaxis(AxisMultiplierType mult_type, float mult, QObject *parent)
      : QValueAxis(parent),
        axis_multiplier(mult),
        axis_multiplier_type(mult_type)
{
//    connect(this, &QValueAxis::rangeChanged, this, &SmryYaxis::rangeChanged);
}

/*
void SmryYaxis::rangeChanged(qreal min, qreal max)
{
    //std::cout << "here we go, yaxis changed " << std::endl;
}
*/

void SmryYaxis::update_series_data(AxisMultiplierType mult_type, float mult, const std::vector<SmrySeries*>& series)
{
    for (size_t n = 0; n < series.size(); n++){
        if (series[n]->attachedAxes()[1] == this){

            auto vect = series[n]->points();
            auto new_mult = mult / axis_multiplier;

            for (size_t i = 0; i < vect.size(); i++)
                series[n]->replace(i, vect[i].x(), vect[i].y() * new_mult );
        }
    }

    axis_multiplier = mult;
    axis_multiplier_type = mult_type;
}


void SmryYaxis::update_axis_multiplier(const std::vector<SmrySeries*>& series)
{
    std::vector<std::vector<float>> yvalues;

    for (size_t n = 0; n < series.size(); n++){

        yvalues.push_back({});

        if (series[n]->attachedAxes()[1] == this){
            auto vect = series[n]->points();

            for (size_t i = 0; i < vect.size(); i++)
                yvalues[n].push_back(static_cast<float>(vect[i].y()/multiplier()));
        }
    }

    AxisMultiplierType updated_axis_multiplier_type = AxisMultiplierType::one;

    for (size_t n = 0; n < series.size(); n++){

        if (series[n]->attachedAxes()[1] == this) {

            float p90v = calc_p90(yvalues[n]);

            if (p90v > 1.0e9)
                updated_axis_multiplier_type = AxisMultiplierType::billion;
            else if (p90v > 1.0e6)
                updated_axis_multiplier_type = AxisMultiplierType::million;
            else if (p90v > 1.0e3)
                updated_axis_multiplier_type = AxisMultiplierType::thousand;
        }
    }

    if (updated_axis_multiplier_type != axis_multiplier_type){

        float updated_multiplier = 1.0;

        switch (updated_axis_multiplier_type) {
        case AxisMultiplierType::billion:
            updated_multiplier = 1.0e-9;
            break;
        case AxisMultiplierType::million:
            updated_multiplier = 1.0e-6;
            break;
        case AxisMultiplierType::thousand:
            updated_multiplier = 1.0e-3;
            break;
        }

        for (size_t n = 0; n < series.size(); n++){
            if (series[n]->attachedAxes()[1] == this){
                auto vect = series[n]->points();

                for (size_t i = 0; i < vect.size(); i++)
                    series[n]->replace(i, vect[i].x(), vect[i].y() / axis_multiplier * updated_multiplier );
            }
        }

        axis_multiplier = updated_multiplier;
        axis_multiplier_type = updated_axis_multiplier_type;
    }

    view_title();
}


void SmryYaxis::remove_last_unit()
{
    m_titles.pop_back();
    view_title();
}


void SmryYaxis::add_title(std::string title)
{
    m_titles.push_back(title);
}


void SmryYaxis::view_title()
{
    std::string axis_title_text = "";

    std::vector<std::string> unique_units;

    for (auto val : m_titles){
        std::vector<std::string>::iterator it = std::find(unique_units.begin(), unique_units.end(), val);

        if (it == unique_units.end()){
            unique_units.push_back(val);
        }
    }

    axis_title_text = unique_units[0];

    for (size_t n = 1; n < unique_units.size(); n++)
        axis_title_text = axis_title_text + ", " + unique_units[n];

    switch (axis_multiplier_type) {
    case AxisMultiplierType::billion:
        axis_title_text = "1.0e+9 , " + axis_title_text;
        break;
    case AxisMultiplierType::million:
        axis_title_text = "1.0e+6 , " + axis_title_text;
        break;
    case AxisMultiplierType::thousand:
        axis_title_text = "1.0e+3 , " + axis_title_text;
        break;
    }

    this->setTitleText(QString::fromStdString(axis_title_text));
}


float SmryYaxis::calc_p90(const std::vector<float>& data)
{
    auto tmp = data;

    std::sort(tmp.begin(), tmp.end());
    size_t  p = static_cast<size_t>(tmp.size() * 0.9) ;

    return tmp[p];
}


void SmryYaxis::setMinAndMax(double min_val, double max_val)
{
   m_min = min_val;
   m_max = max_val;
}


void SmryYaxis::resetAxisRange()
{
    this->setRange(m_min, m_max);

}


void SmryYaxis::print_axis_range()
{
    std::cout << "min: " << m_min << " max: " << m_max << std::endl;
};

bool SmryYaxis::set_range(double min_val, double max_val)
{
    //  this->setRange is calling QValueAxis::setRange

    if (min_val < max_val){
        this->setRange(static_cast<qreal>(min_val), static_cast<qreal>(max_val));
        return true;
    } else {
        return false;
    }
}


