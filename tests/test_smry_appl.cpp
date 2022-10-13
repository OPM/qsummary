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


#include <QtTest/QtTest>

#include <appl/smry_appl.hpp>
#include <tests/qsum_test_utility.hpp>
#include <appl/qsum_func_lib.hpp>

#include <chrono>
#include <thread>


class TestQsummary: public QObject
{
    Q_OBJECT

private slots:

    void test_show_markers();
    void test_delete();
    void test_scale_axis_vect_opt();
    void test_scale_axis_interactive_1();
    void test_scale_axis_interactive_2();
};

const int max_number_of_charts = 2000;

QDateTime make_date_from_ms(double mssepoc)
{
    QDate d1;
    QTime t1;

    d1.setDate(1970,1,1);
    t1.setHMS(0,0,0,0);

    QDateTime res_date;
    res_date.setTimeSpec(Qt::UTC);

    res_date.setDate(d1);
    res_date.setTime(t1);

    return res_date.addMSecs(static_cast<qint64>(mssepoc));
}

double make_ms_from_datetime(int y, int m, int d)
{
    QDate d1;
    QTime t1;

    d1.setDate(y,m,d);
    t1.setHMS(0,0,0,0);

    QDateTime res_date;
    res_date.setTimeSpec(Qt::UTC);

    res_date.setDate(d1);
    res_date.setTime(t1);

    return res_date.toMSecsSinceEpoch();
}

QRectF make_rect(QRectF plota, SmryXaxis* xaxis, SmryYaxis* yaxis,
                 const QDateTime& zoom_x_from, const QDateTime& zoom_x_to, double y_from, double y_to)
{
    auto xrange = xaxis->get_xrange();

    int y1 = zoom_x_from.date().year();
    int m1 = zoom_x_from.date().month();
    int d1 = zoom_x_from.date().day();

    int y2 = zoom_x_to.date().year();
    int m2 = zoom_x_to.date().month();
    int d2 = zoom_x_to.date().day();

    double ms_from = make_ms_from_datetime(y1, m1, d1);
    double ms_to = make_ms_from_datetime(y2, m2, d2);

    double ms_xrange_from = std::get<0>(xrange);
    double ms_xrange_to = std::get<1>(xrange);

    double ya_min = static_cast<double>(yaxis->min());
    double ya_max = static_cast<double>(yaxis->max());

    double x = plota.left() + (ms_from-ms_xrange_from)/(ms_xrange_to-ms_xrange_from)*plota.width();
    double w = plota.left() + (ms_to-ms_xrange_from)/(ms_xrange_to-ms_xrange_from)*plota.width() - x;
    double y = plota.bottom() - (y_to - ya_min)/(ya_max - ya_min)*plota.height();
    double h = plota.bottom() - (y_from - ya_min)/(ya_max - ya_min)*plota.height() - y;

    QRectF rect(x,y,w,h);

    return rect;
}

bool chk_date_range(SmryXaxis* xaxis, int y1, int m1, int d1, int y2, int m2, int d2)
{
    auto xaxis_range = xaxis ->get_xrange();

    QDateTime dt_from = make_date_from_ms(std::get<0>(xaxis_range));
    QDateTime dt_to = make_date_from_ms(std::get<1>(xaxis_range));

    if (dt_from.date().year() != y1)
        return false;

    if (dt_from.date().month() != m1)
        return false;

    if (dt_from.date().day() != d1)
        return false;

    if (dt_to.date().year() != y2)
        return false;

    if (dt_to.date().month() != m2)
        return false;

    if (dt_to.date().day() != d2)
        return false;

    return true;
}


bool chk_yaxis_val_range(SmryYaxis* yaxis, float y_from, float y_to)
{
    float a_min = static_cast<float>(yaxis->min());
    float a_max = static_cast<float>(yaxis->max());

    a_min = std::round(a_min*10.0) / 10.0;
    a_max = std::round(a_max*10.0) / 10.0;

    if (a_min != y_from)
        return false;

    if (a_max != y_to)
        return false;

    return true;
}



