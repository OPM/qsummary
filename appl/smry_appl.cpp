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

#include <appl/smry_appl.hpp>


#include <iostream>
#include <iomanip>
#include <math.h>
#include <omp.h>
#include <limits>
#include <algorithm>


SmryAppl::SmryAppl(std::vector<std::string> arg_vect, loader_list_type& loaders,
                   input_list_type chart_input, std::unique_ptr<DerivedSmry>& derived_smry, QWidget *parent)
    : QGraphicsView(new QGraphicsScene, parent)
{

    if (derived_smry != nullptr)
        m_derived_smry = std::move(derived_smry);


    this->initColorAndStyle();

    auto fileList = std::get<0>(loaders);
    int max_pr_chart = max_vect_chart(chart_input);

    if (fileList.size() > 0) {

        m_smry_files = std::move(std::get<0>(loaders));
        m_file_type = std::move(std::get<1>(loaders));
        m_esmry_loader = std::move(std::get<2>(loaders));
        m_ext_esmry_loader = std::move(std::get<3>(loaders));

        for (size_t smry_ind = 0; smry_ind < m_smry_files.size(); smry_ind++){
            auto ftime = std::filesystem::last_write_time ( m_smry_files[smry_ind] );
            file_stamp_vector.push_back ( ftime );

            if (m_file_type[smry_ind] == FileType::SMSPEC) {
                root_name_list.push_back ( m_esmry_loader[smry_ind]->rootname() );
                vect_list.push_back ( m_esmry_loader[smry_ind]->keywordList() );

                if ((!m_esmry_loader[smry_ind]->hasKey("TIMESTEP") && (m_esmry_loader[smry_ind]->all_steps_available())))
                    vect_list.back().push_back("TIMESTEP");

            } else if (m_file_type[smry_ind] == FileType::ESMRY) {
                root_name_list.push_back ( m_ext_esmry_loader[smry_ind]->rootname() );
                vect_list.push_back ( m_ext_esmry_loader[smry_ind]->keywordList() );

                if ((!m_ext_esmry_loader[smry_ind]->hasKey("TIMESTEP") && (m_ext_esmry_loader[smry_ind]->all_steps_available())))
                    vect_list.back().push_back("TIMESTEP");
            }
        }
    }

    axis_mode = false;

    str_var = "";

    cmd_line_error = false;

    p1 = 0;
    p2 = 0;

    vect_ind = 0;
    smry_ind = 0;

    chart_ind = 0;

    stackedWidget = new QStackedWidget;

    this->init_new_chart();

    grid1  = new QGridLayout ( this );
    hbox1 = new QHBoxLayout();

    lbl_cmd = new QLabel ( "Commands:", this );
    lbl_num = new QLabel ( "# matches", this );

    lbl_rootn = new QLabel ( "" , this );

    //max_pr_chart

    if (chart_input.size() > 0){
        ens_mode = max_pr_chart > 9 ? true : false;
    } else {
        ens_mode = m_smry_files.size() > 9 ? true : false;
    }

    lbl_plot = new QLabel ( "new chart", this );

    lbl_cmd->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );

    le_commands = new QLineEdit ( this );

    charts_list.push_back ( {} );

    QtCharts::QChart* chart = new QtCharts::QChart();

    chart->setMinimumSize(1400, 600);

    grid1->addWidget ( stackedWidget, 0, 0 );

    hbox1->addWidget ( lbl_cmd );
    hbox1->addWidget ( le_commands );

    hbox1->addWidget ( lbl_num );
    hbox1->addStretch();

    hbox1->addWidget ( lbl_rootn );

    hbox1->addStretch();
    hbox1->addWidget ( lbl_plot );

    grid1->addLayout ( hbox1, 1, 0 );

    le_commands->setFocus();

    connect ( le_commands, &QLineEdit::textEdited, this, &SmryAppl::command_modified );

    le_commands->installEventFilter ( this );

    lbl_num->setText ( QString::fromStdString ( "" ) );

    if ( chart_input.size() > 0 )
        this->create_charts_from_input ( chart_input );

    double total_opening = 0.0;
    double total_loading = 0.0;

    for (size_t n = 0; n < m_file_type.size(); n++){
        if (m_file_type[n] == FileType::SMSPEC){
            auto elapsed = m_esmry_loader[n]->get_io_elapsed();
            total_opening += std::get<0>(elapsed);
            total_loading += std::get<1>(elapsed);
        } else if (m_file_type[n] == FileType::ESMRY){
            auto elapsed = m_ext_esmry_loader[n]->get_io_elapsed();
            total_opening += std::get<0>(elapsed);
            total_loading += std::get<1>(elapsed);
        }
    }

    if (fileList.size() > 0) {

        m_smry_loaded = true;

        std::ostringstream ss;

        ss << "I/O  opening " << std::fixed << std::setprecision(5) << total_opening;
        ss << " sec, loading: " << std::fixed << std::setprecision(5) << total_loading << " sec";

        std::cout << ss.str() << std::endl;

    } else {

        m_smry_loaded = false;
        lbl_plot->setText("use <ctrl> + o to open a summary file ");
        this->le_commands->setEnabled(0);

        QPalette *palette = new QPalette();
        palette->setColor ( QPalette::Text,Qt::gray );

        lbl_cmd->setPalette ( *palette );
    }
}

void SmryAppl::init_new_chart()
{
    charts_list.push_back ( {} );

    QtCharts::QChart* chart = new QtCharts::QChart();
    chartList.push_back ( chart );

    ChartView *chart_view = new ChartView( chart, this );

    chart_view_list.push_back ( chart_view );
    stackedWidget->addWidget(chart_view);

    axisX.push_back ( nullptr );

    num_yaxis.push_back ( 0 );
    yaxis_units.push_back ( {} );

    axisY.push_back ( {} );
    series.push_back ( {} );
    ens_series.push_back ( {} );
}


void SmryAppl::create_charts_from_input ( const input_list_type& chart_input )
{

    if (ens_mode) {

        for ( size_t c = 0; c < chart_input.size(); c++ ) {
            auto vect_input = std::get<0>(chart_input[c]);

            std::cout << "size: " << vect_input.size() << std::endl;
            std::string xrange_str = std::get<1>(chart_input[c]);

            std::cout << "xrange_str: " << xrange_str << std::endl;
            // bool SmryAppl::add_new_ens_series ( int chart_ind, std::string vect_name, int vaxis_ind )

            for (size_t n = 0; n < vect_input.size(); n++){

                int smryind = std::get<0> ( vect_input[n] );
                std::string vect_name = std::get<1> ( vect_input[n] );

                std::cout << n;
                std::cout << " smry index: " << smryind;
                std::cout << " vect_name: " << vect_name;

                std::cout << std::endl;
            }

        }

        std::cout << "ensemble chart from cmd line under construction \n";
        exit(1);

    } else {

        for ( size_t c = 0; c < chart_input.size(); c++ ) {

            chart_ind = c;

            if ( chart_ind > 0 )
                this->init_new_chart();

            std::vector<SmryAppl::vect_input_type> vect_input;
            vect_input = std::get<0>(chart_input[c]);
            std::string xrange_str = std::get<1>(chart_input[c]);

            for ( size_t i = 0; i < vect_input.size(); i++ ) {
                int n = std::get<0> ( vect_input[i] );
                std::string vect_name = std::get<1> ( vect_input[i] );
                int axis_ind = std::get<2> ( vect_input[i] );
                bool is_derived = std::get<3> ( vect_input[i] );

                if (this->add_new_series ( chart_ind, n, vect_name, axis_ind, is_derived) == false) {
                    std::cout << "!warning, not able to add series '" << vect_name <<"' for case ";
                    std::cout << root_name_list[n]  << "\n";
                }

            }

            if (series[c].size() > 0)
                update_full_xrange(c);

            if (xrange_str.size() > 0) {

                if (!axisX[c]->set_range ( xrange_str ))
                    std::cout << "!Warning, fail to set x-range for chart index: " << chart_ind << "\n";
                else {
                    auto min_max_range = axisX[c]->get_xrange();
                    update_all_yaxis(min_max_range, c);  // should this be false ?
                }

            } else {

                auto min_max_range = axisX[c]->get_xrange();
                //axisX[c]->resetAxisRange();
                update_all_yaxis(min_max_range, c);
            }
        }
    }

    chart_ind = 0;
    stackedWidget->setCurrentIndex(chart_ind);

    this->update_chart_labels();
}


template <typename T>
bool SmryAppl::double_check_well_vector(std::string& vect_name, std::unique_ptr<T>& smry)
{
    auto p = vect_name.find(":");
    auto wname = vect_name.substr(p + 1);

    if (wname.size() > 8)
        wname = wname.substr(0,8);

    std::string pattern = vect_name.substr(0, p + 1) + wname + "*";

    auto vlist = smry->keywordList(pattern);

    if (vlist.size() == 1){
        vect_name = vlist[0];
        return true;
    } else {
        return false;
    }

    return true;
}

