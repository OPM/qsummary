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
#include <iostream>

#include <opm/io/eclipse/ESmry.hpp>
#include <opm/io/eclipse/ExtESmry.hpp>

#include <opm/io/eclipse/EclFile.hpp>

#include <appl/qsum_cmdf.hpp>


class TestQsummary: public QObject
{
    Q_OBJECT

private slots:

   void test_1a();
   void test_1c();
   void test_1f();
   void test_1h();
   void test_2a();
   void test_2b();
   void test_2c();
   void test_3a();
   void test_3b();
   void test_3c();
};

// https://doc.qt.io/qt-6/qtest-tutorial.html

// ctest -V for verbose output

// smry_id, name and unit
//using var_type = std::tuple<int, std::string, std::string>;

// name, smry_id and smry_key
//using param_type = std::tuple<std::string, int, std::string>;
//using param_list_type = std::vector<param_type>;

// var, params and expression
//using define_type = std::tuple<var_type, param_list_type, std::string>;

void print_input_charts(const SmryAppl::input_list_type& input_charts)
{

    std::cout << "number of charts: " << input_charts.size() << std::endl;

    for ( size_t c = 0; c < input_charts.size(); c++ ) {

        std::cout << "chart : " << c << std::endl;

        std::vector<SmryAppl::vect_input_type> vect_input;
        vect_input = std::get<0>(input_charts[c]);

        for ( size_t i=0; i < vect_input.size(); i++ ) {
            int n = std::get<0> ( vect_input[i] );
            std::string vect_name = std::get<1> ( vect_input[i] );
            int axis = std::get<2> ( vect_input[i] );
            bool is_derived = std::get<3> ( vect_input[i] );

            std::cout << "smry_ind= " << n << " > vect_name: " << vect_name << "  axis: ";
            std::cout << axis << " is derived: " << std::boolalpha << is_derived << std::endl;
        }
    }
}



void TestQsummary::test_1a()
{
    int num_files = 3;

    std::vector<FileType> file_type;
    file_type.resize(num_files);

    file_type[0] = FileType::SMSPEC;
    file_type[1] = FileType::ESMRY;
    file_type[2] = FileType::ESMRY;

    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>> esmry_loader;
    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>> lodsmry_loader;

    esmry_loader[0] = std::make_unique<Opm::EclIO::ESmry>("../tests/smry_files/SENS0.SMSPEC");
    lodsmry_loader[1] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS1.ESMRY");
    lodsmry_loader[2] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS2.ESMRY");

    {
        std::string cmd_file = "../tests/cmd_files/test1a.txt";
        QsumCMDF cmdfile(cmd_file, num_files, "");

        SmryAppl::input_list_type input_charts;

        cmdfile.make_charts_from_cmd(input_charts, "");

        //print_input_charts(input_charts);

        QCOMPARE(input_charts.size(), 1);

        std::vector<SmryAppl::vect_input_type> ref_vect_input;

        ref_vect_input.push_back({0, "WOPR:PROD-1", -1, false});
        ref_vect_input.push_back({1, "WOPR:PROD-1", -1, false});
        ref_vect_input.push_back({2, "WOPR:PROD-1", -1, false});
        ref_vect_input.push_back({0, "WXX1:PROD-1", -1, true});

        std::vector<SmryAppl::vect_input_type> test_vect_input;
        test_vect_input = std::get<0>(input_charts[0]);

        QCOMPARE(test_vect_input.size(), ref_vect_input.size());

        for (size_t n = 0; n < test_vect_input.size(); n++)
            QCOMPARE(test_vect_input[n], ref_vect_input[n]);

    }

}