void smry_input(const std::vector<std::string>& fname_list,
                const std::vector<FileType>& file_type,
                std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>>& esmry_loader,
                std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>>& ext_smry_loader,
                std::vector<std::filesystem::path>& smry_files)

{
    int num_files = fname_list.size();

    for (size_t n = 0; n < num_files; n++)
        if (file_type[n] == FileType::ESMRY)
            ext_smry_loader[n] = std::make_unique<Opm::EclIO::ExtESmry>(fname_list[n]);
        else if (file_type[n] == FileType::SMSPEC)
            esmry_loader[n] = std::make_unique<Opm::EclIO::ESmry>(fname_list[n]);
        else
            throw std::runtime_error("file type not set correctly");

    for (size_t n = 0; n < num_files; n++)
        smry_files.push_back( std::filesystem::path(fname_list[n]) );
}


void TestQsummary::test_show_markers()
{
    SmryAppl::input_list_type input_charts;
    SmryAppl::loader_list_type loaders;

    int num_files = 3;

    std::vector<std::string> fname_list;
    std::vector<FileType> file_type;

    file_type.resize(num_files);
    fname_list.resize(num_files);

    fname_list[0] = "../tests/smry_files/SENS0.ESMRY";
    fname_list[1] = "../tests/smry_files/SENS1.SMSPEC";
    fname_list[2] = "../tests/smry_files/SENS2.ESMRY";

    file_type = { FileType::ESMRY, FileType::SMSPEC, FileType::ESMRY };

    std::string smry_vect = "";
    std::string xrange_str = "";

    // -----

    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>> esmry_loader;
    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>> ext_smry_loader;
    std::vector<std::filesystem::path> smry_files;

    // set up loaders and smry file system paths
    smry_input(fname_list, file_type, esmry_loader, ext_smry_loader, smry_files);

    // set up input_charts data type
    QSum::chart_input_from_string(smry_vect, input_charts, file_type, esmry_loader, ext_smry_loader, max_number_of_charts, xrange_str);

    // make loaders for smryAppl
    loaders = std::make_tuple(smry_files, file_type, std::move(esmry_loader), std::move(ext_smry_loader));

    // derived summary object for smryAP
    std::unique_ptr<DerivedSmry> derived_smry;

    SmryAppl window(fname_list, loaders, input_charts, derived_smry);

    window.resize(1400, 700);

    QLineEdit* cmdline = window.get_cmdline();

    QSum::add_cmd_line("1FOPR", cmdline );  // will effectively be 1;FOPR
    QSum::add_cmd_line("2FOPR", cmdline );  // will effectively be 2;FOPR

    QCOMPARE(window.number_of_charts(), 1);
    QCOMPARE(window.number_of_series(0), 2);

    auto smry_series = window.get_smry_series(0);

    QCOMPARE(smry_series.size(), 2);

    QCOMPARE(smry_series[0]->pointsVisible(), false);
    QCOMPARE(smry_series[1]->pointsVisible(), false);

    QTest::keyEvent(QTest::Click, cmdline, Qt::Key_M, Qt::ControlModifier);

    QCOMPARE(smry_series[0]->pointsVisible(), true);
    QCOMPARE(smry_series[1]->pointsVisible(), true);

    QSum::add_cmd_line("3FOPR", cmdline );  // will effectively be 3;FOPR

    smry_series = window.get_smry_series(0);
    QCOMPARE(smry_series.size(), 3);

    QCOMPARE(smry_series[0]->pointsVisible(), true);
    QCOMPARE(smry_series[1]->pointsVisible(), true);
    QCOMPARE(smry_series[2]->pointsVisible(), false);

    QTest::keyEvent(QTest::Click, cmdline, Qt::Key_M, Qt::ControlModifier);

    QCOMPARE(smry_series[0]->pointsVisible(), true);
    QCOMPARE(smry_series[1]->pointsVisible(), true);
    QCOMPARE(smry_series[2]->pointsVisible(), true);

    QTest::keyEvent(QTest::Click, cmdline, Qt::Key_M, Qt::ControlModifier);

    QCOMPARE(smry_series[0]->pointsVisible(), false);
    QCOMPARE(smry_series[1]->pointsVisible(), false);
    QCOMPARE(smry_series[2]->pointsVisible(), false);

    //window.grab().save("tjohei.png");
}