bool SmryAppl::add_new_series ( int chart_ind, int smry_ind, std::string vect_name, int vaxis_ind, bool is_derived)
{
    std::vector<float> datav;

    std::vector<float> timev;

    auto start_get = std::chrono::system_clock::now();

    if ((is_derived) && (smry_ind < 0)){
        timev = m_derived_smry->get(smry_ind, "TIME" );
    } else if (m_file_type[smry_ind] == FileType::SMSPEC)
        timev = m_esmry_loader[smry_ind]->get("TIME");
    else if (m_file_type[smry_ind] == FileType::ESMRY)
        timev = m_ext_esmry_loader[smry_ind]->get("TIME");

    std::string smry_unit;

    bool hasVect;
    bool all_steps;

    if (is_derived){
        hasVect = m_derived_smry->is_derived(smry_ind, vect_name);

        if (!hasVect)
            throw std::runtime_error("derived smry vector " + std::to_string(smry_ind) + ":" + vect_name + " not found in m_derived_smry");

    } else if (m_file_type[smry_ind] == FileType::SMSPEC){
        hasVect = m_esmry_loader[smry_ind]->hasKey(vect_name);
    } else if (m_file_type[smry_ind] == FileType::ESMRY) {
        hasVect = m_ext_esmry_loader[smry_ind]->hasKey(vect_name);
    }

    // well name in Flow and Eclipse (input data file) can be longer than 8 characters
    // In case this happens, these well names will be truncated to 8 characters before ending up
    // in smspec files. For ESMRY the following may be the case.
    //  a) ESMRY files produced by simulator Flow will contain all characters
    //  b) ESMRY files post simulation converted from SMSPEC/UNSMRY -> ESMRY will har truncated well names


    if ((!hasVect) && (vect_name.substr(0,1) == "W")){

        if (m_file_type[smry_ind] == FileType::SMSPEC)
            hasVect = this->double_check_well_vector(vect_name, m_esmry_loader[smry_ind]);
        else if (m_file_type[smry_ind] == FileType::ESMRY)
            hasVect = this->double_check_well_vector(vect_name, m_ext_esmry_loader[smry_ind]);
    }


    if (!hasVect){
        std::cout << "\n!Error loading vector '" << vect_name << "'";
        std::cout << " from summary file: " << m_smry_files[smry_ind].string() << "\n\n";
        exit(1);
    }

    if ((vect_name == "TIMESTEP") && (all_steps) && (!hasVect)){

        datav.reserve(timev.size());
        datav.push_back(timev[0]);

        for (size_t n = 1; n < timev.size(); n++) {
            datav.push_back(timev[n] - timev[n-1]);
        }

        smry_unit = "DAYS";

    } else {

        auto start_get = std::chrono::system_clock::now();

        if (is_derived){
            datav = m_derived_smry->get(smry_ind, vect_name );
            smry_unit = m_derived_smry->get_unit ( smry_ind, vect_name );

        } else if (m_file_type[smry_ind] == FileType::SMSPEC){
            hasVect = m_esmry_loader[smry_ind]->hasKey(vect_name);

            try {
                datav = m_esmry_loader[smry_ind]->get ( vect_name );
            } catch (...){
                std::string message;
                message = "Error loading " + vect_name + " from SMSPEC/UNSMRY " + m_esmry_loader[smry_ind]->rootname();
                throw std::runtime_error(message);
            }

            smry_unit = m_esmry_loader[smry_ind]->get_unit ( vect_name );

        } else if (m_file_type[smry_ind] == FileType::ESMRY){
            hasVect = m_ext_esmry_loader[smry_ind]->hasKey(vect_name);

            try {
                datav = m_ext_esmry_loader[smry_ind]->get ( vect_name );
            } catch (...) {
                std::string message;
                message = "Error loading " + vect_name + " from ESMRY " + m_ext_esmry_loader[smry_ind]->rootname();
                throw std::runtime_error(message);
            }

            smry_unit = m_ext_esmry_loader[smry_ind]->get_unit ( vect_name );
        }

        if (!hasVect)
            return false;
    }


    if ( vect_name.substr(0,4) == "WWIP" )
        smry_unit = "SM3/DAY";   // unit missing when smry file genrated by Eclipse
    else if ( vect_name.substr(0,3) == "WPI" )
        smry_unit = "SM3/D/B";   // different unit in flow and Eclipse, using Eclipse version

    if ( smry_unit.size() > 0 ) {
        int p1 = smry_unit.find_first_not_of ( " " );
        smry_unit = smry_unit.substr ( p1 );
    }

    std::chrono::system_clock::time_point startd; // = esmry_list[smry_ind]->startdate();
    std::vector<int> start_vect;

    QDate d1;
    QTime tm1;

    if (smry_ind < 0){
        startd = m_derived_smry->startdate();
    } else if (m_file_type[smry_ind] == FileType::SMSPEC){
        startd = m_esmry_loader[smry_ind]->startdate();
        start_vect = m_esmry_loader[smry_ind]->start_v();

        int sec = start_vect[5] / 1000000;
        int millisec = (start_vect[5] % 1000000) / 1000;

        d1.setDate(start_vect[2], start_vect[1], start_vect[0]);
        tm1.setHMS(start_vect[3], start_vect[4], sec, millisec);

    } else if (m_file_type[smry_ind] == FileType::ESMRY){

        startd = m_ext_esmry_loader[smry_ind]->startdate();
        start_vect = m_ext_esmry_loader[smry_ind]->start_v();
        d1.setDate(start_vect[2], start_vect[1], start_vect[0]);
        tm1.setHMS(start_vect[3], start_vect[4], start_vect[5], start_vect[6]);
    }

    QDateTime dt_start_sim(d1, tm1, Qt::UTC);

    vectorEntry ve = make_vector_entry ( vect_name );

    SeriesEntry serie_data = std::make_tuple ( smry_ind, vect_name, ve, smry_unit, vaxis_ind, is_derived);

    charts_list[chart_ind].push_back ( serie_data );

    // ->  1.0e-4


    AxisMultiplierType mult_type;
    float multiplier = 1.0;

    float val_p90 = calc_p90 ( datav );

    if ( val_p90 > 1.0e9 ) {
        mult_type = AxisMultiplierType::billion;
        multiplier = 1e-9;
    } else if ( val_p90 > 1.0e6 ) {
        mult_type = AxisMultiplierType::million;
        multiplier = 1e-6;
    } else if ( val_p90 > 1.0e3 ) {
        mult_type = AxisMultiplierType::thousand;
        multiplier = 1e-3;
    } else
        mult_type = AxisMultiplierType::one;

    // ->  1.6e-3


    if ( vaxis_ind > static_cast<int> ( axisY[chart_ind].size() + 1 ) )
        throw std::invalid_argument ( "invalid axis index" );

    int yaxsis_ind;

    if ( vaxis_ind == -1 ) {

        yaxsis_ind = -1;

        for ( size_t n = 0; n < yaxis_units[chart_ind].size(); n++ ) {
            if ( yaxis_units[chart_ind][n] == smry_unit )
                yaxsis_ind = static_cast<int> ( n );
        }

    } else if ( vaxis_ind == ( axisY[chart_ind].size() + 1 ) ) {
        yaxsis_ind = -1;

    } else {
        yaxsis_ind = vaxis_ind -1;
    }

    if ( yaxsis_ind == -1 ) {

        yaxis_units[chart_ind].push_back ( smry_unit );
        num_yaxis[chart_ind]++;

        switch ( mult_type ) {
        case AxisMultiplierType::billion:
            axisY[chart_ind].push_back ( new SmryYaxis ( mult_type, float ( 1.0e-9 ) ) );
            break;
        case AxisMultiplierType::million:
            axisY[chart_ind].push_back ( new SmryYaxis ( mult_type, 1.0e-6 ) );
            break;
        case AxisMultiplierType::thousand:
            axisY[chart_ind].push_back ( new SmryYaxis ( mult_type, 1.0e-3 ) );
            break;
        case AxisMultiplierType::one:
            axisY[chart_ind].push_back ( new SmryYaxis ( mult_type, 1.0 ) );
            break;
        default:
            throw std::invalid_argument ( "unknown axis multiplier type" );
            break;
        }

        yaxsis_ind = axisY[chart_ind].size()-1;

        if ( yaxsis_ind == 0 )
            chartList[chart_ind]->addAxis ( axisY[chart_ind][yaxsis_ind], Qt::AlignLeft );
        else
            chartList[chart_ind]->addAxis ( axisY[chart_ind][yaxsis_ind], Qt::AlignRight );

        axisY[chart_ind][yaxsis_ind]->add_title ( smry_unit );
        axisY[chart_ind][yaxsis_ind]->view_title();

    } else {

        if ( mult_type != axisY[chart_ind][yaxsis_ind]->multiplier_type() ) {

            float existing_mult = axisY[chart_ind][yaxsis_ind]->multiplier();

            if ( axisY[chart_ind][yaxsis_ind]->multiplier() != multiplier ) {

                multiplier = existing_mult;

            } else {

                axisY[chart_ind][yaxsis_ind]->update_series_data ( mult_type, multiplier, series[chart_ind] );
            }

            axisY[chart_ind][yaxsis_ind]->view_title();
        }

        axisY[chart_ind][yaxsis_ind]->add_title ( smry_unit );

        axisY[chart_ind][yaxsis_ind]->view_title();
    }

    // ->  3.2e-3

    series[chart_ind].push_back ( new SmrySeries(chartList[chart_ind]) );

    std::string objName;

    if (smry_ind < 0)
        objName = "Derived Smry " + vect_name;
    else
        objName = root_name_list[smry_ind] + " " + vect_name;

    series[chart_ind].back()->setObjectName ( QString::fromStdString ( objName ) );

    size_t n0 = 0;
    size_t n1 = datav.size() - 1;

    bool use_minimum_range = false;


    if ( ( vect_name.substr ( 0,1 ) == "W" ) && ( use_minimum_range ) ) {

        vectorEntry ve = make_vector_entry ( vect_name );

        std::string wbhp_name = "WBHP:" + std::get<1> ( ve );

        if ( has_smry_vect(smry_ind, wbhp_name ) ) {

            std::vector<float> wbhp;

            if (m_file_type[smry_ind] == FileType::SMSPEC)
                wbhp = m_esmry_loader[smry_ind]->get ( wbhp_name );
            else if (m_file_type[smry_ind] == FileType::ESMRY)
                wbhp = m_ext_esmry_loader[smry_ind]->get ( wbhp_name );

            while ( ( n0 < wbhp.size() ) && ( wbhp[n0] == 0.0 ) )
                n0++;

            while ( ( n1 > n0 ) && ( wbhp[n1] == 0.0 ) )
                n1--;
        }
    }

    if ( n0 == timev.size() )
        n0 = 0;

    // ->  3.2e-3

    for ( size_t n = n0; n <  n1 + 1; n++ ) {

        if (!isnan(datav[n])) {
            double d_msec = static_cast<double>(timev[n])* 24.0 * 3600.0 * 1000.0;
            d_msec = round(d_msec);

            QDateTime dtime = dt_start_sim;
            dtime.setTimeSpec(Qt::UTC);      // improves runtime significantly

            dtime = dtime.addMSecs(static_cast<qint64>(d_msec));

            //QString dt1_qstr = dtime.toString("yyyy-MM-dd HH:mm:ss.zzz");
            //std::cout << "serie data time : " << dt1_qstr.toStdString() << std::endl;

            series[chart_ind].back()->append ( dtime.toMSecsSinceEpoch(), datav[n] * multiplier );
        }
    }

    // ->  4.0e-3

    series[chart_ind].back()->setPointsVisible ( false );
    series[chart_ind].back()->calcMinAndMax();

    // ->  4.7e-3

    chartList[chart_ind]->addSeries ( series[chart_ind].back() );

    int ind = series[chart_ind].size() - 1;

    QPen pen;
    pen.setColor ( color_tab[ind] );
    pen.setWidth ( linew );
    pen.setStyle ( style_tab[ind] );

    series[chart_ind].back()->setPen( pen );

    if ( axisX[chart_ind] == nullptr ) {

        axisX[chart_ind] = new SmryXaxis(chart_view_list[chart_ind]);

        auto vect_x1 = series[chart_ind].back()->pointsVector();
        //axisX[chart_ind]->set_min_value ( vect_x1[0].x() );

        chartList[chart_ind]->addAxis ( axisX[chart_ind], Qt::AlignBottom );
    }

    // ->  5.2e-3


    series[chart_ind].back()->attachAxis ( axisX[chart_ind] );
    series[chart_ind].back()->attachAxis ( axisY[chart_ind][yaxsis_ind] );

    // ->  5.6e-3

    yaxis_map[series[chart_ind].back()] = axisY[chart_ind][yaxsis_ind];

    this->update_xaxis_range ( axisX[chart_ind] );
    this->update_axis_range ( axisY[chart_ind][yaxsis_ind] );

    this->update_chart_title_and_legend ( chart_ind );

    // Click to highlight, legend tooltip and legend reflecting series style
    QLegend* legend = chartList[chart_ind]->legend();
    legend->setShowToolTips(true);
    auto cur_series = series[chart_ind].back();
    cur_series->setPointLabelsVisible(false);
    QList<QLegendMarker*> markers = legend->markers(cur_series);
    if (markers.size() > 0) {
        auto marker = markers.at(0);
        connect(markers.at(0), &QLegendMarker::clicked, [=]() {
            if (!cur_series->isHighlighted()) {
                QFont font = marker->font();
                font.setBold(true);
                marker->setFont(font);
                QPen pen = cur_series->pen();
                pen.setWidth(4);
                cur_series->setPen(pen);
                cur_series->setHighlighted(true);
            } else {
                QFont font = marker->font();
                font.setBold(false);
                marker->setFont(font);
                QPen pen = cur_series->pen();
                pen.setWidth(2);
                cur_series->setPen(pen);
                cur_series->setHighlighted(false);
            }
        });
        QPen pen = cur_series->pen();
        marker->setPen(pen);
        marker->setShape(QLegend::MarkerShapeFromSeries);
    }

    // tskille: need something here to repaint the chart view.
    // this is not a good fix, but works for now
    // this is causing an segmentation fault when used with delete series
    chart_view_list[chart_ind]->update_graphics();


    std::string lbl_str = std::to_string ( chart_ind + 1 ) + "/" + std::to_string ( chartList.size() );
    lbl_plot->setText ( QString::fromStdString ( lbl_str ) );

    return true;
}