void TestQsummary::test_1c()
{
    int num_files = 3;

    std::vector<FileType> file_type;
    file_type.resize(num_files);

    file_type[0] = FileType::SMSPEC;
    file_type[1] = FileType::ESMRY;
    file_type[2] = FileType::ESMRY;

    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>> esmry_loader;
    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>> lodsmry_loader;

    esmry_loader[0] = std::make_unique<Opm::EclIO::ESmry>("../tests/smry_files/SENS0.SMSPEC");
    lodsmry_loader[1] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS1.ESMRY");
    lodsmry_loader[2] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS2.ESMRY");

    {
        std::string cmd_file = "../tests/cmd_files/test1c.txt";
        QsumCMDF cmdfile(cmd_file, num_files, "");

        SmryAppl::input_list_type input_charts;

        cmdfile.make_charts_from_cmd(input_charts, "");

        //print_input_charts(input_charts);

        QCOMPARE(input_charts.size(), 3);

        std::vector<std::vector<SmryAppl::vect_input_type>> ref_vect_input;

        ref_vect_input.push_back({});
        ref_vect_input[0].push_back({0, "WOPR:PROD-1", -1, false});
        ref_vect_input[0].push_back({1, "WOPR:PROD-1", -1, false});
        ref_vect_input[0].push_back({2, "WOPR:PROD-1", -1, false});
        ref_vect_input[0].push_back({0, "WXX1:PROD-1", -1, true});

        ref_vect_input.push_back({});
        ref_vect_input[1].push_back({0, "WOPR:PROD-1", -1, false});
        ref_vect_input[1].push_back({1, "WOPR:PROD-1", -1, false});
        ref_vect_input[1].push_back({2, "WOPR:PROD-1", -1, false});
        ref_vect_input[1].push_back({1, "WXX1:PROD-1", -1, true});

        ref_vect_input.push_back({});
        ref_vect_input[2].push_back({0, "WOPR:PROD-1", -1, false});
        ref_vect_input[2].push_back({1, "WOPR:PROD-1", -1, false});
        ref_vect_input[2].push_back({2, "WOPR:PROD-1", -1, false});
        ref_vect_input[2].push_back({2, "WXX1:PROD-1", -1, true});

        std::vector<SmryAppl::vect_input_type> test_vect_input;

        for (size_t t = 0; t < ref_vect_input.size(); t++){
            test_vect_input = std::get<0>(input_charts[t]);

            QCOMPARE(test_vect_input.size(), ref_vect_input[t].size());

            for (size_t n = 0; n < test_vect_input.size(); n++)
                QCOMPARE(test_vect_input[n], ref_vect_input[t][n]);

        }
    }
}

void TestQsummary::test_1f()
{
    int num_files = 3;

    std::vector<FileType> file_type;
    file_type.resize(num_files);

    file_type[0] = FileType::SMSPEC;
    file_type[1] = FileType::ESMRY;
    file_type[2] = FileType::ESMRY;

    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>> esmry_loader;
    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>> lodsmry_loader;

    esmry_loader[0] = std::make_unique<Opm::EclIO::ESmry>("../tests/smry_files/SENS0.SMSPEC");
    lodsmry_loader[1] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS1.ESMRY");
    lodsmry_loader[2] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS2.ESMRY");

    {
        std::string cmd_file = "../tests/cmd_files/test1f.txt";
        QsumCMDF cmdfile(cmd_file, num_files, "");

        SmryAppl::input_list_type input_charts;

        cmdfile.make_charts_from_cmd(input_charts, "");

        //print_input_charts(input_charts);

        QCOMPARE(input_charts.size(), 3);

        std::vector<std::vector<SmryAppl::vect_input_type>> ref_vect_input;

        ref_vect_input.push_back({});
        ref_vect_input[0].push_back({0, "WOPR:PROD-3", -1, false});
        ref_vect_input[0].push_back({0, "WXX1:PROD-X", -1, true});
        ref_vect_input[0].push_back({0, "WXX2:PROD-X", -1, true});

        ref_vect_input.push_back({});
        ref_vect_input[1].push_back({1, "WOPR:PROD-3", -1, false});
        ref_vect_input[1].push_back({1, "WXX1:PROD-X", -1, true});
        ref_vect_input[1].push_back({1, "WXX2:PROD-X", -1, true});

        ref_vect_input.push_back({});
        ref_vect_input[2].push_back({2, "WOPR:PROD-3", -1, false});
        ref_vect_input[2].push_back({2, "WXX1:PROD-X", -1, true});
        ref_vect_input[2].push_back({2, "WXX2:PROD-X", -1, true});

        std::vector<SmryAppl::vect_input_type> test_vect_input;

        for (size_t t = 0; t < ref_vect_input.size(); t++){
            test_vect_input = std::get<0>(input_charts[t]);

            QCOMPARE(test_vect_input.size(), ref_vect_input[t].size());

            for (size_t n = 0; n < test_vect_input.size(); n++)
                QCOMPARE(test_vect_input[n], ref_vect_input[t][n]);

        }
    }
}