void TestQsummary::test_delete()
{
    SmryAppl::input_list_type input_charts;
    SmryAppl::loader_list_type loaders;

    int num_files = 3;

    std::vector<std::string> fname_list;
    std::vector<FileType> file_type;

    file_type.resize(num_files);
    fname_list.resize(num_files);

    fname_list[0] = "../tests/smry_files/SENS0.ESMRY";
    fname_list[1] = "../tests/smry_files/SENS1.SMSPEC";
    fname_list[2] = "../tests/smry_files/SENS2.ESMRY";

    file_type = { FileType::ESMRY, FileType::SMSPEC, FileType::ESMRY };

    std::string smry_vect = "FOPR,WOPR:*";
    std::string xrange_str = "";

    // -----

    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>> esmry_loader;
    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>> ext_smry_loader;
    std::vector<std::filesystem::path> smry_files;

    // set up loaders and smry file system paths
    smry_input(fname_list, file_type, esmry_loader, ext_smry_loader, smry_files);

    // set up input_charts data type
    QSum::chart_input_from_string(smry_vect, input_charts, file_type, esmry_loader, ext_smry_loader, max_number_of_charts, xrange_str);

    // make loaders for smryAppl
    loaders = std::make_tuple(smry_files, file_type, std::move(esmry_loader), std::move(ext_smry_loader));

    // derived summary object for smryAP
    std::unique_ptr<DerivedSmry> derived_smry;

    SmryAppl window(fname_list, loaders, input_charts, derived_smry);

    window.resize(1400, 700);

    // created 5 charts with 3 series on each
    QCOMPARE(window.number_of_charts(), 5);

    // test with command line not active, eventfilter for le_commands (QLineEdit) will not be used

    for (size_t n = 0; n < 5; n++)
        QCOMPARE(window.number_of_series(n), 3);

    QTest::keyEvent(QTest::Click, &window, Qt::Key_Delete);

    QCOMPARE(window.number_of_series(0), 2);

    QTest::keyEvent(QTest::Click, &window, Qt::Key_Delete);
    QTest::keyEvent(QTest::Click, &window, Qt::Key_Delete);

    QCOMPARE(window.number_of_series(0), 0);

    QTest::keyEvent(QTest::Click, &window, Qt::Key_Delete);

    QCOMPARE(window.number_of_charts(), 4);

    for (size_t n = 0; n < 4; n++)
        QCOMPARE(window.number_of_series(n), 3);

    QTest::keyEvent(QTest::Click, &window, Qt::Key_Delete, Qt::ControlModifier);

    QCOMPARE(window.number_of_charts(), 3);

    QLineEdit* cmdline = window.get_cmdline();

    QTest::keyEvent(QTest::Click, cmdline, Qt::Key_Delete);

    QCOMPARE(window.number_of_charts(), 3);
    QCOMPARE(window.number_of_series(0), 2);

    QTest::keyEvent(QTest::Click, cmdline, Qt::Key_Delete);
    QTest::keyEvent(QTest::Click, cmdline, Qt::Key_Delete);

    QCOMPARE(window.number_of_series(0), 0);

    QTest::keyEvent(QTest::Click, cmdline, Qt::Key_Delete);

    QCOMPARE(window.number_of_charts(), 2);

    for (size_t n = 0; n < 2; n++)
        QCOMPARE(window.number_of_series(n), 3);

    QTest::keyEvent(QTest::Click, cmdline, Qt::Key_Delete, Qt::ControlModifier);

    QCOMPARE(window.number_of_charts(), 1);
    QCOMPARE(window.number_of_series(0), 3);

    // Tries to delete last chart, should not be possible, hence event should be ignored

    QTest::keyEvent(QTest::Click, cmdline, Qt::Key_Delete, Qt::ControlModifier);

    QCOMPARE(window.number_of_charts(), 1);
    QCOMPARE(window.number_of_series(0), 3);

    QTest::keyEvent(QTest::Click, cmdline, Qt::Key_Delete);
    QTest::keyEvent(QTest::Click, cmdline, Qt::Key_Delete);
    QTest::keyEvent(QTest::Click, cmdline, Qt::Key_Delete);

    QCOMPARE(window.number_of_charts(), 1);
    QCOMPARE(window.number_of_series(0), 0);

    // Tries to delete last chart, should not be possible, hence event should be ignored

    QTest::keyEvent(QTest::Click, cmdline, Qt::Key_Delete);

    QCOMPARE(window.number_of_charts(), 1);
    QCOMPARE(window.number_of_series(0), 0);

    // create two new charts with 2 series on both

    QSum::add_cmd_line("1FOPR", cmdline );  // will effectively be 1;FOPR
    QSum::add_cmd_line("2FOPR", cmdline );  // will effectively be 1;FOPR

    QTest::keyEvent(QTest::Click, cmdline, Qt::Key_PageDown);

    QSum::add_cmd_line("2FWCT", cmdline );  // will effectively be 1;FOPR
    QSum::add_cmd_line("3FWCT", cmdline );  // will effectively be 1;FOPR

    QCOMPARE(window.number_of_charts(), 2);
    QCOMPARE(window.number_of_series(0), 2);
    QCOMPARE(window.number_of_series(1), 2);

    // test with command line active, eventfilter for le_commands (QLineEdit) will be used

    QTest::keyEvent(QTest::Click, &window, Qt::Key_Delete, Qt::ControlModifier);

    QCOMPARE(window.number_of_charts(), 1);

    // Tries to delete last chart, should not be possible, hence event should be ignored

    QTest::keyEvent(QTest::Click, &window, Qt::Key_Delete, Qt::ControlModifier);

    QCOMPARE(window.number_of_charts(), 1);
    QCOMPARE(window.number_of_series(0), 2);

    QTest::keyEvent(QTest::Click, cmdline, Qt::Key_Delete);
    QTest::keyEvent(QTest::Click, cmdline, Qt::Key_Delete);

    QCOMPARE(window.number_of_charts(), 1);
    QCOMPARE(window.number_of_series(0), 0);

    // Tries to delete last chart, should not be possible, hence event should be ignored

    QTest::keyEvent(QTest::Click, cmdline, Qt::Key_Delete);

    QCOMPARE(window.number_of_charts(), 1);
    QCOMPARE(window.number_of_series(0), 0);

    //window.grab().save("tjohei.png");
}