bool SmryAppl::add_new_ens_series ( int chart_ind, std::string vect_name, int vaxis_ind )
{
    auto start_open_add_series = std::chrono::system_clock::now();

    size_t nthreads = omp_get_max_threads();

    std::cout << "\nNumber of threads: " << nthreads;

    omp_set_num_threads(nthreads);

    double before_loading = 0.0;

    for (size_t n = 0; n < m_file_type.size(); n++){
        if (m_file_type[n] == FileType::SMSPEC){
            auto elapsed = m_esmry_loader[n]->get_io_elapsed();
            before_loading += std::get<1>(elapsed);
        } else if (m_file_type[n] == FileType::ESMRY){
            auto elapsed = m_ext_esmry_loader[n]->get_io_elapsed();
            before_loading += std::get<1>(elapsed);
        }
    }


    size_t num_files = m_file_type.size();

    std::vector<std::vector<float>> datav;
    std::vector<std::vector<float>> timev;

    for (size_t n = 0; n < num_files; n++){
        datav.push_back({});
        timev.push_back({});
    }

    auto start_loading = std::chrono::system_clock::now();

    #pragma omp parallel for
    for (size_t ind = 0; ind < m_file_type.size(); ind ++){
        if (m_file_type[ind] == FileType::SMSPEC){
            timev[ind] = m_esmry_loader[ind]->get("TIME");
            datav[ind] = m_esmry_loader[ind]->get ( vect_name );
        } else if (m_file_type[ind] == FileType::ESMRY) {
            timev[ind] = m_ext_esmry_loader[ind]->get("TIME");
            datav[ind] = m_ext_esmry_loader[ind]->get ( vect_name );
        }
    }

    auto end_loading = std::chrono::system_clock::now();

    std::chrono::duration<double> elapsed_loading = end_loading-start_loading;

    std::cout << " , elapsed loading: " <<  elapsed_loading.count() << std::endl;

    std::string smry_unit;

    if (m_file_type[0] == FileType::SMSPEC)
        smry_unit = m_esmry_loader[0]->get_unit ( vect_name );

    else if (m_file_type[smry_ind] == FileType::ESMRY)
        smry_unit = m_ext_esmry_loader[0]->get_unit ( vect_name );

    if ( vect_name == "TCPU" )
        smry_unit = "SECONDS";

    size_t yaxsis_ind = 0;

    axisY[chart_ind].push_back ( new SmryYaxis ( AxisMultiplierType::one, 1.0 ) );

    chartList[chart_ind]->addAxis ( axisY[chart_ind][yaxsis_ind], Qt::AlignLeft );

    axisY[chart_ind][yaxsis_ind]->add_title ( smry_unit );
    axisY[chart_ind][yaxsis_ind]->view_title();

    axisX[chart_ind] = new SmryXaxis(chart_view_list[chart_ind]);

    chartList[chart_ind]->addAxis ( axisX[chart_ind], Qt::AlignBottom );

    float minValue = std::numeric_limits<float>::max();
    float maxValue = std::numeric_limits<float>::min();

    std::vector<float>::const_iterator it;

    for (size_t ind = 0; ind < m_file_type.size(); ind ++){
        it = std::max_element(datav[ind].begin(), datav[ind].end());
        maxValue = std::max(maxValue, *it);

        it = std::min_element(datav[ind].begin(), datav[ind].end());
        minValue = std::min(minValue, *it);
    }

    for (size_t ind = 0; ind < m_file_type.size(); ind ++){

        std::chrono::system_clock::time_point startd;
        std::vector<int> start_vect;

        if (m_file_type[ind] == FileType::SMSPEC)
            startd = m_esmry_loader[ind]->startdate();
        else if (m_file_type[ind] == FileType::ESMRY){
            startd = m_ext_esmry_loader[ind]->startdate();
            start_vect = m_ext_esmry_loader[ind]->start_v();
        }

        for (auto& v : start_vect)
            std::cout << v << std::endl;

        exit(1);

        std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds> ( startd.time_since_epoch() );
        std::chrono::seconds s = std::chrono::duration_cast<std::chrono::seconds> ( ms );

        std::time_t t = s.count();

        int fractional_seconds = ms.count() % 1000;

        std::tm *ltm = localtime(&t);
        //std::tm *ltm = gmtime ( &t );

        QDate d1 {1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday};

        QTime tm1 { ltm->tm_hour, ltm->tm_min, ltm->tm_sec, fractional_seconds };
        QDateTime dt_start_sim;

        dt_start_sim.setDate ( d1 );
        dt_start_sim.setTime ( tm1 );

        dt_start_sim.setTimeSpec(Qt::UTC);

        vectorEntry ve = make_vector_entry ( vect_name );

        SeriesEntry serie_data = std::make_tuple ( ind, vect_name, ve, smry_unit, -1, false );

        charts_list[chart_ind].push_back ( serie_data );

        ens_series[chart_ind].push_back ( new QLineSeries() );

        std::string objName = root_name_list[ind] + " " + vect_name;
        ens_series[chart_ind].back()->setObjectName ( QString::fromStdString ( objName ) );

        size_t n1 = datav[ind].size() - 1;

        QDateTime dtime = dt_start_sim;
        double d_msec = static_cast<double>(timev[ind][0])* 24.0 * 3600.0 * 1000.0;
        d_msec = round(d_msec);

        dtime = dtime.addMSecs(static_cast<qint64>(d_msec));

        ens_series[chart_ind].back()->append ( dtime.toMSecsSinceEpoch(), datav[ind][0] );

        for ( size_t n = 1; n <  n1 + 1; n++ ) {

            double d_msec = static_cast<double>(timev[ind][n] - timev[ind][n-1])* 24.0 * 3600.0 * 1000.0;
            d_msec = round(d_msec);

            dtime = dtime.addMSecs(static_cast<qint64>(d_msec));

            ens_series[chart_ind].back()->append ( dtime.toMSecsSinceEpoch(), datav[ind][n] );
        }

        ens_series[chart_ind].back()->setPointsVisible ( false );

        chartList[chart_ind]->addSeries (ens_series[chart_ind].back() );

        ens_series[chart_ind].back()->attachAxis ( axisY[chart_ind][yaxsis_ind] );
        ens_series[chart_ind].back()->attachAxis ( axisX[chart_ind] );

        QPen pen;
        pen.setColor (  QColor ( "gray" ) );
        pen.setWidth ( linew );
        pen.setStyle ( style_tab[1] );
        ens_series[chart_ind].back()->setPen ( pen );

    }

    chartList[chart_ind]->legend()->hide();
    chartList[chart_ind]->setTitle ( QString::fromStdString ( vect_name ) );

    double after_loading = 0.0;

    for (size_t n = 0; n < m_file_type.size(); n++){
        if (m_file_type[n] == FileType::SMSPEC){
            auto elapsed = m_esmry_loader[n]->get_io_elapsed();
            after_loading += std::get<1>(elapsed);
        } else if (m_file_type[n] == FileType::ESMRY){
            auto elapsed = m_ext_esmry_loader[n]->get_io_elapsed();
            after_loading += std::get<1>(elapsed);
        }
    }

    double total_loading = after_loading - before_loading;
    std::ostringstream ss;

    ss << "I/O  loading: " << std::fixed << std::setprecision(5) << total_loading << " sec\n";

    std::cout << ss.str() << std::endl;

    auto end_open_add_series = std::chrono::system_clock::now();

    std::chrono::duration<double> elapsed_seconds = end_open_add_series-start_open_add_series;

    std::cout << "\n -> duration adding ensemble series " <<  elapsed_seconds.count() << std::endl;

    return true;
}


SmryAppl::vectorEntry SmryAppl::make_vector_entry ( std::string vect_name )
{
    SmryAppl::vectorEntry res;

    const std::vector<std::string> misc {
        "DAY",
        "ELAPSED",
        "MAXDPR", "MAXDSG", "MAXDSO", "MAXDSW", "MLINEARS", "MONTH", "MSUMLINS", "MSUMNEWT",
        "NEWTON", "NLINEARS", "NLINSMAX", "NLINSMIN",
        "STEPTYPE",
        "TCPU", "TCPUDAY", "TCPUTS", "TELAPLIN", "TIME", "TIMESTEP",
        "YEAR", "YEARS",
    };

    if ( std::find ( misc.begin(), misc.end(), vect_name ) != misc.end() ) {
        res = std::make_tuple ( vect_name, "", "" );
    } else if ( vect_name.substr ( 0,1 ) == "A" ) {
        int p = vect_name.find_first_of ( ":" );
        res = std::make_tuple ( vect_name.substr ( 0,p ), vect_name.substr ( p+1 ), "" );
    } else if ( vect_name.substr ( 0,1 ) == "B" ) {
        int p = vect_name.find_first_of ( ":" );
        res = std::make_tuple ( vect_name.substr ( 0,p ), vect_name.substr ( p+1 ), "" );
    } else if ( vect_name.substr ( 0,1 ) == "C" ) {
        int p1 = vect_name.find_first_of ( ":" );
        int p2 = vect_name.find_first_of ( ":", p1+1 );

        std::string name = vect_name.substr ( 0,p1 );
        std::string item1 = vect_name.substr ( p1+1,p2-p1-1 );
        std::string item2 = vect_name.substr ( p2+1 );

        res = std::make_tuple ( name, item1, item2 );

    } else if ( vect_name.substr ( 0,1 ) == "F" ) {
        res = std::make_tuple ( vect_name, "", "" );
    } else if ( vect_name.substr ( 0,1 ) == "G" ) {
        int p = vect_name.find_first_of ( ":" );
        res = std::make_tuple ( vect_name.substr ( 0,p ), vect_name.substr ( p+1 ), "" );

     } else if ( vect_name.substr ( 0,1 ) == "R" ) {
        int p = vect_name.find_first_of ( ":" );
        res = std::make_tuple ( vect_name.substr ( 0,p ), vect_name.substr ( p+1 ), "" );

    } else if ( vect_name.substr ( 0,1 ) == "S" ) {
        int p1 = vect_name.find_first_of ( ":" );
        int p2 = vect_name.find_first_of ( ":", p1+1 );

        std::string name = vect_name.substr ( 0,p1 );
        std::string item1 = vect_name.substr ( p1+1,p2-p1-1 );
        std::string item2 = vect_name.substr ( p2+1 );

        res = std::make_tuple ( name, item1, item2 );

    } else if ( vect_name.substr ( 0,1 ) == "W" ) {
        int p = vect_name.find_first_of ( ":" );
        res = std::make_tuple ( vect_name.substr ( 0,p ), vect_name.substr ( p+1 ), "" );
    } else {
        throw std::runtime_error ( "vector '" + vect_name + "' of unknown type" );
    }

    return res;
}


void SmryAppl::update_chart_labels()
{
    std::string lbl_str = std::to_string ( chart_ind + 1 );

    if ( series.back().size() == 0 )
        lbl_str = lbl_str + "/" + std::to_string ( chartList.size() - 1 );
    else
        lbl_str = lbl_str + "/" + std::to_string ( chartList.size() );

    if ( series[chart_ind].size() == 0 )
        lbl_plot->setText ( "new chart" );
    else
        lbl_plot->setText ( QString::fromStdString ( lbl_str ) );
}


void SmryAppl::update_xaxis_range ( SmryXaxis* axis )
{
    qreal min_val = std::numeric_limits<quint64>::max();
    qreal max_val = 0.0;

    for ( size_t n = 0; n < series[chart_ind].size(); n ++ ) {

        auto x_axis = series[chart_ind][n]->attachedAxes();

        if ( x_axis[0] == axis ) {

            auto values = series[chart_ind][n]->pointsVector();

            for ( size_t m = 0; m < values.size(); m++ ) {
                if ( values[m].x() < min_val )
                    min_val = values[m].x();

                if ( values[m].x() > max_val )
                    max_val = values[m].x();
            }
        }
    }

    //axis->setMinAndMax ( min_val, max_val );
    //axis->resetAxisRange();
}

void SmryAppl::update_axis_range(SmryYaxis* axis){

    // loop through all series attached to this axis and calc
    // over all min and max values and call
    // -> bool axis->set_range(float min_val, float max_val);


    double min_y = +1.0*std::numeric_limits<double>::max();
    double max_y = -1.0*std::numeric_limits<double>::max();

    for (size_t e = 0; e < series[chart_ind].size(); e ++) {
        if (yaxis_map[series[chart_ind][e]] == axis) {

            auto s_min_max = series[chart_ind][e]->get_min_max_value();

            if (std::get<0>(s_min_max) < min_y)
                min_y = std::get<0>(s_min_max);

            if (std::get<1>(s_min_max) > max_y)
                max_y = std::get<1>(s_min_max);
        }
    }

    if (min_y == max_y){

        if (min_y > 0.0)
            min_y = 0.0;
        else
            max_y = 0.1;

        adjust_yaxis_props(axis, min_y, max_y);
    }

    if (!axis->set_range(min_y, max_y)){

        if (min_y != max_y) {
            std::ostringstream ss;
            ss << "Not able to set min and max for y-axis min:";
            ss << std::scientific << std::setprecision(5) << min_y << " max: ";
            ss << std::scientific << std::setprecision(5) << max_y;

            qWarning() << QString::fromStdString(ss.str());
        }
    }
}


void SmryAppl::adjust_yaxis_props(SmryYaxis* axis, double& min_data, double& max_data)
{
    double d1 = max_data - min_data;

    double min = min_data - 0.1*d1;
    double max = max_data + 0.1*d1;

    //std::cout << "adjust_yaxis_props (step1): " << std::setprecision(2) << min << "  " << max << std::endl;

    double delta = 0.1;
    int nThicks = 5;

    if ( ( min == 0.0 ) && ( max == 0.0 ) ) {
        max = 0.1;
        delta = 0.05;

    } else {

        delta = ( max - min ) / nThicks;
        double logval = floor ( log10 ( delta ) );

        delta = delta / pow ( 10, logval );

        delta = floor ( delta ) *pow ( 10, logval );

        double new_min = floor ( min / delta ) *delta;
        double new_max = new_min + nThicks*delta;

        if ((new_min < 0.0) && (min_data >=0.0))
            new_min = 0.0;

        while ( new_max < max ) {
            new_max = new_max + delta;
            nThicks++;
        }

        max = new_max;
        min = new_min;
    }

    axis->setRange ( min, max );

    axis->setTickCount ( nThicks+1 );
    axis->setMinorTickCount ( 2 );

    min_data = min;
    max_data = max;
}