void TestQsummary::test_1h()
{
    int num_files = 3;

    std::vector<FileType> file_type;
    file_type.resize(num_files);

    file_type[0] = FileType::SMSPEC;
    file_type[1] = FileType::ESMRY;
    file_type[2] = FileType::ESMRY;

    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>> esmry_loader;
    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>> lodsmry_loader;

    esmry_loader[0] = std::make_unique<Opm::EclIO::ESmry>("../tests/smry_files/SENS0.SMSPEC");
    lodsmry_loader[1] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS1.ESMRY");
    lodsmry_loader[2] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS2.ESMRY");

    {
        std::string cmd_file = "../tests/cmd_files/test1h.txt";
        QsumCMDF cmdfile(cmd_file, num_files, "");

        SmryAppl::input_list_type input_charts;

        cmdfile.make_charts_from_cmd(input_charts, "");

        //print_input_charts(input_charts);

        QCOMPARE(input_charts.size(), 3);

        std::vector<std::vector<SmryAppl::vect_input_type>> ref_vect_input;

        ref_vect_input.push_back({});
        ref_vect_input[0].push_back({0, "FWPR", -1, false});
        ref_vect_input[0].push_back({0, "FOWR", -1, true});

        ref_vect_input.push_back({});
        ref_vect_input[1].push_back({1, "FWPR", -1, false});
        ref_vect_input[1].push_back({1, "FOWR", -1, true});

        ref_vect_input.push_back({});
        ref_vect_input[2].push_back({2, "FWPR", -1, false});
        ref_vect_input[2].push_back({2, "FOWR", -1, true});

        std::vector<SmryAppl::vect_input_type> test_vect_input;

        for (size_t t = 0; t < ref_vect_input.size(); t++){
            test_vect_input = std::get<0>(input_charts[t]);

            QCOMPARE(test_vect_input.size(), ref_vect_input[t].size());

            for (size_t n = 0; n < test_vect_input.size(); n++)
                QCOMPARE(test_vect_input[n], ref_vect_input[t][n]);

        }
    }
}

void TestQsummary::test_2a()
{
    int num_files = 3;

    std::vector<FileType> file_type;
    file_type.resize(num_files);

    file_type[0] = FileType::SMSPEC;
    file_type[1] = FileType::ESMRY;
    file_type[2] = FileType::ESMRY;

    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>> esmry_loader;
    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>> lodsmry_loader;

    esmry_loader[0] = std::make_unique<Opm::EclIO::ESmry>("../tests/smry_files/SENS0.SMSPEC");
    lodsmry_loader[1] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS1.ESMRY");
    lodsmry_loader[2] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS2.ESMRY");

    {
        std::string cmd_file = "../tests/cmd_files/test2a.txt";
        QsumCMDF cmdfile(cmd_file, num_files, "");

        SmryAppl::input_list_type input_charts;

        cmdfile.make_charts_from_cmd(input_charts, "");

        //print_input_charts(input_charts);

        QCOMPARE(input_charts.size(), 6);

        std::vector<std::vector<SmryAppl::vect_input_type>> ref_vect_input;

        ref_vect_input.push_back({});
        ref_vect_input[0].push_back({0, "WOPR:PROD-1", -1, false});
        ref_vect_input[0].push_back({1, "WOPR:PROD-1", -1, false});
        ref_vect_input[0].push_back({2, "WOPR:PROD-1", -1, false});

        ref_vect_input.push_back({});
        ref_vect_input[1].push_back({0, "WOPR:PROD-2", -1, false});
        ref_vect_input[1].push_back({1, "WOPR:PROD-2", -1, false});
        ref_vect_input[1].push_back({2, "WOPR:PROD-2", -1, false});

        ref_vect_input.push_back({});
        ref_vect_input[2].push_back({0, "WOPR:PROD-3", -1, false});
        ref_vect_input[2].push_back({1, "WOPR:PROD-3", -1, false});
        ref_vect_input[2].push_back({2, "WOPR:PROD-3", -1, false});

        ref_vect_input.push_back({});
        ref_vect_input[3].push_back({0, "WOPR:PROD-4", -1, false});
        ref_vect_input[3].push_back({1, "WOPR:PROD-4", -1, false});
        ref_vect_input[3].push_back({2, "WOPR:PROD-4", -1, false});

        ref_vect_input.push_back({});
        ref_vect_input[4].push_back({0, "WBHP:INJ1", -1, false});
        ref_vect_input[4].push_back({1, "WBHP:INJ1", -1, false});
        ref_vect_input[4].push_back({2, "WBHP:INJ1", -1, false});

        ref_vect_input.push_back({});
        ref_vect_input[5].push_back({0, "WBHP:INJ2", -1, false});
        ref_vect_input[5].push_back({1, "WBHP:INJ2", -1, false});
        ref_vect_input[5].push_back({2, "WBHP:INJ2", -1, false});

        std::vector<SmryAppl::vect_input_type> test_vect_input;

        for (size_t t = 0; t < ref_vect_input.size(); t++){
            test_vect_input = std::get<0>(input_charts[t]);

            QCOMPARE(test_vect_input.size(), ref_vect_input[t].size());

            for (size_t n = 0; n < test_vect_input.size(); n++)
                QCOMPARE(test_vect_input[n], ref_vect_input[t][n]);

        }
    }
}

