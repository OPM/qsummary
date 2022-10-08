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

class TestQsummary: public QObject
{
    Q_OBJECT

private slots:

    void test_show_markers();
    void test_delete();
};

const int max_number_of_charts = 2000;


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

    QSum::add_vect_cmd_line("1FOPR", cmdline );  // will effectively be 1;FOPR
    QSum::add_vect_cmd_line("2FOPR", cmdline );  // will effectively be 2;FOPR

    QCOMPARE(window.number_of_charts(), 1);
    QCOMPARE(window.number_of_series(0), 2);

    auto smry_series = window.get_smry_series(0);

    QCOMPARE(smry_series.size(), 2);

    QCOMPARE(smry_series[0]->pointsVisible(), false);
    QCOMPARE(smry_series[1]->pointsVisible(), false);

    QTest::keyEvent(QTest::Click, cmdline, Qt::Key_M, Qt::ControlModifier);

    QCOMPARE(smry_series[0]->pointsVisible(), true);
    QCOMPARE(smry_series[1]->pointsVisible(), true);

    QSum::add_vect_cmd_line("3FOPR", cmdline );  // will effectively be 3;FOPR

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

    QSum::add_vect_cmd_line("1FOPR", cmdline );  // will effectively be 1;FOPR
    QSum::add_vect_cmd_line("2FOPR", cmdline );  // will effectively be 1;FOPR

    QTest::keyEvent(QTest::Click, cmdline, Qt::Key_PageDown);

    QSum::add_vect_cmd_line("2FWCT", cmdline );  // will effectively be 1;FOPR
    QSum::add_vect_cmd_line("3FWCT", cmdline );  // will effectively be 1;FOPR

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


QTEST_MAIN(TestQsummary)

#include "test_smry_appl.moc"