template <typename T>
bool SmryAppl::reopen_loader(int n, std::unique_ptr<T>& smry)
{
    std::unique_ptr<T> smry_tmp;

    try {
        smry_tmp = std::make_unique<T> ( m_smry_files[n] );
    } catch (...) {
        std::string message = "Error with reopen loader, failed when opening summary file " + m_smry_files[n].string();
        throw std::runtime_error(message);
    }

    auto nstep_tmp = smry_tmp->numberOfTimeSteps();
    auto nstep = smry->numberOfTimeSteps();

    auto ftime = std::filesystem::last_write_time ( m_smry_files[n] );
    bool updated = false;

    if ( ftime > file_stamp_vector[n] ){
        smry = std::move ( smry_tmp );
        updated = true;
    } else if ( nstep_tmp > nstep ){
        smry = std::move ( smry_tmp );
        updated = true;
    }

    if (updated) {
        auto ftime = std::filesystem::last_write_time ( m_smry_files[n] );
        file_stamp_vector[n] = ftime;
    }

    vect_list.push_back ( smry->keywordList() );

    return updated;
}

void SmryAppl::reset_axis_state(int chart_index, const std::vector<std::vector<QDateTime>>& xrange_state)
{
    size_t num_charts = xrange_state.size();

    QDateTime dt_min_utc = xrange_state[chart_index][0];
    QDateTime dt_max_utc = xrange_state[chart_index][1];
    QDateTime xr_from = xrange_state[chart_index][2];
    QDateTime xr_to = xrange_state[chart_index][3];
    QDateTime full_xr_from = xrange_state[chart_index][4];
    QDateTime full_xr_to = xrange_state[chart_index][5];

    bool full_range = false;

    if ((dt_min_utc == full_xr_from) && (dt_max_utc == full_xr_to))
        full_range = true;

    auto full_xrange = calc_min_max_dt(chart_index);
    full_xr_from = std::get<0>(full_xrange);
    full_xr_to = std::get<1>(full_xrange);

    if (full_range) {
        dt_min_utc = full_xr_from;
        dt_max_utc = full_xr_to;
    }

    std::vector<QDateTime> mod_xrange_state;
    mod_xrange_state = {dt_min_utc, dt_max_utc, xr_from, xr_to, full_xr_from, full_xr_to};

    axisX[chart_index]->set_xrange_state(mod_xrange_state);
}


bool SmryAppl::reload_and_update_charts()
{
    // check if some of the files are updated and re-load if this is the case

    vect_list.clear();
    bool need_update = false;
    int n_smry = static_cast<int>(m_smry_files.size());
    std::vector<bool> updated_list;
    updated_list.resize(n_smry);

    for (size_t n = 0; n < m_smry_files.size(); n++){

        if (std::filesystem::exists(m_smry_files[n] )) {

            if (m_file_type[n] == FileType::SMSPEC)
                updated_list[n] = reopen_loader( n, m_esmry_loader[n] );
            else if (m_file_type[n] == FileType::ESMRY)
                updated_list[n] = reopen_loader( n, m_ext_esmry_loader[n] );

            if (updated_list[n])
                need_update = true;

        } else {

            std::string msg = "re-load of smspec " + m_smry_files[n].string() + "failed, file not found ";
            std::cout << "\n!warning, " << msg << std::endl;
        }
    }

    if (!need_update)
        return false;

    if (m_derived_smry != nullptr)
        m_derived_smry->recalc(m_file_type, m_esmry_loader, m_ext_esmry_loader);

    std::vector<std::vector<std::tuple<int, std::string, int, bool>>> series_properties;
    std::vector<std::vector<QDateTime>> xrange_state;

    for ( int ind = 0; ind < chartList.size(); ind++ )
        if (charts_list[ind].size() > 0)
            xrange_state.push_back(axisX[ind]->get_xrange_state());

    for ( int ind = 0; ind < chartList.size(); ind++ ) {

        series_properties.push_back ( {} );

        for ( size_t n = 0; n < charts_list[ind].size(); n++ ) {

            std::tuple<int, std::string, int, bool> props;
            props = std::make_tuple ( std::get<0> ( charts_list[ind][n] ), std::get<1> ( charts_list[ind][n] ),
                                     std::get<4> ( charts_list[ind][n]), std::get<5> (charts_list[ind][n] ));

            series_properties.back().push_back ( props );
        }
    }

    int current_chart_ind = chart_ind;

    // updating existing plots

    int num_charts = chartList.size();

    int n = num_charts - 1;

    while (n > -1){
        this->delete_chart(n);
        n--;
    }

    // make_preload_list and load data if smry file is updated

    for (int n = 0; n < n_smry ; n++) {
        if (updated_list[n]) {
            std::vector<std::string> pre_load_list = {"TIME"};

            for ( int c = 0; c < num_charts; c++ )
                for ( size_t m = 0; m < series_properties[c].size(); m++ )
                    if (std::get<0> ( series_properties[c][m] ) == n)
                        if (!std::get<3> ( series_properties[c][m] ))
                            pre_load_list.push_back(std::get<1> ( series_properties[c][m] ));

            if (m_file_type[n] == FileType::SMSPEC)
                m_esmry_loader[n]->loadData(pre_load_list);
            else if (m_file_type[n] == FileType::ESMRY)
                m_ext_esmry_loader[n]->loadData(pre_load_list);
        }
    }

    if (series_properties[num_charts - 1].size() == 0)
        num_charts --;

    for ( int ind = 0; ind < num_charts; ind++ ) {

        this->init_new_chart();

        chart_ind = ind;

        for ( size_t m = 0; m < series_properties[ind].size(); m++ ) {

            auto smry_ind = std::get<0> ( series_properties[ind][m] );
            auto vect_name = std::get<1> ( series_properties[ind][m] );
            auto vaxis_ind = std::get<2> ( series_properties[ind][m] );
            auto is_derived = std::get<3> ( series_properties[ind][m] );

            add_new_series ( ind, smry_ind, vect_name, vaxis_ind, is_derived);
        }

        this->reset_axis_state(ind, xrange_state);

        auto min_max_range = axisX[ind]->get_xrange();
        update_all_yaxis(min_max_range, ind);
    }

    chart_ind = current_chart_ind;

    stackedWidget->setCurrentIndex(chart_ind);

    return true;
}


void SmryAppl::delete_last_series()
{
    auto axis = series[chart_ind].back()->attachedAxes();

    if ( series[chart_ind].size() > 0 ) {

        series[chart_ind].back()->detachAxis ( axis[0] );
        series[chart_ind].back()->detachAxis ( axis[1] );

        chartList[chart_ind]->removeSeries ( series[chart_ind].back() );

        delete series[chart_ind].back();

        series[chart_ind].pop_back();

        charts_list[chart_ind].pop_back();
    }

    if ( series[chart_ind].size() == 0 ) {

        chartList[chart_ind]->removeAxis ( axisY[chart_ind][0] );
        chartList[chart_ind]->removeAxis ( axisX[chart_ind] );

        chart_view_list[chart_ind]->hide_xaxis_obj();

        delete ( axisX[chart_ind] );
        delete ( axisY[chart_ind][0] );

        axisX[chart_ind] = nullptr;

        yaxis_units[chart_ind].pop_back();

        axisY[chart_ind].pop_back();

    } else {

        int count = 0;

        for ( size_t n = 0; n < series[chart_ind].size(); n++ ) {

            auto series_axis = series[chart_ind][n]->attachedAxes();

            if ( series_axis[1] == axis[1] )
                count++;
        }

        if ( count > 0 ) {

            size_t ind = 0;

            for ( size_t n = 0; n < axisY[chart_ind].size(); n++ )
                if ( axis[1] == axisY[chart_ind][n] )
                    ind = n;

            axisY[chart_ind][ind]->update_axis_multiplier ( series[chart_ind] );
            axisY[chart_ind][ind]->remove_last_unit();

        } else {

            chartList[chart_ind]->removeAxis ( axis[1] );

            int ind = -1;

            for ( size_t m=0; m < axisY[chart_ind].size(); m++ )
                if ( axisY[chart_ind][m] == axis[1] )
                    ind = m;

            // assuption that always last axis that should be deleted/remove

            if ( ( ind + 1 ) != axisY[chart_ind].size() )
                throw std::runtime_error ( "!! yaxis to be deleted, not last in vector" );

            delete axisY[chart_ind][ind];

            axisY[chart_ind].pop_back();
            yaxis_units[chart_ind].pop_back();
        }
    }

    if (series[chart_ind].size() > 0) {
        auto min_max_range = axisX[chart_ind]->get_xrange();
        update_all_yaxis(min_max_range, chart_ind);
        this->update_chart_title_and_legend ( chart_ind );
    }

    if ( ( series[chart_ind].size() == 0 ) && ( chart_ind == ( chartList.size() - 1 ) ) ) {
        lbl_plot->setText ( "new chart" );
    }
}

void  SmryAppl::delete_chart ( int ind )
{
    chart_ind = ind;

    while ( series[chart_ind].size() > 0 )
        this->delete_last_series();

    chartList.erase ( chartList.begin() + ind );

    axisX.erase ( axisX.begin() + ind );
    axisY.erase ( axisY.begin() + ind );
    series.erase ( series.begin() + ind );

    yaxis_units.erase ( yaxis_units.begin() + ind );

    charts_list.erase ( charts_list.begin() + ind );

    stackedWidget->removeWidget(chart_view_list[chart_ind]);

    ChartView* p_chart_view = chart_view_list[chart_ind];

    chart_view_list.erase ( chart_view_list.begin() + ind );

    delete p_chart_view;

    if (chart_ind == stackedWidget->count())
        chart_ind--;

    this->update_chart_labels();
}


void SmryAppl::update_chart_title_and_legend ( int chart_ind )
{
    auto series_props = charts_list[chart_ind];

    std::set<int> smry_ind_set;
    std::set<std::string> name_set;
    std::set<std::string> item1_set;
    std::set<std::string> item2_set;

    bool legende_rootn = true;
    bool legende_name = true;
    bool legende_item1 = true;
    bool legende_item2 = true;

    for ( size_t n=0; n < series_props.size(); n++ ) {

        SmryAppl::vectorEntry ve = std::get<2> ( series_props[n] );

        smry_ind_set.insert ( std::get<0> ( series_props[n] ) );
        name_set.insert ( std::get<0> ( ve ) );
        item1_set.insert ( std::get<1> ( ve ) );
        item2_set.insert ( std::get<2> ( ve ) );
    }

    std::string title = "";


    if ( smry_ind_set.size() == 1 ) {

        int smry_index = *(smry_ind_set.begin());

        if (smry_index < 0)
            title = "Derives Smry";
        else if (m_file_type[smry_index] == FileType::SMSPEC)
            title = m_esmry_loader[smry_index]->rootname();
        else if (m_file_type[smry_index] == FileType::ESMRY)
            title = m_ext_esmry_loader[smry_index]->rootname();

        legende_rootn = false;
    }

    if ( name_set.size() == 1 ) {
        if ( title.size() == 0 )
            title = *name_set.begin();
        else
            title = title + " - " + *name_set.begin();

        legende_name = false;
    }

    if ( item1_set.size() == 1 ) {
        if ( title.size() == 0 )
            title = *item1_set.begin();
        else
            title = title + ":" + *item1_set.begin();

        legende_item1 = false;
    }

    if ( item2_set.size() == 1 ) {
        if ( title.size() == 0 )
            title = *item2_set.begin();
        else
            title = title + ":" + *item2_set.begin();

        legende_item2 = false;
    }

    for ( size_t n=0; n < series_props.size(); n++ ) {

        std::string legende_string = "";
        int smry_ind = std::get<0> ( series_props[n] );
        SmryAppl::vectorEntry ve = std::get<2> ( series_props[n] );

        if ( legende_rootn ) {
            if (smry_ind < 0)
               legende_string = "Derived";
            else if (m_file_type[smry_ind] == FileType::SMSPEC)
               legende_string = m_esmry_loader[smry_ind]->rootname();
            else if (m_file_type[smry_ind] == FileType::ESMRY)
               legende_string = m_ext_esmry_loader[smry_ind]->rootname();

        }

        if ( legende_name )
            if ( legende_string.size() == 0 )
                legende_string = std::get<0> ( ve );
            else
                legende_string = legende_string + " - " + std::get<0> ( ve );

        if ( legende_item1 )
            if ( legende_string.size() == 0 )
                legende_string = std::get<1> ( ve );
            else
                legende_string = legende_string + " - " + std::get<1> ( ve );

        if ( legende_item2 )
            if ( legende_string.size() == 0 )
                legende_string = std::get<2> ( ve );
            else
                legende_string = legende_string + " - " + std::get<2> ( ve );

        series[chart_ind][n]->setName ( QString::fromStdString ( legende_string ) );
    }

    while ( title.back() == ':' ) {
        title = title.substr ( 0, title.size() - 1 );
    }

    chartList[chart_ind]->setTitle ( QString::fromStdString ( title ) );

    if ( ( not legende_rootn ) and ( not legende_name ) and ( not legende_item1 ) and ( not legende_item2 ) ) {
        chartList[chart_ind]->legend()->hide();
    } else {
        chartList[chart_ind]->legend()->show();
        chartList[chart_ind]->legend()->setAlignment ( Qt::AlignBottom );
    }
}