void TestQsummary::test_scale_axis_vect_opt()
{
    SmryAppl::input_list_type input_charts;
    SmryAppl::loader_list_type loaders;

    int num_files = 3;

    std::vector<std::string> fname_list;
    std::vector<FileType> file_type;

    file_type.resize(num_files);
    fname_list.resize(num_files);

    fname_list[0] = "../tests/smry_files/NORNE_ATW2013.ESMRY";
    fname_list[1] = "../tests/smry_files/NORNE_S1.ESMRY";
    fname_list[2] = "../tests/smry_files/NORNE_S2.ESMRY";

    file_type = { FileType::ESMRY, FileType::ESMRY, FileType::ESMRY };

    std::string smry_vect = "FOPR";
    std::string xrange_str = "2002 2003";

    // -----

    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>> esmry_loader;
    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>> ext_smry_loader;
    std::vector<std::filesystem::path> smry_files;

    // set up loaders and smry file system paths
    smry_input(fname_list, file_type, esmry_loader, ext_smry_loader, smry_files);

    // set up input_charts data type
    QSum::chart_input_from_string(smry_vect, input_charts, file_type, esmry_loader, ext_smry_loader, max_number_of_charts, xrange_str);

    // make loaders for smryAppl
    loaders = std::make_tuple(smry_files, file_type, std::move(esmry_loader), std::move(ext_smry_loader));

    // derived summary object for smryAP
    std::unique_ptr<DerivedSmry> derived_smry;

    SmryAppl window(fname_list, loaders, input_charts, derived_smry);

    window.resize(1400, 700);

    // zoom in with rectangle, typical user with rubberband zoop
    // tskille failse without windows.grab().... ???
    // needs this before using zoomIn with retangle
    // https://doc.qt.io/qt-6/qwidget.html#grab

    window.grab();

    QCOMPARE(window.number_of_charts(), 1);

    QtCharts::QChart* chart = window.get_chart(0);

    QRectF plota = chart->plotArea();

    SmryXaxis* xaxis = window.get_smry_xaxis(0);
    SmryYaxis* yaxis = window.get_smry_yaxis(0, 0);

    QDateTime x_from;
    QDateTime x_to;
    x_from.setTimeSpec(Qt::UTC);
    x_to.setTimeSpec(Qt::UTC);

    x_from.setDate({2002,3,31});
    x_from.setTime({0,0,0,0});

    x_to.setDate({2002,5,13});
    x_to.setTime({0,0,0,0});

    QRectF zoom_rect = make_rect(plota, xaxis, yaxis, x_from, x_to, 15, 32);

    chart->zoomIn(zoom_rect);

    QCOMPARE(xaxis->has_xrange(), true);

    QCOMPARE(chk_date_range(xaxis, 2002, 3, 31, 2002, 5, 13), true);
    QCOMPARE(chk_yaxis_val_range(yaxis, 15.0, 32.0), true);

    // Use <ctrl> + R to reset zoom. Since this chart has xrange set and
    // current zoom area are within this xrange, new range should be xrange

    QTest::keyEvent(QTest::Click, &window, Qt::Key_R, Qt::ControlModifier);

    QCOMPARE(chk_date_range(xaxis, 2002, 1, 1, 2003, 1, 1), true);
    QCOMPARE(chk_yaxis_val_range(yaxis, 5.0, 40.0), true);

    // Use <ctrl> + R to reset zoom again. This time to the full xrange

    QTest::keyEvent(QTest::Click, &window, Qt::Key_R, Qt::ControlModifier);

    QCOMPARE(chk_date_range(xaxis, 1997, 8, 2, 2006, 12, 1), true);
    QCOMPARE(chk_yaxis_val_range(yaxis, 0.0, 48.0), true);

    // Use <ctrl> + R to reset zoom. Since this chart has xrange set and
    // current range is full range, new range should be xrange

    QTest::keyEvent(QTest::Click, &window, Qt::Key_R, Qt::ControlModifier);

    QCOMPARE(chk_date_range(xaxis, 2002, 1, 1, 2003, 1, 1), true);
    QCOMPARE(chk_yaxis_val_range(yaxis, 5.0, 40.0), true);
}