void TestQsummary::test_2b()
{
    int num_files = 3;

    std::vector<FileType> file_type;
    file_type.resize(num_files);

    file_type[0] = FileType::SMSPEC;
    file_type[1] = FileType::ESMRY;
    file_type[2] = FileType::ESMRY;

    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>> esmry_loader;
    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>> lodsmry_loader;

    esmry_loader[0] = std::make_unique<Opm::EclIO::ESmry>("../tests/smry_files/SENS0.SMSPEC");
    lodsmry_loader[1] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS1.ESMRY");
    lodsmry_loader[2] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS2.ESMRY");

    {
        std::string cmd_file = "../tests/cmd_files/test2b.txt";
        QsumCMDF cmdfile(cmd_file, num_files, "");

        SmryAppl::input_list_type input_charts;

        cmdfile.make_charts_from_cmd(input_charts, "");

        //print_input_charts(input_charts);

        QCOMPARE(input_charts.size(), 6);

        std::vector<std::vector<SmryAppl::vect_input_type>> ref_vect_input;

        ref_vect_input.push_back({});
        ref_vect_input[0].push_back({0, "WOPR:PROD-1", -1, false});
        ref_vect_input[0].push_back({1, "WOPR:PROD-1", -1, false});
        ref_vect_input[0].push_back({2, "WOPR:PROD-1", -1, false});

        ref_vect_input.push_back({});
        ref_vect_input[1].push_back({0, "WOPR:PROD-2", -1, false});
        ref_vect_input[1].push_back({1, "WOPR:PROD-2", -1, false});
        ref_vect_input[1].push_back({2, "WOPR:PROD-2", -1, false});

        ref_vect_input.push_back({});
        ref_vect_input[2].push_back({0, "WOPR:PROD-3", -1, false});
        ref_vect_input[2].push_back({1, "WOPR:PROD-3", -1, false});
        ref_vect_input[2].push_back({2, "WOPR:PROD-3", -1, false});

        ref_vect_input.push_back({});
        ref_vect_input[3].push_back({0, "WOPR:PROD-4", -1, false});
        ref_vect_input[3].push_back({1, "WOPR:PROD-4", -1, false});
        ref_vect_input[3].push_back({2, "WOPR:PROD-4", -1, false});

        ref_vect_input.push_back({});
        ref_vect_input[4].push_back({0, "WBHP:INJ1", -1, false});
        ref_vect_input[4].push_back({1, "WBHP:INJ1", -1, false});
        ref_vect_input[4].push_back({2, "WBHP:INJ1", -1, false});

        ref_vect_input.push_back({});
        ref_vect_input[5].push_back({0, "WBHP:INJ2", -1, false});
        ref_vect_input[5].push_back({1, "WBHP:INJ2", -1, false});
        ref_vect_input[5].push_back({2, "WBHP:INJ2", -1, false});

        std::vector<SmryAppl::vect_input_type> test_vect_input;

        for (size_t t = 0; t < ref_vect_input.size(); t++){
            test_vect_input = std::get<0>(input_charts[t]);

            QCOMPARE(test_vect_input.size(), ref_vect_input[t].size());

            for (size_t n = 0; n < test_vect_input.size(); n++)
                QCOMPARE(test_vect_input[n], ref_vect_input[t][n]);

        }
    }
}

