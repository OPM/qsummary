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

#include <appl/smry_appl.hpp>
#include <appl/qsum_func_lib.hpp>


class TestQsummary: public QObject
{
    Q_OBJECT

private slots:

   void test_1a();
   void test_1b();
};


bool compare_chartInput(const SmryAppl::input_list_type& input_charts_ref,
                        const SmryAppl::input_list_type& input_charts_test)
{
    if (input_charts_ref.size() != input_charts_test.size())
        return false;

    for ( size_t c = 0; c < input_charts_ref.size(); c++ ) {
        auto vect_input_test = std::get<0>(input_charts_test[c]);
        auto vect_input_ref = std::get<0>(input_charts_ref[c]);

        if (vect_input_test.size() != vect_input_ref.size())
            return false;

        auto xstr_test = std::get<0>(input_charts_test[c]);
        auto xstr_ref = std::get<0>(input_charts_ref[c]);

        if (xstr_test != xstr_ref)
            return false;

        for (size_t i = 0; i <  vect_input_ref.size(); i ++){

            int id_ref = std::get<0> ( vect_input_ref[i] );
            int id_test = std::get<0> (vect_input_test[i] );

            if (id_ref != id_test)
                return false;

            std::string vname_ref = std::get<1> ( vect_input_ref[i] );
            std::string vname_test = std::get<1> (vect_input_test[i] );

            if (vname_ref != vname_test)
                return false;

            int axis_ind_ref = std::get<2> ( vect_input_ref[i] );
            int axis_ind_test = std::get<2> (vect_input_test[i] );

            if (axis_ind_ref != axis_ind_test)
                return false;

            bool derived_ref = std::get<3> ( vect_input_ref[i] );
            bool derived__test = std::get<3> (vect_input_test[i] );

            if (derived_ref != derived__test)
                return false;
        }
    }

    return true;
}


void TestQsummary::test_1a()
{
    // setting up input files

    std::vector<std::string> file_list;
    file_list.resize(6);

    file_list[0]="realization-1/1_R013_OMEGA-1.ESMRY";
    file_list[1]="realization-1/1_R013_OMEGA_1910A-1.ESMRY";
    file_list[2]="realization-21/1_R013_OMEGA-21.ESMRY";
    file_list[3]="realization-21/1_R013_OMEGA_1910A-21.ESMRY";
    file_list[4]="realization-22/1_R013_OMEGA-22.ESMRY";
    file_list[5]="realization-22/1_R013_OMEGA_1910A-22.ESMRY";

    std::vector<std::filesystem::path> smry_files;

    for (auto& fname : file_list){
        std::filesystem::path filename(fname);
        smry_files.push_back(filename);
    }

    // set up charts input to be used with function QSum::charts_separate_folders
    SmryAppl::input_list_type input_charts;
    std::string xrange_str;

    std::vector<std::tuple<int, std::string, int, bool>> vect_list;

    vect_list.push_back(std::make_tuple(0, "TCPU", -1, false));
    vect_list.push_back(std::make_tuple(1, "TCPU", -1, false));
    vect_list.push_back(std::make_tuple(2, "TCPU", -1, false));
    vect_list.push_back(std::make_tuple(3, "TCPU", -1, false));
    vect_list.push_back(std::make_tuple(4, "TCPU", -1, false));
    vect_list.push_back(std::make_tuple(5, "TCPU", -1, false));

    input_charts = {std::make_tuple(vect_list, xrange_str)};

    // chart input which is modified by function QSum::charts_separate_folders
    SmryAppl::input_list_type mod_input_charts;
    mod_input_charts = QSum::charts_separate_folders(smry_files, input_charts);

    // set up chart input as it should be after using function QSum::charts_separate_folders
    // should be compared with output from function QSum::charts_separate_folders
    SmryAppl::input_list_type ref_input_charts;

    std::vector<std::vector<std::tuple<int, std::string, int, bool>>> ref_vect_list;
    ref_vect_list.resize(3);

    ref_vect_list[0].push_back(std::make_tuple(0, "TCPU", -1, false));
    ref_vect_list[0].push_back(std::make_tuple(1, "TCPU", -1, false));
    ref_vect_list[1].push_back(std::make_tuple(2, "TCPU", -1, false));
    ref_vect_list[1].push_back(std::make_tuple(3, "TCPU", -1, false));
    ref_vect_list[2].push_back(std::make_tuple(4, "TCPU", -1, false));
    ref_vect_list[2].push_back(std::make_tuple(5, "TCPU", -1, false));


    ref_input_charts = { std::make_tuple(ref_vect_list[0], xrange_str),
                         std::make_tuple(ref_vect_list[1], xrange_str),
                         std::make_tuple(ref_vect_list[2], xrange_str)
                       };

    QCOMPARE(compare_chartInput(ref_input_charts, mod_input_charts), true);

}