void TestQsummary::test_scale_axis_interactive_1()
{
    SmryAppl::input_list_type input_charts;
    SmryAppl::loader_list_type loaders;

    int num_files = 3;

    std::vector<std::string> fname_list;
    std::vector<FileType> file_type;

    file_type.resize(num_files);
    fname_list.resize(num_files);

    fname_list[0] = "../tests/smry_files/NORNE_ATW2013.ESMRY";
    fname_list[1] = "../tests/smry_files/NORNE_S1.ESMRY";
    fname_list[2] = "../tests/smry_files/NORNE_S2.ESMRY";

    file_type = { FileType::ESMRY, FileType::ESMRY, FileType::ESMRY };

    std::string smry_vect = "";
    std::string xrange_str = "";

    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>> esmry_loader;
    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>> ext_smry_loader;
    std::vector<std::filesystem::path> smry_files;

    // set up loaders and smry file system paths
    smry_input(fname_list, file_type, esmry_loader, ext_smry_loader, smry_files);

    // set up input_charts data type
    //QSum::chart_input_from_string(smry_vect, input_charts, file_type, esmry_loader, ext_smry_loader, max_number_of_charts, xrange_str);

    // make loaders for smryAppl
    loaders = std::make_tuple(smry_files, file_type, std::move(esmry_loader), std::move(ext_smry_loader));

    // derived summary object for smryAP
    std::unique_ptr<DerivedSmry> derived_smry;

    SmryAppl window(fname_list, loaders, input_charts, derived_smry);

    window.resize(1400, 700);

    // zoom in with rectangle, typical user with rubberband zoop
    // tskille failse without windows.grab().... ???
    // needs this before using zoomIn with retangle
    // https://doc.qt.io/qt-6/qwidget.html#grab

    window.grab();

    QLineEdit* cmdline = window.get_cmdline();

    QSum::add_cmd_line("1FOPR", cmdline );
    QSum::add_cmd_line("2FOPR", cmdline );

    QSum::add_cmd_line(":xrange 2002 2003", cmdline );

    SmryXaxis* xaxis = window.get_smry_xaxis(0);
    SmryYaxis* yaxis = window.get_smry_yaxis(0, 0);

    QCOMPARE(xaxis->has_xrange(), true);

    QtCharts::QChart* chart = window.get_chart(0);

    QRectF plota = chart->plotArea();

    QDateTime x_from;
    QDateTime x_to;
    x_from.setTimeSpec(Qt::UTC);
    x_to.setTimeSpec(Qt::UTC);

    x_from.setDate({2002,3,31});
    x_from.setTime({0,0,0,0});

    x_to.setDate({2002,5,13});
    x_to.setTime({0,0,0,0});

    QRectF zoom_rect = make_rect(plota, xaxis, yaxis, x_from, x_to, 15, 32);

    chart->zoomIn(zoom_rect);

    QCOMPARE(xaxis->has_xrange(), true);

    QSum::add_cmd_line("3FOPR", cmdline );  // will effectively be 2;FOPR

    QCOMPARE(xaxis->has_xrange(), true);

    //auto xaxis_range = xaxis ->get_xrange();

    //QDateTime dt_from = make_date_from_ms(std::get<0>(xaxis_range));
    //QDateTime dt_to = make_date_from_ms(std::get<1>(xaxis_range));

    QCOMPARE(chk_date_range(xaxis, 2002, 3, 31, 2002, 5, 13), true);
    QCOMPARE(chk_yaxis_val_range(yaxis, 16.0, 32.0), true);

    // Use <ctrl> + R to reset zoom. Since this chart has xrange set and
    // current zoom area are within this xrange, new range should be xrange

    QTest::keyEvent(QTest::Click, &window, Qt::Key_R, Qt::ControlModifier);

    QCOMPARE(chk_date_range(xaxis, 2002, 1, 1, 2003, 1, 1), true);
    QCOMPARE(chk_yaxis_val_range(yaxis, 5.0, 40.0), true);

    // Use <ctrl> + R to reset zoom again. This time to the full xrange

    QTest::keyEvent(QTest::Click, &window, Qt::Key_R, Qt::ControlModifier);

    QCOMPARE(chk_date_range(xaxis, 1997, 8, 2, 2006, 12, 1), true);
    QCOMPARE(chk_yaxis_val_range(yaxis, 0.0, 48.0), true);

    //window.grab().save("tjohei.png");


}