void TestQsummary::test_2c()
{
    int num_files = 3;

    std::vector<FileType> file_type;
    file_type.resize(num_files);

    file_type[0] = FileType::SMSPEC;
    file_type[1] = FileType::ESMRY;
    file_type[2] = FileType::ESMRY;

    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>> esmry_loader;
    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>> lodsmry_loader;

    esmry_loader[0] = std::make_unique<Opm::EclIO::ESmry>("../tests/smry_files/SENS0.SMSPEC");
    lodsmry_loader[1] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS1.ESMRY");
    lodsmry_loader[2] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS2.ESMRY");

    {
        std::string cmd_file = "../tests/cmd_files/test2c.txt";
        QsumCMDF cmdfile(cmd_file, num_files, "PROD-1,PROD-2");

        SmryAppl::input_list_type input_charts;

        cmdfile.make_charts_from_cmd(input_charts, "");

        //print_input_charts(input_charts);

        QCOMPARE(input_charts.size(), 6);

        std::vector<std::vector<SmryAppl::vect_input_type>> ref_vect_input;

        ref_vect_input.push_back({});
        ref_vect_input[0].push_back({0, "WOPR:PROD-1", -1, false});
        ref_vect_input[0].push_back({1, "WOPR:PROD-1", -1, false});
        ref_vect_input[0].push_back({2, "WOPR:PROD-1", -1, false});

        ref_vect_input.push_back({});
        ref_vect_input[1].push_back({0, "WWCT:PROD-1", -1, false});
        ref_vect_input[1].push_back({1, "WWCT:PROD-1", -1, false});
        ref_vect_input[1].push_back({2, "WWCT:PROD-1", -1, false});

        ref_vect_input.push_back({});
        ref_vect_input[2].push_back({0, "WGOR:PROD-1", -1, false});
        ref_vect_input[2].push_back({1, "WGOR:PROD-1", -1, false});
        ref_vect_input[2].push_back({2, "WGOR:PROD-1", -1, false});

        ref_vect_input.push_back({});
        ref_vect_input[3].push_back({0, "WOPR:PROD-2", -1, false});
        ref_vect_input[3].push_back({1, "WOPR:PROD-2", -1, false});
        ref_vect_input[3].push_back({2, "WOPR:PROD-2", -1, false});

        ref_vect_input.push_back({});
        ref_vect_input[4].push_back({0, "WWCT:PROD-2", -1, false});
        ref_vect_input[4].push_back({1, "WWCT:PROD-2", -1, false});
        ref_vect_input[4].push_back({2, "WWCT:PROD-2", -1, false});

        ref_vect_input.push_back({});
        ref_vect_input[5].push_back({0, "WGOR:PROD-2", -1, false});
        ref_vect_input[5].push_back({1, "WGOR:PROD-2", -1, false});
        ref_vect_input[5].push_back({2, "WGOR:PROD-2", -1, false});

        std::vector<SmryAppl::vect_input_type> test_vect_input;

        for (size_t t = 0; t < ref_vect_input.size(); t++){
            test_vect_input = std::get<0>(input_charts[t]);

            QCOMPARE(test_vect_input.size(), ref_vect_input[t].size());

            for (size_t n = 0; n < test_vect_input.size(); n++)
                QCOMPARE(test_vect_input[n], ref_vect_input[t][n]);

        }
    }
}

void TestQsummary::test_3a()
{
    int num_files = 3;

    std::vector<FileType> file_type;
    file_type.resize(num_files);

    file_type[0] = FileType::SMSPEC;
    file_type[1] = FileType::ESMRY;
    file_type[2] = FileType::ESMRY;

    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>> esmry_loader;
    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>> lodsmry_loader;

    esmry_loader[0] = std::make_unique<Opm::EclIO::ESmry>("../tests/smry_files/SENS0.SMSPEC");
    lodsmry_loader[1] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS1.ESMRY");
    lodsmry_loader[2] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS2.ESMRY");

    {
        std::string cmd_file = "../tests/cmd_files/test3a.txt";
        QsumCMDF cmdfile(cmd_file, num_files, "");

        SmryAppl::input_list_type input_charts;

        cmdfile.make_charts_from_cmd(input_charts, "");

        //print_input_charts(input_charts);

        QCOMPARE(input_charts.size(), 9);

        std::vector<std::vector<SmryAppl::vect_input_type>> ref_vect_input;

        ref_vect_input.push_back({});
        ref_vect_input[0].push_back({0, "WWCT:PROD-1", -1, false});

        ref_vect_input.push_back({});
        ref_vect_input[1].push_back({0, "WWCT:PROD-2", -1, false});

        ref_vect_input.push_back({});
        ref_vect_input[2].push_back({0, "WWCT:PROD-3", -1, false});

        ref_vect_input.push_back({});
        ref_vect_input[3].push_back({1, "WWCT:PROD-1", -1, false});

        ref_vect_input.push_back({});
        ref_vect_input[4].push_back({1, "WWCT:PROD-2", -1, false});

        ref_vect_input.push_back({});
        ref_vect_input[5].push_back({1, "WWCT:PROD-3", -1, false});

        ref_vect_input.push_back({});
        ref_vect_input[6].push_back({2, "WWCT:PROD-1", -1, false});

        ref_vect_input.push_back({});
        ref_vect_input[7].push_back({2, "WWCT:PROD-2", -1, false});

        ref_vect_input.push_back({});
        ref_vect_input[8].push_back({2, "WWCT:PROD-3", -1, false});

        std::vector<SmryAppl::vect_input_type> test_vect_input;

        for (size_t t = 0; t < ref_vect_input.size(); t++){
            test_vect_input = std::get<0>(input_charts[t]);

            QCOMPARE(test_vect_input.size(), ref_vect_input[t].size());

            for (size_t n = 0; n < test_vect_input.size(); n++)
                QCOMPARE(test_vect_input[n], ref_vect_input[t][n]);

        }

    }
}