std::vector<std::string> SmryAppl::split_string ( std::string str_arg )
{
    std::vector<std::string> tokens;

    int p = str_arg.find ( " " );
    int p0 = 0;

    while ( p > -1 ) {
        tokens.push_back ( str_arg.substr ( p0, p - p0 ) );

        p0 = p + 1;
        p = str_arg.find ( " ", p0 );
    }

    tokens.push_back ( str_arg.substr ( p0, p - p0 ) );

    return tokens;
}


std::vector<std::string> SmryAppl::get_iter_sets(std::set<std::string>& items_set,
                                                 std::set<int>& smry_ind_set,
                                                 std::vector<int>& id_list, char c)
{
    std::vector<std::string> vect_name_list;

    for ( size_t n = 0; n < charts_list[chart_ind].size(); n++ ) {

        int id = std::get<0> ( charts_list[chart_ind][n] );
        std::string name = std::get<1> ( charts_list[chart_ind][n] );

        auto vect = std::get<2> ( charts_list[chart_ind][n] );

        if ( name.substr ( 0,1 )[0] == c ) {
            vect_name_list.push_back ( std::get<0> ( vect ) );
            id_list.push_back ( id );

            std::string item_name =  std::get<1> ( vect );

            items_set.insert ( item_name );
            smry_ind_set.insert ( id );
        }
    }

    return vect_name_list;
}


std::vector<std::string> SmryAppl::get_item_list(std::string filt, const std::vector<std::string>& vect_name_list,
                                    const std::vector<int>& id_list)
{
    std::vector<std::string> item_list;
    std::vector<std::string>::iterator it;

    for ( size_t n = 0; n < id_list.size(); n ++ ) {

        std::vector<std::string> wlist;
        int case_ind = id_list[n];

        switch (m_file_type[case_ind]) {
        case FileType::SMSPEC:
            wlist = m_esmry_loader[case_ind]->keywordList ( vect_name_list[n] + ":" + filt );
            break;
        case FileType::ESMRY:
            wlist = m_ext_esmry_loader[case_ind]->keywordList ( vect_name_list[n] + ":" + filt );
            break;
        default:
          throw std::runtime_error("invalied file type, can't be loaded");
        break;
    }


        for ( auto val : wlist ) {
            int p = val.find_last_of ( ":" );
            it = std::find(item_list.begin(), item_list.end(), val.substr ( p + 1) );

            if (it == item_list.end())
                item_list.push_back(val.substr ( p + 1));
        }
    }

    return item_list;
}


std::vector<std::string> SmryAppl::make_well_list ( int chart_ind, const std::string& filt )
{
    std::vector<std::string> well_list;
    std::set<std::string> well_set;
    std::set<int> smry_ind_set;

    std::vector<std::string> vect_name_list;
    std::vector<int> id_list;

    vect_name_list = get_iter_sets(well_set, smry_ind_set, id_list, 'W');

    // if number of wells used in selected chart is zero or greater that 1
    // it is not possible to use chart as template for generating multiple
    // charts.

    if ( well_set.size() != 1 )
        return well_list;


    std::vector<std::string> draft_well_list = get_item_list(filt, vect_name_list, id_list);

    for ( size_t n = 0; n < draft_well_list.size(); n++ ) {

        std::string well_name = draft_well_list[n];

        bool well_name_ok = true;

        for ( size_t n = 0; n < id_list.size(); n ++ ) {
            int case_ind = id_list[n];

            if (!has_smry_vect(case_ind, vect_name_list[n] + ":" + well_name))
                well_name_ok = false;
        }

        bool has_wbhp = true;

        std::string wbhp_vect_name = "WBHP";
        wbhp_vect_name = wbhp_vect_name + ":" + well_name;

        for ( size_t n = 0; n < id_list.size(); n ++ ){
            int case_ind = id_list[n];

            if (!has_smry_vect(case_ind, wbhp_vect_name))
                has_wbhp = false;
        }

        bool ignore_well = true;

        if ( has_wbhp ) {
            for ( size_t n = 0; n < id_list.size(); n ++ ) {
                auto data = get_smry_vect(id_list[n], wbhp_vect_name);
                for ( auto v : data )
                    if ( v > 0.0 )
                        ignore_well = false;
            }
        }

        if ( ignore_well )
            well_name_ok = false;

        if ( ( well_name_ok ) && ( well_name != *well_set.begin() ) )
            well_list.push_back ( well_name );
    }

    return well_list;
}


std::vector<std::string> SmryAppl::make_group_list ( int chart_ind, const std::string& filt )
{
    std::vector<std::string> group_list;
    std::set<std::string> grp_set;
    std::set<int> smry_ind_set;

    std::vector<std::string> vect_name_list;
    std::vector<int> id_list;

    vect_name_list = get_iter_sets(grp_set, smry_ind_set, id_list, 'G');

    // if number of wells used in selected chart is zero or greater that 1
    // it is not possible to use chart as template for generating multiple
    // charts.

    if ( grp_set.size() != 1 )
        return group_list;

    std::vector<std::string> draft_group_list = get_item_list(filt, vect_name_list, id_list);

    for ( size_t n = 0; n < draft_group_list.size(); n++ ) {

        std::string group_name = draft_group_list[n];

        bool group_name_ok = true;

        for ( size_t n = 0; n < id_list.size(); n ++ ) {
            if ( !has_smry_vect(id_list[n], vect_name_list[n] + ":" + group_name ) )
                group_name_ok = false;
        }

        if ( ( group_name_ok ) && ( group_name != *grp_set.begin() ) )
            group_list.push_back ( group_name );
    }

    return group_list;
}


std::vector<std::string> SmryAppl::make_aquifer_list ( int chart_ind, const std::string& filt )
{
    std::vector<std::string> aquifer_list;
    std::set<std::string> aqu_set;
    std::set<int> smry_ind_set;

    std::vector<std::string> vect_name_list;
    std::vector<int> id_list;

    vect_name_list = get_iter_sets(aqu_set, smry_ind_set, id_list, 'A');

    // if number of wells used in selected chart is zero or greater that 1
    // it is not possible to use chart as template for generating multiple
    // charts.

    if ( aqu_set.size() != 1 )
        return aquifer_list;

    std::vector<std::string> draft_aquifer_list = get_item_list(filt, vect_name_list, id_list);


    for ( size_t n = 0; n < draft_aquifer_list.size(); n++ ) {

        std::string aquifer_name = draft_aquifer_list[n];

        bool aquifer_name_ok = true;

        for ( size_t n = 0; n < id_list.size(); n ++ ) {
            if ( !has_smry_vect(id_list[n], vect_name_list[n] + ":" + aquifer_name ) )
                aquifer_name_ok = false;
        }

        if ( ( aquifer_name_ok ) && ( aquifer_name != *aqu_set.begin() ) )
            aquifer_list.push_back ( aquifer_name );
    }

    return aquifer_list;
}


void SmryAppl::make_well_charts ( std::string filt )
{
    auto well_list = make_well_list ( chart_ind, filt );

    int prev_chart_ind = chart_ind;

    for ( auto val : well_list ) {

        chart_ind ++;
        this->init_new_chart();

        for ( size_t n = 0; n < charts_list[prev_chart_ind].size(); n++ ) {

            int id = std::get<0> ( charts_list[prev_chart_ind][n] );
            std::string name = std::get<1> ( charts_list[prev_chart_ind][n] );

            auto vect = std::get<2> ( charts_list[prev_chart_ind][n] );

            if ( name.substr ( 0,1 ) == "W" ) {
                int p = name.find_last_of ( ":" );
                name = name.substr ( 0, p + 1 ) + val;
            }

            this->add_new_series ( chart_ind, id, name );
        }
    }

    chart_ind = prev_chart_ind;

    stackedWidget->setCurrentIndex(chart_ind);

    this->update_chart_labels();
}


void SmryAppl::make_group_charts ( std::string filt )
{
    auto group_list = make_group_list ( chart_ind, filt );

    int prev_chart_ind = chart_ind;

    for ( auto val : group_list ) {

        chart_ind ++;
        this->init_new_chart();

        for ( size_t n = 0; n < charts_list[prev_chart_ind].size(); n++ ) {

            int id = std::get<0> ( charts_list[prev_chart_ind][n] );
            std::string name = std::get<1> ( charts_list[prev_chart_ind][n] );

            auto vect = std::get<2> ( charts_list[prev_chart_ind][n] );

            if ( name.substr ( 0,1 ) == "G" ) {
                int p = name.find_last_of ( ":" );
                name = name.substr ( 0, p + 1 ) + val;
            }

            this->add_new_series ( chart_ind, id, name );
        }
    }

    chart_ind = prev_chart_ind;

    stackedWidget->setCurrentIndex(chart_ind);

    this->update_chart_labels();
}


void SmryAppl::make_aquifer_charts ( std::string filt )
{
    auto aquifer_list = make_aquifer_list ( chart_ind, filt );

    int prev_chart_ind = chart_ind;

    for ( auto val : aquifer_list ) {

        chart_ind ++;
        this->init_new_chart();

        for ( size_t n = 0; n < charts_list[prev_chart_ind].size(); n++ ) {

            int id = std::get<0> ( charts_list[prev_chart_ind][n] );
            std::string name = std::get<1> ( charts_list[prev_chart_ind][n] );

            auto vect = std::get<2> ( charts_list[prev_chart_ind][n] );

            if ( name.substr ( 0,1 ) == "A" ) {
                int p = name.find_last_of ( ":" );
                name = name.substr ( 0, p + 1 ) + val;
            }

            this->add_new_series ( chart_ind, id, name );
        }
    }

    chart_ind = prev_chart_ind;

    stackedWidget->setCurrentIndex(chart_ind);

    this->update_chart_labels();
}


bool SmryAppl::update_yrange ( const std::string& cmd_str )
{
    std::vector<std::string> str_tokens = split_string ( cmd_str );

    if ( str_tokens.size() < 3 )
        return false;

    int yaxis_ind;

    double min_val;
    double max_val;

    try {
        yaxis_ind = std::stoi ( str_tokens[0] );
        min_val = std::stod ( str_tokens[1] );
        max_val = std::stod ( str_tokens[2] );
    }

    catch ( ... ) {
        return false;
    }

    if ( min_val > max_val )
        return false;

    if ( ( yaxis_ind < 1 ) || ( yaxis_ind > axisY[chart_ind].size() ) )
        return false;

    int n_tick = -1;

    if ( str_tokens.size() > 3 ) {
        try {
            n_tick = std::stoi ( str_tokens[3] );
        }

        catch ( ... ) {
            return false;
        }
    }

    axisY[chart_ind][yaxis_ind - 1]->set_range ( min_val, max_val );

    if ( n_tick > 0 )
        axisY[chart_ind][yaxis_ind - 1]->setTickCount ( n_tick );

    return true;
}


float SmryAppl::calc_p90 ( const std::vector<float>& data )
{
    auto tmp = data;

    std::sort ( tmp.begin(), tmp.end() );
    size_t  p = static_cast<size_t> ( tmp.size() * 0.9 ) ;

    return tmp[p];
}