void TestQsummary::test_scale_axis_interactive_2()
{
    SmryAppl::input_list_type input_charts;
    SmryAppl::loader_list_type loaders;

    int num_files = 3;

    std::vector<std::string> fname_list;
    std::vector<FileType> file_type;

    file_type.resize(num_files);
    fname_list.resize(num_files);

    fname_list[0] = "../tests/smry_files/NORNE_ATW2013.ESMRY";
    fname_list[1] = "../tests/smry_files/NORNE_S1.ESMRY";
    fname_list[2] = "../tests/smry_files/NORNE_S2.ESMRY";

    file_type = { FileType::ESMRY, FileType::ESMRY, FileType::ESMRY };

    std::string smry_vect = "";
    std::string xrange_str = "";

    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>> esmry_loader;
    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>> ext_smry_loader;
    std::vector<std::filesystem::path> smry_files;

    // set up loaders and smry file system paths
    smry_input(fname_list, file_type, esmry_loader, ext_smry_loader, smry_files);

    // set up input_charts data type
    //QSum::chart_input_from_string(smry_vect, input_charts, file_type, esmry_loader, ext_smry_loader, max_number_of_charts, xrange_str);

    // make loaders for smryAppl
    loaders = std::make_tuple(smry_files, file_type, std::move(esmry_loader), std::move(ext_smry_loader));

    // derived summary object for smryAP
    std::unique_ptr<DerivedSmry> derived_smry;

    SmryAppl window(fname_list, loaders, input_charts, derived_smry);

    window.resize(1400, 700);

    // zoom in with rectangle, typical user with rubberband zoop
    // tskille failse without windows.grab().... ???
    // needs this before using zoomIn with retangle
    // https://doc.qt.io/qt-6/qwidget.html#grab

    window.grab();

    QLineEdit* cmdline = window.get_cmdline();

    QSum::add_cmd_line("1FPR", cmdline );
    QSum::add_cmd_line("2FPR", cmdline );

    QSum::add_cmd_line(":xrange 1999 2002", cmdline );

    SmryXaxis* xaxis = window.get_smry_xaxis(0);
    SmryYaxis* yaxis = window.get_smry_yaxis(0, 0);

    QCOMPARE(xaxis->has_xrange(), true);

    QtCharts::QChart* chart = window.get_chart(0);

    QRectF plota = chart->plotArea();

    QDateTime x_from;
    QDateTime x_to;
    x_from.setTimeSpec(Qt::UTC);
    x_to.setTimeSpec(Qt::UTC);

    x_from.setDate({1999,2,1});
    x_from.setTime({0,0,0,0});

    x_to.setDate({1999,10,1});
    x_to.setTime({0,0,0,0});

    //QRectF zoom_rect = make_rect(plota, xaxis, yaxis, x_from, x_to, 15, 32);
    QRectF zoom_rect = make_rect(plota, xaxis, yaxis, x_from, x_to, 236.9, 243.5);

    chart->zoomIn(zoom_rect);

    QCOMPARE(chk_date_range(xaxis, 1999, 2, 1, 1999, 10, 1), true);
    QCOMPARE(chk_yaxis_val_range(yaxis, 236.9, 243.5), true);

    QSum::add_cmd_line("3FPR", cmdline );

    QCOMPARE(chk_date_range(xaxis, 1999, 2, 1, 1999, 10, 1), true);
    QCOMPARE(chk_yaxis_val_range(yaxis, 236.0, 252.0), true);

    // deleteing 3FPR need rescale yaxis
    QTest::keyEvent(QTest::Click, &window, Qt::Key_Delete);

    QCOMPARE(chk_date_range(xaxis, 1999, 2, 1, 1999, 10, 1), true);
    QCOMPARE(chk_yaxis_val_range(yaxis, 238.0, 240.8), true);

    //window.grab().save("tmp.png");
}

QTEST_MAIN(TestQsummary)

#include "test_smry_appl.moc"