void TestQsummary::test_3b()
{
    int num_files = 3;

    std::vector<FileType> file_type;
    file_type.resize(num_files);

    file_type[0] = FileType::SMSPEC;
    file_type[1] = FileType::ESMRY;
    file_type[2] = FileType::ESMRY;

    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>> esmry_loader;
    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>> lodsmry_loader;

    esmry_loader[0] = std::make_unique<Opm::EclIO::ESmry>("../tests/smry_files/SENS0.SMSPEC");
    lodsmry_loader[1] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS1.ESMRY");
    lodsmry_loader[2] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS2.ESMRY");

    {
        std::string cmd_file = "../tests/cmd_files/test3b.txt";
        QsumCMDF cmdfile(cmd_file, num_files, "");

        SmryAppl::input_list_type input_charts;

        cmdfile.make_charts_from_cmd(input_charts, "");

        //print_input_charts(input_charts);

        QCOMPARE(input_charts.size(), 3);

        std::vector<std::vector<SmryAppl::vect_input_type>> ref_vect_input;

        ref_vect_input.push_back({});
        ref_vect_input[0].push_back({0, "WOPR:PROD-1", -1, false});
        ref_vect_input[0].push_back({1, "WOPR:PROD-1", -1, false});
        ref_vect_input[0].push_back({2, "WOPR:PROD-1", -1, false});
        ref_vect_input[0].push_back({0, "WOPR:PROD-2", -1, false});
        ref_vect_input[0].push_back({1, "WOPR:PROD-2", -1, false});
        ref_vect_input[0].push_back({2, "WOPR:PROD-2", -1, false});
        ref_vect_input[0].push_back({0, "WOPR:PROD-3", -1, false});
        ref_vect_input[0].push_back({1, "WOPR:PROD-3", -1, false});
        ref_vect_input[0].push_back({2, "WOPR:PROD-3", -1, false});

        ref_vect_input.push_back({});
        ref_vect_input[1].push_back({0, "WWCT:PROD-1", -1, false});
        ref_vect_input[1].push_back({1, "WWCT:PROD-1", -1, false});
        ref_vect_input[1].push_back({2, "WWCT:PROD-1", -1, false});
        ref_vect_input[1].push_back({0, "WWCT:PROD-2", -1, false});
        ref_vect_input[1].push_back({1, "WWCT:PROD-2", -1, false});
        ref_vect_input[1].push_back({2, "WWCT:PROD-2", -1, false});
        ref_vect_input[1].push_back({0, "WWCT:PROD-3", -1, false});
        ref_vect_input[1].push_back({1, "WWCT:PROD-3", -1, false});
        ref_vect_input[1].push_back({2, "WWCT:PROD-3", -1, false});

        ref_vect_input.push_back({});
        ref_vect_input[2].push_back({0, "WGOR:PROD-1", -1, false});
        ref_vect_input[2].push_back({1, "WGOR:PROD-1", -1, false});
        ref_vect_input[2].push_back({2, "WGOR:PROD-1", -1, false});
        ref_vect_input[2].push_back({0, "WGOR:PROD-2", -1, false});
        ref_vect_input[2].push_back({1, "WGOR:PROD-2", -1, false});
        ref_vect_input[2].push_back({2, "WGOR:PROD-2", -1, false});
        ref_vect_input[2].push_back({0, "WGOR:PROD-3", -1, false});
        ref_vect_input[2].push_back({1, "WGOR:PROD-3", -1, false});
        ref_vect_input[2].push_back({2, "WGOR:PROD-3", -1, false});

        std::vector<SmryAppl::vect_input_type> test_vect_input;

        for (size_t t = 0; t < ref_vect_input.size(); t++){
            test_vect_input = std::get<0>(input_charts[t]);

            QCOMPARE(test_vect_input.size(), ref_vect_input[t].size());

            for (size_t n = 0; n < test_vect_input.size(); n++)
                QCOMPARE(test_vect_input[n], ref_vect_input[t][n]);

        }
    }
}