void SmryAppl::initColorAndStyle()
{
    linew = 2;


    QColor green;
    QColor sandy;
    QColor red;

    green.setRgb ( 21,171,0 );
    sandy.setRgb ( 244,164,96 );
    red.setRgb ( 255,64,35 );

    color_tab.push_back ( QColor ( "black" ) );
    color_tab.push_back ( green );
    color_tab.push_back ( red );
    color_tab.push_back ( QColor ( "blue" ) );
    color_tab.push_back ( QColor ( "cyan" ) );
    color_tab.push_back ( QColor ( "gray" ) );
    color_tab.push_back ( QColor ( "magenta" ) );
    color_tab.push_back ( QColor ( "violet" ) );
    color_tab.push_back ( QColor ( "darkBlue" ) );
    color_tab.push_back ( QColor ( "darkCyan" ) );
    color_tab.push_back ( QColor ( "darkMagenta" ) );

    //color_tab.push_back ( sandy );
    //color_tab.push_back ( QColor ( "violet" ) );

    style_tab.push_back ( Qt::SolidLine );
    style_tab.push_back ( Qt::DotLine );
    style_tab.push_back ( Qt::DotLine );
    style_tab.push_back ( Qt::DotLine );
    style_tab.push_back ( Qt::DotLine );
    style_tab.push_back ( Qt::DotLine );
    style_tab.push_back ( Qt::DotLine );
    style_tab.push_back ( Qt::DotLine );
    style_tab.push_back ( Qt::DotLine );
    style_tab.push_back ( Qt::DotLine );
    style_tab.push_back ( Qt::DotLine );

    //style_tab.push_back(Qt::DashLine);
    //style_tab.push_back(Qt::DashDotLine);
    //style_tab.push_back(Qt::DotDashLine);

}


void SmryAppl::add_cmd_to_hist(std::string cmd)
{

    auto it = std::find(command_hist.begin(), command_hist.end(), cmd);

    if (it == command_hist.end()){
        command_hist.insert(command_hist.begin(), cmd);

    } else {
        size_t n = std::distance(command_hist.begin(), it);

        while (n > 0 ){
            command_hist[n] = command_hist[n - 1];
            n--;
        }

        command_hist[0] = cmd;
    }

    cmd_hist_ind = -1;
}


void SmryAppl::reset_cmdline()
{
    smry_ind = 0;
    cmd_var = "";
    str_var = "";
    cmd_mode = false;

    le_commands->setText ( "" );
    lbl_rootn->setText ( QString::fromStdString ( root_name_list[smry_ind] ) );
}

void SmryAppl::copy_to_clipboard()
{
    QClipboard *clipboard = QGuiApplication::clipboard();
/*
    QRect rect = chart_view_list[chart_ind]->rect();
    int bw = 10;
    rect.adjust(bw + 2, bw + 2, -2*bw, -2*bw -1);
*/

    //QPixmap pixmap = chart_view_list[chart_ind]->grab(rect);
    QPixmap pixmap = chart_view_list[chart_ind]->grab();

    QImage image = pixmap.toImage();
    clipboard->setImage(image);
}

bool SmryAppl::eventFilter ( QObject *object, QEvent *event )
{

    if ( object == le_commands ) {
        if ( ( event->type() == QEvent::MouseButtonPress ) or
               ( event->type() == QEvent::MouseButtonPress ) or
                ( event->type() == QEvent::MouseButtonPress ) ) {

            return true;
        }
    }

    if ( event->type() == QEvent::KeyRelease ) {

        QKeyEvent *keyEvent = static_cast<QKeyEvent *> ( event );

        if ( keyEvent->key()  == Qt::Key_Alt )
            m_alt_key = false;

        if ( keyEvent->key()  == Qt::Key_Shift )
            m_shift_key = false;

        if ( keyEvent->key()  == Qt::Key_Control )
            m_ctrl_key = false;
    }

    if ( event->type() == QEvent::KeyPress ) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *> ( event );

        if ( keyEvent->key()  == Qt::Key_Alt )
            m_alt_key = true;

        if ( keyEvent->key()  == Qt::Key_Shift )
            m_shift_key = true;

        if ( keyEvent->key()  == Qt::Key_Control )
            m_ctrl_key = true;
    }


    if ( event->type() == QEvent::KeyPress ) {

        QKeyEvent *keyEvent = static_cast<QKeyEvent *> ( event );

        if ( keyEvent->key()  == Qt::Key_Delete  &&  m_ctrl_key  && !m_shift_key && !m_alt_key ) {

            if ( chartList.size() > 1 )
                this->delete_chart ( chart_ind );

            return true;

        } else if ( keyEvent->key()  == Qt::Key_Delete ) {

            this->handle_delete_series();

            return true;

        } else if (m_smry_loaded && (keyEvent->key() == Qt::Key_Home)) {
            // User pressed 'Home'.  Go to first chart in series.
            this->select_first_chart();

            return true;

        } else if (m_smry_loaded && (keyEvent->key() == Qt::Key_End)) {
            // User pressed 'End'.  Go to last chart in series.
            this->select_last_chart();

            return true;

        } else if ( keyEvent->key()  == Qt::Key_Z  &&  m_ctrl_key  && !m_shift_key && !m_alt_key ) {

            auto min_max_range = axisX[chart_ind]->get_xrange();
            update_all_yaxis(min_max_range, chart_ind, true);

            return true;

        } else if ( keyEvent->key()  == Qt::Key_X  &&  m_ctrl_key  && !m_shift_key && !m_alt_key ) {

            this->calc_min_xrange();

        } else if ( keyEvent->key() == Qt::Key_C  &&  m_ctrl_key  && !m_shift_key && !m_alt_key ) {

            this->copy_to_clipboard();
        }

        if ( cmd_mode ) {

            if ( keyEvent->key()  == Qt::Key_Up ) {

                if (cmd_hist_ind < (command_hist.size() -1)){
                    cmd_hist_ind++;
                    cmd_var = command_hist[cmd_hist_ind];
                    le_commands->setText (QString::fromStdString (cmd_var));
                }

                return true;
            }

            if ( keyEvent->key()  == Qt::Key_Down ) {

                if (cmd_hist_ind > 0){
                    cmd_hist_ind--;
                    cmd_var = command_hist[cmd_hist_ind];
                    le_commands->setText (QString::fromStdString (cmd_var));
                }

                return true;
            }

            return false;

        } else {

            if ( keyEvent->key()  == Qt::Key_Up ) {

                // activate command mode is arraw up and commans line string is empty

                if ((str_var  == "" ) && (command_hist.size() > 0)) {
                    cmd_mode = true;
                    cmd_hist_ind = 0 ;
                    cmd_var = command_hist[cmd_hist_ind];

                    le_commands->setText (QString::fromStdString (cmd_var));
                    lbl_rootn->setText ( "command mode" );

                    return true;
                }
            }
        }
    }


    if ( event->type() == QEvent::KeyPress ) {

        QKeyEvent *keyEvent = static_cast<QKeyEvent *> ( event );

        if ( keyEvent->key()  == Qt::Key_Return ) {

            return false;
        }
    }

    if ( event->type() == QEvent::KeyPress ) {

        QKeyEvent *keyEvent = static_cast<QKeyEvent *> ( event );

        if ( ( keyEvent->key() == Qt::Key_Colon ) && ( le_commands->text() == "" ) ) {

            QPalette *palette = new QPalette();
            palette->setColor ( QPalette::Text,Qt::blue );
            le_commands->setPalette ( *palette );

            cmd_var = ":";
            cmd_mode = true;

            lbl_rootn->setText ( "command mode" );

            le_commands->setText ( ":" );

            return true;

        } else if ( keyEvent->key() == Qt::Key_Space ) {

            if ( ( vect_ok ) && ( not axis_mode ) ) {
                axis_mode = true;
                acceptAutoComlete();

                QString test = le_commands->text() + " ";
                le_commands->setText ( test );

                return true;

            } else {

                return true;
            }
        }
    }

    if ( ( ens_mode ) && ( object == le_commands && event->type() == QEvent::KeyPress ) ) {

        QKeyEvent *keyEvent = static_cast<QKeyEvent *> ( event );

        std::vector<Qt::Key> num_keys = {Qt::Key_0, Qt::Key_1, Qt::Key_2, Qt::Key_3, Qt::Key_4, Qt::Key_5,
                                         Qt::Key_6, Qt::Key_7, Qt::Key_8, Qt::Key_9
                                        };

        std::vector<Qt::Key>::iterator it = std::find ( num_keys.begin(), num_keys.end(), keyEvent->key() );

        if ( it != num_keys.end() )
            if (str_var.size() == 0)
                return true;
    }


    if ( ( axis_mode ) && ( object == le_commands && event->type() == QEvent::KeyPress ) ) {

        QKeyEvent *keyEvent = static_cast<QKeyEvent *> ( event );

        std::vector<Qt::Key> num_keys = {Qt::Key_0, Qt::Key_1, Qt::Key_2, Qt::Key_3, Qt::Key_4, Qt::Key_5,
                                         Qt::Key_6, Qt::Key_7, Qt::Key_8, Qt::Key_9
                                        };

        std::vector<Qt::Key>::iterator it = std::find ( num_keys.begin(), num_keys.end(), keyEvent->key() );

        if ( it != num_keys.end() ) {

            int index = std::distance ( num_keys.begin(), it );

            if ( ( index > 0 ) && ( index < static_cast<int> ( axisY[chart_ind].size() + 2 ) ) ) {
                std::string cmd_string = le_commands->text().toStdString();
                cmd_string = cmd_string + std::to_string ( index );

                le_commands->setText ( QString::fromStdString ( cmd_string ) );
            }

            return true;

        } else if ( keyEvent->key() == Qt::Key_Backspace ) {

            std::string test = le_commands->text().toStdString();

            if ( is_number ( test.substr ( test.size()-1, 1 ) ) ) {
                le_commands->setText ( QString::fromStdString ( test.substr ( 0, test.size()-1 ) ) );
                return true;

            } else if ( test.substr ( test.size()-1, 1 ) == " " ) {
                axis_mode = false;
                return false;
            }

        } else if ( keyEvent->key() == Qt::Key_Return ) {

            return false;
        }
    }

    if ( object == le_commands && event->type() == QEvent::KeyPress ) {

        QKeyEvent *keyEvent = static_cast<QKeyEvent *> ( event );

        if ( ( cmd_line_error ) && ( keyEvent->key() != Qt::Key_Backspace ) ) {

            return true;

        } else if ( keyEvent->key() == Qt::Key_Tab ) {

            acceptAutoComlete();

            return true;

        } else if ( ( keyEvent->key() == Qt::Key_Shift )
                    or ( keyEvent->key() == Qt::Key_Right )
                    or ( keyEvent->key() == Qt::Key_Left ) ) {

            return true;

        } else if ( keyEvent->key() == Qt::Key_Backspace ) {

            if ( p2 == 0 ) {

                return true;
            }

            if ( str_var.substr ( str_var.size()-1 ) == ";" ) {

                le_commands->clear();
                p1 = 0;
                p2 = 0;
                smry_ind = 0;
                str_var = "";

                lbl_rootn->setText ( QString::fromStdString ( root_name_list[smry_ind] ) );

                cmd_line_error = false;

                return false;

            } else if ( str_var.substr ( str_var.size()-1 ) == ":" ) {

                cmd_mode = false;

                QPalette *palette = new QPalette();
                palette->setColor ( QPalette::Text,Qt::black );
                le_commands->setPalette ( *palette );

            } else {

                le_commands->setText ( QString::fromStdString ( str_var.substr ( 0,p2 ) ) );
                return false;
            }

        } else if ( keyEvent->key() == Qt::Key_Return ) {

            acceptAutoComlete();

            return false;

        } else if ( keyEvent->key() == Qt::Key_Down ) {

            if ( str_var.size() > 0 ) {

                vect_ind++;

                if ( vect_ind == vect_lookup_list.size() )
                    vect_ind--;

                modifiy_vect_lookup();
            }

            return true;

        } else if ( keyEvent->key() == Qt::Key_Up ) {

            if ( str_var.size() > 0 ) {

                if ( vect_ind > 0 )
                    vect_ind--;

                modifiy_vect_lookup();
            }

            return true;

        } else {

            le_commands->setText ( QString::fromStdString ( str_var.substr ( 0,p2 ) ) );
            return false;
        }
    }

    return false;
}

void SmryAppl::calc_min_xrange()
{
    auto chart_series = this->get_smry_series(chart_ind);

    if (chart_series.size() > 0) {

        auto min_max_dt = chart_series[0]->get_nonzero_range();

        QDateTime dt_min_x = std::get<0>(min_max_dt);
        QDateTime dt_max_x = std::get<1>(min_max_dt);

        for (size_t n = 1; n < chart_series.size(); n++) {

            min_max_dt = chart_series[0]->get_nonzero_range();

            if (std::get<0>(min_max_dt) < dt_min_x)
                dt_min_x =  std::get<0>(min_max_dt);

            if (std::get<1>(min_max_dt) > dt_max_x)
                dt_max_x =  std::get<1>(min_max_dt);
        }

        auto xaxis = this->get_smry_xaxis(chart_ind);

        xaxis->setRange(dt_min_x, dt_max_x);

        auto min_max_range = axisX[chart_ind]->get_xrange();
        update_all_yaxis(min_max_range, chart_ind);
    }
}


