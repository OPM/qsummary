/*
   Copyright 2019 Equinor ASA.

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

#include <appl/qsum_func_lib.hpp>

#include <algorithm>
#include <omp.h>
#include <iostream>


int next_vect(int from, const std::string& vect_string){

    // find first of comma (,) which is not a digit (0-9) and not * or ?

    bool found = false;
    int p2;

    while (!found){
        p2 = vect_string.find_first_of(",",from);

        if (p2 == -1)
            return p2;

        char next_char = vect_string[p2+1];

        if ((!std::isdigit(next_char)) && (next_char != '*') && (next_char != '?') ){
            return p2;
        } else {
            from = p2 + 1;
        }
    }

    return -1;
}

void QSum::chart_input_from_string(std::string& vect_string,
                                   SmryAppl::input_list_type& input_charts,
                                   const std::vector<FileType>& file_type,
                                   std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>>& esmry_loader,
                                   std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>>& lodsmry_loader,
                                   const int max_number_of_charts,
                                   const std::string& xrange
                                  )
{
    std::transform(vect_string.begin(), vect_string.end(), vect_string.begin(), ::toupper);

    while (vect_string.back() == ',')
        vect_string.pop_back();

    std::vector<std::string> vect_list;

    int p1 = 0;
    int p2 = next_vect(p1, vect_string);

    while (p2 > -1) {
        vect_list.push_back(vect_string.substr(p1, p2 - p1));
        p1 = p2 + 1;
        p2 = next_vect(p1, vect_string);
    }

    vect_list.push_back(vect_string.substr(p1));

    for (auto vect : vect_list) {

        int p = vect.find_first_of("*");

        std::vector<std::string> keyw_list;

        if (p > -1) {

            if (file_type[0] == FileType::SMSPEC)
                keyw_list = esmry_loader[0]->keywordList(vect);
            else if (file_type[0] == FileType::ESMRY)
                keyw_list = lodsmry_loader[0]->keywordList(vect);

        } else {
            keyw_list.push_back(vect);
        }

        QSum::update_input(input_charts, keyw_list, file_type, esmry_loader, lodsmry_loader, max_number_of_charts, xrange);
    }

}

void QSum::update_input(SmryAppl::input_list_type& input_charts,
                  const std::vector<std::string>& keyw_list,
                  const std::vector<FileType>& file_type,
                  std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>>& esmry_loader,
                  std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>>& lodsmry_loader,
                  const int max_number_of_charts,
                  const std::string& xrange
                 )
{
    for (auto v : keyw_list) {

        std::vector<SmryAppl::vect_input_type> vect_list;

        for (size_t n = 0; n < file_type.size(); n ++){

            bool hasVect;

            if (file_type[n] == FileType::SMSPEC)
                hasVect = esmry_loader[n]->hasKey(v);
            else if (file_type[n] == FileType::ESMRY)
                hasVect = lodsmry_loader[n]->hasKey(v);

            if (hasVect)
                vect_list.push_back(std::make_tuple (n, v, -1, false));
        }

        if (vect_list.size() > 0){
            SmryAppl::char_input_type chart;
            chart = std::make_tuple(vect_list, xrange);
            input_charts.push_back(chart);
        }

        if (input_charts.size() > max_number_of_charts)
            throw std::invalid_argument("exceeds maximum number of charts ");
    }
}


void QSum::pre_load_smry(const std::vector<std::filesystem::path>& smry_files,
                   const SmryAppl::input_list_type& input_charts,
                   const std::vector<FileType>& file_type,
                   std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>>& esmry_loader,
                   std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>>& lodsmry_loader,
                   const size_t nthreads)
{
    std::vector<std::vector<std::string>> smry_pre_load;

    for (size_t n = 0; n < smry_files.size(); n++)
        smry_pre_load.push_back({"TIME"});

    for (size_t c = 0; c < input_charts.size(); c++) {
        auto vect_input = std::get<0>(input_charts[c]);

        for (size_t s = 0; s < vect_input.size(); s++) {
            auto smry_ind = std::get<0>(vect_input[s]);
            auto vect_name = std::get<1>(vect_input[s]);
            auto is_derived = std::get<3>(vect_input[s]);

            if (!is_derived){
                if (std::count(smry_pre_load[smry_ind].begin(), smry_pre_load[smry_ind].end(), vect_name) == 0){
                    smry_pre_load[smry_ind].push_back(vect_name);
                }
            }
        }
    }

    omp_set_num_threads(nthreads);

    auto start_load = std::chrono::system_clock::now();

    #pragma omp parallel for
    for (size_t n = 0; n < smry_files.size(); n++){
        if (smry_pre_load[n].size() > 0)
            if (file_type[n] == FileType::SMSPEC){
                esmry_loader[n]->loadData(smry_pre_load[n]);
            } else if (file_type[n] == FileType::ESMRY){
                lodsmry_loader[n]->loadData(smry_pre_load[n]);
            }
    }

    auto end_load = std::chrono::system_clock::now();

    std::chrono::duration<double> elapsed_seconds = end_load-start_load;

    std::cout << ", loading: " <<  elapsed_seconds.count();
}

template <typename T>
bool QSum::has_nonzero(std::unique_ptr<T>& smry, const std::string key)
{
    if ((key == "TIMESTEP") && (smry->all_steps_available()))
        return true;

    if (!smry->hasKey(key))
        return false;

    auto vect = smry->get(key);

    for (auto v : vect)
        if (abs(v) > 0.0)
            return true;

    return false;
}



void QSum::remove_zero_vect(const std::vector<std::filesystem::path>& smry_files,
                            SmryAppl::input_list_type& input_charts,
                            const std::vector<FileType>& file_type,
                            std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>>& esmry_loader,
                            std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>>& lodsmry_loader)
{

    int c = static_cast<int>(input_charts.size()) -1;

    while (c > -1) {

        auto vect_input = std::get<0>(input_charts[c]);
        auto xrange_str = std::get<1>(input_charts[c]);

        int n = static_cast<int>(vect_input.size()) -1 ;

        while (n > -1) {
            auto smry_ind = std::get<0>(vect_input[n]);
            auto vect_name = std::get<1>(vect_input[n]);

            bool nonzero;

            if (file_type[n] == FileType::SMSPEC)
                nonzero = has_nonzero(esmry_loader[n], vect_name);
            else if (file_type[n] == FileType::ESMRY)
                nonzero = has_nonzero(lodsmry_loader[n], vect_name);

            if (!nonzero)
                vect_input.erase( vect_input.begin() + n );

            n--;
        }

        if (vect_input.size() > 0)
            input_charts[c] = std::make_tuple(vect_input, xrange_str);
        else
            input_charts.erase( input_charts.begin() + c );

        c --;
    }
}

SmryAppl::input_list_type QSum::charts_separate_folders(const std::vector<std::filesystem::path>& smry_files,
                                                   const SmryAppl::input_list_type& input_charts)
{
    SmryAppl::input_list_type updated_chart_input;
    std::vector<std::string> folderList;

    for (auto f: smry_files)
        folderList.push_back(f.parent_path().string());

    int cind = 0;

    std::vector<std::vector<int>> smry_ind_vect;
    std::vector<std::vector<std::string>> key_vect;
    std::vector<std::string> xrange_str_vect;
    std::vector<std::string> f_list;

    for (auto element: input_charts) {

        auto series_vector = std::get<0>(element);
        auto xrange_str = std::get<1>(element);


        for (size_t n = 0; n < series_vector.size(); n++) {
            int smry_ind = std::get<0>(series_vector[n]);
            std::string folder = folderList[smry_ind];

            int chart_index;

            if (n == 0) {
                smry_ind_vect.push_back({});
                key_vect.push_back({});
                xrange_str_vect.push_back(xrange_str);
                f_list.push_back(folder);

                chart_index = 0;

            } else {

                auto it = std::find(f_list.begin(), f_list.end(), folder);

                if (it != f_list.end()) {
                    chart_index = std::distance(f_list.begin(), it);
                } else {
                    f_list.push_back(folder);
                    smry_ind_vect.push_back({});
                    key_vect.push_back({});
                    xrange_str_vect.push_back(xrange_str);

                    chart_index = smry_ind_vect.size() - 1;
                }
            }

            smry_ind_vect[chart_index].push_back(std::get<0>(series_vector[n]));
            key_vect[chart_index].push_back(std::get<1>(series_vector[n]));
        }

        cind++;
    }


    for (size_t c = 0; c < smry_ind_vect.size(); c++){

        SmryAppl::char_input_type chart_input;
        std::vector<SmryAppl::vect_input_type> vect_input_list;

        for (size_t n = 0; n < smry_ind_vect[c].size(); n++){
            std::tuple<int, std::string, int, bool> vect;
            vect = std::make_tuple(smry_ind_vect[c][n], key_vect[c][n], -1, false);
            vect_input_list.push_back(vect);
        }

        chart_input = std::make_tuple(vect_input_list, xrange_str_vect[c]);

        updated_chart_input.push_back(chart_input);
    }

    return updated_chart_input;
}


template <typename T>
bool QSum::double_check_well_vector(std::string& vect_name, std::unique_ptr<T>& smry)
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


void QSum::check_summary_vectors(SmryAppl::input_list_type& input_charts,
                   const std::vector<FileType>& file_type,
                   std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>>& esmry_loader,
                   std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>>& lodsmry_loader)
{

    for ( size_t c = 0; c < input_charts.size(); c++ ) {

        std::vector<SmryAppl::vect_input_type> vect_input;
        vect_input = std::get<0>(input_charts[c]);

        for ( size_t i = 0; i < vect_input.size(); i++ ) {
            int n = std::get<0> ( vect_input[i] );
            std::string vect_name = std::get<1> ( vect_input[i] );
            int axis = std::get<2> ( vect_input[i] );
            bool is_derived = std::get<3> ( vect_input[i] );

            if (!is_derived) {

                if (file_type[n] == FileType::SMSPEC) {

                    if (!esmry_loader[n]->hasKey(vect_name)) {
                        if (vect_name.substr(0,1) == "W") {
                            if (!double_check_well_vector(vect_name, esmry_loader[n]))
                                throw std::invalid_argument("not able to load smry vector " + vect_name);
                        } else {
                            throw std::invalid_argument("not able to load smry vector " + vect_name);
                        }

                        vect_input[i] = std::make_tuple(n, vect_name, axis, false);
                        auto xrange_str = std::get<1>(input_charts[c]);
                        input_charts[c] = std::make_tuple(vect_input, xrange_str);
                    }

                } else if (file_type[n] == FileType::ESMRY) {

                    if (!lodsmry_loader[n]->hasKey(vect_name)) {
                        if (vect_name.substr(0,1) == "W") {
                            if (!double_check_well_vector(vect_name, lodsmry_loader[n]))
                                throw std::invalid_argument("not able to load smry vector " + vect_name);
                        } else {
                            throw std::invalid_argument("not able to load smry vector " + vect_name);
                        }

                        vect_input[i] = std::make_tuple(n, vect_name, axis, false);
                        auto xrange_str = std::get<1>(input_charts[c]);
                        input_charts[c] = std::make_tuple(vect_input, xrange_str);

                    }
                } else {
                    throw std::invalid_argument("unknown file type");
                }
            }
        }
    }
}


void QSum::print_input_charts(const SmryAppl::input_list_type& input_charts)
{

    std::cout << "number of charts: " << input_charts.size() << std::endl;

    for ( size_t c = 0; c < input_charts.size(); c++ ) {

        std::vector<SmryAppl::vect_input_type> vect_input;
        vect_input = std::get<0>(input_charts[c]);
        auto xstr = std::get<1>(input_charts[c]);

        std::cout << "chart : " << c << " xrange_str ='" << xstr << "'" << std::endl;

        for ( size_t i=0; i < vect_input.size(); i++ ) {
            int n = std::get<0> ( vect_input[i] );
            std::string vect_name = std::get<1> ( vect_input[i] );
            int axis_ind = std::get<2> ( vect_input[i] );
            bool is_derived = std::get<3> ( vect_input[i] );

            std::cout << "smry_ind= " << n << " > vect_name: " << vect_name << "  ";
            std::cout << " > axis_ind: '" << axis_ind << "'  ";
            std::cout << std::boolalpha << is_derived;
            std::cout << " | " << xstr ;
            std::cout << std::endl;
        }
    }
}