void TestQsummary::test_3c()
{
    int num_files = 3;

    std::vector<FileType> file_type;
    file_type.resize(num_files);

    file_type[0] = FileType::SMSPEC;
    file_type[1] = FileType::ESMRY;
    file_type[2] = FileType::ESMRY;

    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>> esmry_loader;
    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>> lodsmry_loader;

    esmry_loader[0] = std::make_unique<Opm::EclIO::ESmry>("../tests/smry_files/SENS0.SMSPEC");
    lodsmry_loader[1] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS1.ESMRY");
    lodsmry_loader[2] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS2.ESMRY");

    {
        std::string cmd_file = "../tests/cmd_files/test3c.txt";
        QsumCMDF cmdfile(cmd_file, num_files, "");

        SmryAppl::input_list_type input_charts;

        cmdfile.make_charts_from_cmd(input_charts, "");

        //print_input_charts(input_charts);

        QCOMPARE(input_charts.size(), 9);

        std::vector<std::vector<SmryAppl::vect_input_type>> ref_vect_input;

        ref_vect_input.push_back({});
        ref_vect_input[0].push_back({0, "WWCT:PROD-1", -1, false});
        ref_vect_input[0].push_back({0, "WWCT2:PROD-1", -1, true});

        ref_vect_input.push_back({});
        ref_vect_input[1].push_back({1, "WWCT:PROD-1", -1, false});
        ref_vect_input[1].push_back({1, "WWCT2:PROD-1", -1, true});

        ref_vect_input.push_back({});
        ref_vect_input[2].push_back({2, "WWCT:PROD-1", -1, false});
        ref_vect_input[2].push_back({2, "WWCT2:PROD-1", -1, true});

        ref_vect_input.push_back({});
        ref_vect_input[3].push_back({0, "WWCT:PROD-2", -1, false});
        ref_vect_input[3].push_back({0, "WWCT2:PROD-2", -1, true});

        ref_vect_input.push_back({});
        ref_vect_input[4].push_back({1, "WWCT:PROD-2", -1, false});
        ref_vect_input[4].push_back({1, "WWCT2:PROD-2", -1, true});

        ref_vect_input.push_back({});
        ref_vect_input[5].push_back({2, "WWCT:PROD-2", -1, false});
        ref_vect_input[5].push_back({2, "WWCT2:PROD-2", -1, true});

        ref_vect_input.push_back({});
        ref_vect_input[6].push_back({0, "WWCT:PROD-3", -1, false});
        ref_vect_input[6].push_back({0, "WWCT2:PROD-3", -1, true});

        ref_vect_input.push_back({});
        ref_vect_input[7].push_back({1, "WWCT:PROD-3", -1, false});
        ref_vect_input[7].push_back({1, "WWCT2:PROD-3", -1, true});

        ref_vect_input.push_back({});
        ref_vect_input[8].push_back({2, "WWCT:PROD-3", -1, false});
        ref_vect_input[8].push_back({2, "WWCT2:PROD-3", -1, true});

        std::vector<SmryAppl::vect_input_type> test_vect_input;

        for (size_t t = 0; t < ref_vect_input.size(); t++){
            test_vect_input = std::get<0>(input_charts[t]);

            QCOMPARE(test_vect_input.size(), ref_vect_input[t].size());

            for (size_t n = 0; n < test_vect_input.size(); n++)
                QCOMPARE(test_vect_input[n], ref_vect_input[t][n]);

        }
    }
}


QTEST_MAIN(TestQsummary)

#include "test_cmdf_input_charts.moc"