void SmryAppl::switch_markes()
{
    bool all_visible = true;

    for ( auto s : series[chart_ind] )
        if ( !s->pointsVisible() )
            all_visible = false;

    bool new_flag = all_visible ? false : true;

    for ( auto s : series[chart_ind] )
        s->setPointsVisible ( new_flag );
}

void SmryAppl::keyReleaseEvent(QKeyEvent *event)
{
    if ( event->key()  == Qt::Key_Alt )
        m_alt_key = false;

    if ( event->key()  == Qt::Key_Shift )
        m_shift_key = false;

    if ( event->key()  == Qt::Key_Control )
        m_ctrl_key = false;
}


void SmryAppl::keyPressEvent ( QKeyEvent *event )
{
    if ( event->key()  == Qt::Key_Alt )
        m_alt_key = true;

    if ( event->key()  == Qt::Key_Shift )
        m_shift_key = true;

    if ( event->key()  == Qt::Key_Control )
        m_ctrl_key = true;


    if (( event->key() == Qt::Key_Return ) && (m_smry_loaded)) {

        if ( cmd_mode ) {

            if ( ( cmd_var == ":r" ) || ( cmd_var == ":R" ) ) {

                bool res = this->reload_and_update_charts();

                this->add_cmd_to_hist(cmd_var);
                this->reset_cmdline();

            } else if ( cmd_var.substr ( 0,4 ) == ":pdf" ) {

                std::string current_path = QDir::currentPath().toStdString();
                std::string fileName = cmd_var.substr ( 4 );

                int p = fileName.find_first_not_of ( " " );
                fileName = current_path + "/" + fileName.substr ( p );

                QString qfile_name = QString::fromStdString ( fileName );

                this->print_pdf ( qfile_name );

                this->add_cmd_to_hist(cmd_var);
                this->reset_cmdline();

            } else if (( cmd_var.substr ( 0,7 ) == ":xrange" ) ||
                       ( cmd_var.substr ( 0,2 ) == ":x" ))   {

                if ( axisX[chart_ind]->set_range ( cmd_var ) ) {

                    auto min_max_range = axisX[chart_ind]->get_xrange();

                    update_all_yaxis(min_max_range, chart_ind);

                    this->add_cmd_to_hist(cmd_var);
                    this->reset_cmdline();

                } else {
                    lbl_rootn->setText ( "!Error, xrange command" );
                }

            } else if (( cmd_var.substr ( 0,7 ) == ":yrange" ) ||
                  ( cmd_var.substr ( 0,2 ) == ":y" )) {

                std::string yrange_cmd_str;

                if ( cmd_var.substr ( 0,7 ) == ":yrange" )
                    yrange_cmd_str = cmd_var.substr ( 8 );
                else
                    yrange_cmd_str = cmd_var.substr ( 3 );

                if ( this->update_yrange ( yrange_cmd_str ) ) {

                    this->add_cmd_to_hist(cmd_var);
                    this->reset_cmdline();

                } else {
                    lbl_rootn->setText ( "!Error, yrange command" );
                }

            } else if ( ( cmd_var.substr ( 0,2 ) == ":m" ) || ( cmd_var.substr ( 0,2 ) == ":M" ) ) {

                switch_markes();

                this->add_cmd_to_hist(cmd_var);
                this->reset_cmdline();

            } else if ( cmd_var.substr ( 0,2 ) == ":t" ) {

                std::cout << "<ctrl> + t \n";
                axisX[chart_ind]->print_ranges();

            } else if (( cmd_var.substr ( 0,12 ) == ":ignore zero" ) ||
                       ( cmd_var.substr ( 0,4 ) == ":i z" )){

                std::vector<std::string> str_tokens = split_string ( cmd_var );

                int from_chart = -1;
                int to_chart = -1;

                if ( str_tokens.size() > 3 ) {

                    try {
                        from_chart = std::stoi ( str_tokens[2] );
                        to_chart = std::stoi ( str_tokens[3] );
                    } catch ( ... ) {
                        std::cout << "invalid command, example :ignore zero 2 5 \n";
                    }

                } else {

                    std::cout << "invalid command, example :ignore zero 2 5 \n";
                }

                if ((from_chart != -1) && (to_chart >= from_chart)){

                    for (int c = from_chart -1; c < to_chart; c++){
                        auto min_max_range = axisX[c]->get_xrange();
                        update_all_yaxis(min_max_range, c, true);
                    }

                } else {
                    std::cout << "invalid command, example :ignore zero 2 5 \n";
                }

                this->add_cmd_to_hist(cmd_var);
                this->reset_cmdline();

            } else if ( cmd_var == ":test" ) {

                std::cout << "\ntesting \n";
                axisY[0][0]->print_axis_range();

                std::cout << "axis: min > " << axisY[0][0]->min();
                std::cout << " axis: max > " << axisY[0][0]->max();
                std::cout << "\n";

                this->add_cmd_to_hist(cmd_var);
                this->reset_cmdline();

            } else if ( ( cmd_var == ":ens" ) || ( cmd_var == ":ENS" ) ) {

                ens_mode = true;
                this->reset_cmdline();

                lbl_rootn->setText ( "Ensemble mode" );

            } else if ( ( cmd_var == ":e" ) || ( cmd_var == ":E" ) ) {

                QApplication::quit();

            } else {

                this->reset_cmdline();
            }

        } else {    // cmd_mode = false

            axis_mode = false;

            std::string vect_name = le_commands->text().toStdString();

            int pos = vect_name.find ( ";" );

            if ( pos > -1 )
                vect_name = vect_name.substr ( pos+1 );

            pos = vect_name.find ( " " );
            int axis_ind = -1;

            if ( pos > -1 ) {
                std::string axis_string = vect_name.substr ( pos + 1 );
                axis_ind = std::stod ( axis_string );
                vect_name = vect_name.substr ( 0, pos );
            }

            if ( axis_ind > ( static_cast<int> ( axisY[chart_ind].size() ) + 1 ) ) {
                throw std::invalid_argument ( "axis index larger that size axisY + 1" );
            }

            if ( ( vect_name.size() > 0 ) && ( vect_ok ) ) {

                if (ens_mode){
                    add_new_ens_series ( chart_ind, vect_name, axis_ind );
                } else {

                    //if (series[chart_ind].size() > 0)
                    //    axisX[chart_ind]->print_ranges();

                    std::tuple<QDateTime,QDateTime> current_xrange;
                    bool set_current = false;

                    if ((series[chart_ind].size() > 0) && (!axisX[chart_ind]->has_full_range())) {
                        current_xrange = axisX[chart_ind]->get_current_xrange();
                        set_current = true;
                        //auto from = std::get<0>(current_xrange);
                        //auto to = std::get<1>(current_xrange);
                        //std::cout << from.toString("yyyy-MM-dd HH:mm:ss.zzz").toStdString() << "\n";
                        //std::cout << to.toString("yyyy-MM-dd HH:mm:ss.zzz").toStdString() << "\n";
                    }

                    add_new_series ( chart_ind, smry_ind, vect_name, axis_ind );

                    update_full_xrange(chart_ind);

                    if (set_current)
                       axisX[chart_ind]->setRange(std::get<0>(current_xrange), std::get<1>(current_xrange));


                    auto min_max_range = axisX[chart_ind]->get_xrange();
                    update_all_yaxis(min_max_range, chart_ind);
                }

                le_commands->clear();

                p1 = 0;
                p2 = 0;
                vect_ind = 0;
                smry_ind = 0 ;

                lbl_num->setText ( QString::fromStdString ( "" ) );

                this->reset_cmdline();
            }
        }
    }

    else if ( event->key() == Qt::Key_O  &&  m_ctrl_key  && !m_shift_key && !m_alt_key ) {

        QString fileName = QFileDialog::getOpenFileName ( this, tr ( "Open Summary" ),
                           QDir::currentPath(), tr ( "Summary Files (*.SMSPEC *.ESMRY);;SMSPEC Files (*.SMSPEC);;ESMRY Files (*.ESMRY)" ) );

        if ( fileName.toStdString().size() > 0 ) {

            std::filesystem::path filename(fileName.toStdString());
            m_smry_files.push_back(filename);
            std::string ext = filename.extension().string();

            size_t smry_ind = m_file_type.size();

            auto ftime = std::filesystem::last_write_time ( m_smry_files[smry_ind] );
            file_stamp_vector.push_back ( ftime );

            if (ext == ".SMSPEC") {
                m_file_type.push_back(FileType::SMSPEC);

                try {
                    m_esmry_loader[smry_ind] = std::make_unique<Opm::EclIO::ESmry>(filename);
                } catch (...) {
                    std::string message = "Error with opening SMSPEC file " + filename.string();
                    throw std::runtime_error(message);
                }

                root_name_list.push_back ( m_esmry_loader[smry_ind]->rootname() );
                vect_list.push_back ( m_esmry_loader[smry_ind]->keywordList() );

            }  else if (ext == ".ESMRY") {
                m_file_type.push_back(FileType::ESMRY);

                try {
                    m_ext_esmry_loader[smry_ind] = std::make_unique<Opm::EclIO::ExtESmry>(filename);
                } catch (...) {
                    std::string message = "Error with opening ESMRY file " + filename.string();
                    throw std::runtime_error(message);
                }

                root_name_list.push_back ( m_ext_esmry_loader[smry_ind]->rootname() );
                vect_list.push_back ( m_ext_esmry_loader[smry_ind]->keywordList() );
            }

            if (!m_smry_loaded) {

                m_smry_loaded = true;
                lbl_plot->setText("new chart");

                le_commands->setEnabled(1);

                QPalette *palette = new QPalette();
                palette->setColor ( QPalette::Text,Qt::black );
                lbl_cmd->setPalette ( *palette );
            }


        }
    }

    else if (m_smry_loaded && event->key() == Qt::Key_R &&  m_ctrl_key  && !m_shift_key && !m_alt_key) {

        //chartList[chart_ind]->zoomReset();
        axisX[chart_ind]->resetAxisRange();

        auto min_max_range = axisX[chart_ind]->get_xrange();

        update_all_yaxis(min_max_range, chart_ind);
    }

    else if (m_smry_loaded && event->key() == Qt::Key_R &&  m_ctrl_key  && m_shift_key && !m_alt_key) {

        bool res = this->reload_and_update_charts();
    }

    else if ( m_smry_loaded && event->key() == Qt::Key_Z  &&  m_ctrl_key  && !m_shift_key && !m_alt_key ) {

        auto min_max_range = axisX[chart_ind]->get_xrange();
        update_all_yaxis(min_max_range, chart_ind, true);
    }

    else if ( m_smry_loaded && event->key() == Qt::Key_X  &&  m_ctrl_key  && !m_shift_key && !m_alt_key ) {

        this->calc_min_xrange();
    }


    else if ( m_smry_loaded && event->key() == Qt::Key_M  &&  m_ctrl_key  && !m_shift_key && !m_alt_key ) {

        switch_markes();
    }


    else if ( m_smry_loaded && event->key() == Qt::Key_C  &&  m_ctrl_key  && !m_shift_key && !m_alt_key ) {

        this->copy_to_clipboard();
    }

    else if (m_smry_loaded && event->key() == Qt::Key_PageDown ) {

        if ( ( chart_ind + 1 ) == chartList.size() ) {

            if (( series[chart_ind].size() > 0 ) || (ens_series[chart_ind].size() > 0)) {

                this->init_new_chart();

                chart_ind ++;
                stackedWidget->setCurrentIndex(chart_ind);
                this->update_chart_labels();

                lbl_rootn->setText ( QString::fromStdString ( root_name_list[smry_ind] ) );
                lbl_plot->setText ( "new chart" );
            }

        } else {

            chart_ind ++;
            stackedWidget->setCurrentIndex(chart_ind);

            this->update_chart_labels();
        }
    }

    else if (m_smry_loaded &&  event->key() == Qt::Key_PageUp ) {

        if ( chart_ind > 0 ) {
            chart_ind --;

            stackedWidget->setCurrentIndex(chart_ind);

            this->update_chart_labels();
        }
    }

    else if (m_smry_loaded && (event->key() == Qt::Key_Home)) {
        // User pressed 'Home'.  Go to first chart in series.
        this->select_first_chart();
    }

    else if (m_smry_loaded && (event->key() == Qt::Key_End)) {
        // User pressed 'End'.  Go to last chart in series.
        this->select_last_chart();
    }

    else if ( m_smry_loaded && event->key() == Qt::Key_F  &&  m_ctrl_key  && !m_shift_key && !m_alt_key ){

        this->export_figure("/project/multiscale/users/tskille/prog/test_data/tjohei.png", 0);
        std::cout << "figure exported\n";
    }

    else if ( m_smry_loaded && event->key() == Qt::Key_W  &&  m_ctrl_key  && !m_shift_key && !m_alt_key ){

        std::string filt = "*";

        bool ok;

        QString text = QInputDialog::getText ( this, tr ( "Well filter dialog" ),
                                               tr ( "filter value:" ), QLineEdit::Normal,
                                               QString::fromStdString ( filt ), &ok );
        if ( ok && !text.isEmpty() ) {

            this->make_well_charts ( text.toStdString() );
        }
    }

    else if ( m_smry_loaded && event->key() == Qt::Key_G  &&  m_ctrl_key  && !m_shift_key && !m_alt_key ){

        std::string filt = "*";

        bool ok;

        QString text = QInputDialog::getText ( this, tr ( "Group filter dialog" ),
                                               tr ( "filter value:" ), QLineEdit::Normal,
                                               QString::fromStdString ( filt ), &ok );
        if ( ok && !text.isEmpty() ) {

            this->make_group_charts ( text.toStdString() );
        }
    }

    else if ( m_smry_loaded && event->key() == Qt::Key_A  &&  m_ctrl_key  && !m_shift_key && !m_alt_key ){

        std::string filt = "*";

        bool ok;

        QString text = QInputDialog::getText ( this, tr ( "Aquifer filter dialog" ),
                                               tr ( "filter value:" ), QLineEdit::Normal,
                                               QString::fromStdString ( filt ), &ok );
        if ( ok && !text.isEmpty() ) {

            this->make_aquifer_charts ( text.toStdString() );
        }
    }

    else if ( m_smry_loaded && event->key() == Qt::Key_P  &&  m_ctrl_key  && !m_shift_key && !m_alt_key ){

        // The DontUseNativeDialog option is used to ensure that the widget-based implementation will
        // be used instead of the native dialog. Freeze when using nativeDialog on Linux RH7 distribution in Equinor.

        QString fileName = QFileDialog::getSaveFileName ( this, tr ( "Save File" ),
                           QDir::currentPath(),
                           tr ( "Pdf (*.pdf)" ) ,0 , QFileDialog::DontUseNativeDialog );

        this->print_pdf ( fileName );
    }


    else if ( m_smry_loaded && event->key() == Qt::Key_Delete  &&  m_ctrl_key  && !m_shift_key && !m_alt_key ){

        if ( chartList.size() > 1 )
            this->delete_chart ( chart_ind );
    }

    else if ((m_smry_loaded) &&  (( event->key() == Qt::Key_Delete ))) {

        this->handle_delete_series();
    }
}