void TestQsummary::test_1b()
{
    // setting up input files

    std::vector<std::string> file_list;
    file_list.resize(6);

    file_list[0]="realization-1/1_R013_OMEGA-1.ESMRY";
    file_list[1]="realization-21/1_R013_OMEGA-21.ESMRY";
    file_list[2]="realization-22/1_R013_OMEGA-22.ESMRY";
    file_list[3]="realization-1/1_R013_OMEGA_1910A-1.ESMRY";
    file_list[4]="realization-21/1_R013_OMEGA_1910A-21.ESMRY";
    file_list[5]="realization-22/1_R013_OMEGA_1910A-22.ESMRY";

    std::vector<std::filesystem::path> smry_files;

    for (auto& fname : file_list){
        std::filesystem::path filename(fname);
        smry_files.push_back(filename);
    }

    // set up charts input to be used with function QSum::charts_separate_folders
    SmryAppl::input_list_type input_charts;
    std::string xrange_str;

    std::vector<std::tuple<int, std::string, int, bool>> vect_list;

    vect_list.push_back(std::make_tuple(0, "TCPU", -1, false));
    vect_list.push_back(std::make_tuple(1, "TCPU", -1, false));
    vect_list.push_back(std::make_tuple(2, "TCPU", -1, false));
    vect_list.push_back(std::make_tuple(3, "TCPU", -1, false));
    vect_list.push_back(std::make_tuple(4, "TCPU", -1, false));
    vect_list.push_back(std::make_tuple(5, "TCPU", -1, false));

    input_charts = {std::make_tuple(vect_list, xrange_str)};

    // chart input which is modified by function QSum::charts_separate_folders
    SmryAppl::input_list_type mod_input_charts;
    mod_input_charts = QSum::charts_separate_folders(smry_files, input_charts);

    // set up chart input as it should be after using function QSum::charts_separate_folders
    // should be compared with output from function QSum::charts_separate_folders
    SmryAppl::input_list_type ref_input_charts;

    std::vector<std::vector<std::tuple<int, std::string, int, bool>>> ref_vect_list;
    ref_vect_list.resize(3);

    ref_vect_list[0].push_back(std::make_tuple(0, "TCPU", -1, false));
    ref_vect_list[0].push_back(std::make_tuple(3, "TCPU", -1, false));
    ref_vect_list[1].push_back(std::make_tuple(1, "TCPU", -1, false));
    ref_vect_list[1].push_back(std::make_tuple(4, "TCPU", -1, false));
    ref_vect_list[2].push_back(std::make_tuple(2, "TCPU", -1, false));
    ref_vect_list[2].push_back(std::make_tuple(5, "TCPU", -1, false));


    ref_input_charts = { std::make_tuple(ref_vect_list[0], xrange_str),
                         std::make_tuple(ref_vect_list[1], xrange_str),
                         std::make_tuple(ref_vect_list[2], xrange_str)
                       };


    QCOMPARE(compare_chartInput(ref_input_charts, mod_input_charts), true);
}


QTEST_MAIN(TestQsummary)

#include "test_sep_folders.moc"