void SmryAppl::handle_delete_series()
{
    if ( series[chart_ind].size() > 0 ) {

        delete_last_series();

        // tskille: This is causing an segmentation fault when
        // first rubberband zoom, then delete series
        // need something different
        // chart_view_list[chart_ind]->update_graphics();

    } else {

        if ( chartList.size() > 1 ) {
            this->delete_chart ( chart_ind );
        }
    }

}


void SmryAppl::command_modified ( const QString & txt )
{
    str_var = txt.toStdString();

    std::for_each ( str_var.begin(), str_var.end(), [] ( char & c ) {
        c = ::toupper ( c );
    } );

    std::string str1 = str_var.substr ( p1 );

    if ( cmd_mode ) {

        cmd_var = txt.toStdString();

        if ( cmd_var == "" )
            cmd_mode = false;

    } else if ( is_number ( str1 ) ) {

        le_commands->setText ( QString::fromStdString ( str1 + ";" ) );

        smry_ind = std::stoi ( str1 ) -1 ;

        str_var = str_var + ";";

        p1 = str_var.size();
        p2 = p1;

        if ( smry_ind >= root_name_list.size() ) {
            QPalette *palette = new QPalette();
            palette->setColor ( QPalette::Text,Qt::red );
            le_commands->setPalette ( *palette );
        } else {
            QPalette *palette = new QPalette();
            palette->setColor ( QPalette::Text,Qt::black );
            le_commands->setPalette ( *palette );
        }

    } else {

        p2 = str_var.size();;

        if ( p2 > 0 ) {

            vect_lookup_list.clear();

            for ( size_t n=0; n < vect_list[smry_ind].size(); n++ )
                if ( vect_list[smry_ind][n].substr ( 0, p2-p1 ) == str1 )
                    vect_lookup_list.push_back ( vect_list[smry_ind][n] );

            vect_ind = 0;

            std::string lbl_str = std::to_string ( vect_lookup_list.size() ) + " matches";
            lbl_num->setText ( QString::fromStdString ( lbl_str ) );

            if ( vect_lookup_list.size() > 0 ) {

                modifiy_vect_lookup();
                vect_ok = true;

            } else {

                vect_ok = false;

                QPalette *palette = new QPalette();
                palette->setColor ( QPalette::Text,Qt::red );
                le_commands->setPalette ( *palette );
            }
        }
    }

    if ( smry_ind >= root_name_list.size() ) {
        lbl_rootn->setText ( "Error !" );
        cmd_line_error = true;

    } else if ( cmd_mode ) {

        if ( cmd_var == ":r" )
            lbl_rootn->setText ( "re-load and update all series" );
        else if ( cmd_var.substr ( 0, 4 ) == ":pdf" )
            lbl_rootn->setText ( "export to pdf document" );
        else if ( cmd_var.substr ( 0, 7 ) == ":xrange" )
            lbl_rootn->setText ( "set range for xaxis" );
        else if ( cmd_var.substr ( 0, 7 ) == ":yrange" )
            lbl_rootn->setText ( "set range for yaxis >  axis min max [n_tick]" );
        else if ( cmd_var == ":e" )
            lbl_rootn->setText ( "set ensemble mode" );
        else if ( cmd_var == ":m" )
            lbl_rootn->setText ( "markers on/off" );
        else
            lbl_rootn->setText ( "??" );

    } else {

        if (not ens_mode)
            lbl_rootn->setText ( QString::fromStdString ( root_name_list[smry_ind] ) );

        cmd_line_error = false;
    }
}


void SmryAppl::modifiy_vect_lookup()
{
    size_t lstr = str_var.size();
    std::string pre = str_var.substr ( 0,p1 );
    le_commands->setText ( QString::fromStdString ( pre + vect_lookup_list[vect_ind] ) );

    le_commands->setSelection ( p1, lstr-p1 );

    QPalette *palette = new QPalette();
    palette->setColor ( QPalette::Text,Qt::black );
    le_commands->setPalette ( *palette );
}


bool SmryAppl::has_smry_vect(int smry_ind, const std::string& keystr)
{
    if (m_file_type[smry_ind] == FileType::SMSPEC)
        return m_esmry_loader[smry_ind]->hasKey ( keystr );

    else if (m_file_type[smry_ind] == FileType::ESMRY)
        return m_ext_esmry_loader[smry_ind]->hasKey ( keystr );

    throw std::runtime_error("unknown file type, can't be loaded");

}


const std::vector<float>& SmryAppl::get_smry_vect(int case_ind, std::string& keystr)
{
    switch( m_file_type[case_ind] ) {
    case FileType::SMSPEC:
        return m_esmry_loader[case_ind]->get(keystr);
        break;
    case FileType::ESMRY:
        return m_ext_esmry_loader[case_ind]->get(keystr);
        break;
    default:
        throw std::invalid_argument ( "invalid file type" );
    }
}


void SmryAppl::acceptAutoComlete()
{

    str_var = le_commands->text().toStdString();
    p2 = str_var.size();

    le_commands->deselect();
    le_commands->setCursorPosition ( p2 );
}


bool SmryAppl::is_number ( const std::string &s )
{
    return !s.empty() && std::all_of ( s.begin(), s.end(), ::isdigit );
}


void SmryAppl::export_figure(const std::string& fname, int chart_ind)
{
    this->grab().save(QString::fromStdString (fname));
}


int SmryAppl::max_vect_chart(input_list_type chart_input)
{
    int max_smry_ind = 0;

    for ( size_t c = 0; c < chart_input.size(); c++ ) {
        auto vect_input = std::get<0>(chart_input[c]);
        max_smry_ind = std::max(max_smry_ind, static_cast<int>(vect_input.size()));
    }

    return max_smry_ind;
}


void SmryAppl::print_pdf ( QString& fileName )
{
    QPdfWriter writer ( fileName );

    int current_chart_ind = chart_ind;

    //QFont m_font;
    //QFontMetrics metrics(m_font);

    //1400, 600

    // original QSizeF size1 ( 700, 1400 );
    QSizeF size1 ( 600, 1400 );
    QPageSize page1 ( size1, QPageSize::Unit::Point, "Custom" );

    writer.setPageSize ( page1 );

    writer.setPageOrientation ( QPageLayout::Landscape );

    QPainter painter ( &writer );

    for ( size_t n = 0; n < chartList.size(); n++ ) {
        if ( series[n].size() > 0 ) {

            if ( n > 0 )
                writer.newPage();

            chart_ind = n;

            stackedWidget->setCurrentIndex(chart_ind);

            chart_view_list[chart_ind]->render ( &painter );

            this->update_chart_labels();
        }
    }

    painter.end();

    chart_ind = current_chart_ind;
    stackedWidget->setCurrentIndex(chart_ind);
}


void SmryAppl::update_all_yaxis(const std::tuple<double, double>& min_max_range, int c_ind, bool ignore_zero)
{
    for (size_t y = 0; y < axisY[c_ind].size(); y ++) {

        double f_min_y = std::numeric_limits<double>::max();
        double f_max_y = -0.9 * std::numeric_limits<double>::max();

        double min_y = std::numeric_limits<double>::max();
        double max_y = -0.9 * std::numeric_limits<double>::max();

        for (size_t e = 0; e < series[c_ind].size(); e ++) {
            if (yaxis_map[series[c_ind][e]] == axisY[c_ind][y]) {

                double xfrom = std::get<0>(min_max_range);
                double xto = std::get<1>(min_max_range);

                auto yrange = series[c_ind][e]->get_min_max_value(xfrom, xto, ignore_zero);

                if (std::get<0>(yrange) < min_y)
                    min_y = std::get<0>(yrange);

                if (std::get<1>(yrange) > max_y)
                    max_y = std::get<1>(yrange);
            }
        }

        adjust_yaxis_props(axisY[c_ind][y], min_y, max_y);
        axisY[c_ind][y]-> setMinAndMax ( f_min_y, f_max_y );
    }
}

SmryYaxis* SmryAppl::get_smry_yaxis(int chart_ind, int axis_ind)
{
    return axisY[chart_ind][axis_ind];
}

std::vector<SmrySeries*> SmryAppl::get_smry_series(int chart_ind)
{
    return series[chart_ind];
}

void SmryAppl::update_full_xrange(int chart_index)
{
    auto xrange = series[chart_index][0]->get_min_max_xrange();
    double min_x = std::get<0>(xrange);
    double max_x = std::get<1>(xrange);

    for ( size_t i = 1; i < series[chart_index].size(); i++ ) {

        xrange = series[chart_index][i]->get_min_max_xrange();

        if (std::get<0>(xrange) < min_x)
            min_x = std::get<0>(xrange);

        if (std::get<1>(xrange) > max_x)
            max_x = std::get<1>(xrange);
    }

    axisX[chart_index]->set_full_range(min_x, max_x);
}

std::tuple<QDateTime, QDateTime> SmryAppl::calc_min_max_dt(int chart_ind)
{
    QDateTime min_dt;
    QDateTime max_dt;

    if (series[chart_ind].size() > 0){
        auto xrange = series[chart_ind][0]->get_min_max_dt_range();
        min_dt = std::get<0>(xrange);
        max_dt = std::get<1>(xrange);
    }

    for (size_t n = 1; n < series[chart_ind].size(); n++){
        auto xrange = series[chart_ind][n]->get_min_max_dt_range();
        if (std::get<0>(xrange) < min_dt)
            min_dt = std::get<0>(xrange);
        if (std::get<1>(xrange) > max_dt)
            max_dt = std::get<1>(xrange);
    }

    return std::make_tuple(min_dt, max_dt);
}


void SmryAppl::select_first_chart()
{
    this->chart_ind = 0;
    this->stackedWidget->setCurrentIndex(this->chart_ind);

    this->update_chart_labels();
}

void SmryAppl::select_last_chart()
{
    this->chart_ind = static_cast<int>(this->chartList.size()) - 1;
    stackedWidget->setCurrentIndex(this->chart_ind);

    this->update_chart_labels();
}
