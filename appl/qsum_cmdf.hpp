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

#ifndef QSUM_CMDF_H
#define QSUM_CMDF_H

#include <appl/smry_appl.hpp>

#include <opm/io/eclipse/ESmry.hpp>
#include <opm/io/eclipse/ExtESmry.hpp>

#include <string>
#include <vector>
#include <map>


class QsumCMDF
{
public:

    // duplicated from SmryAppl
    // smry_ind, name, axis_ind, derived
    using vect_input_type = std::tuple<int, std::string, int, bool>;
    using char_input_type = std::tuple<std::vector<vect_input_type>, std::string>;
    using input_list_type = std::vector<char_input_type>;

    // name, expression and unit
    using define_vect_type = std::vector<std::tuple<std::string, std::string, std::string>>;

    QsumCMDF(const std::string& cmd_file, int num_smry_files,const std::string& cmdl_list);

    void make_charts_from_cmd(input_list_type& input_charts, const std::string xrange_str );

    void print_cmd_lines();
    void print_processd_cmd_lines();

    int count_define(){ return m_define_vect.size();};
    int derived_key_index(const std::string& name);


    void print_m_defined();

    define_vect_type get_define_vect() { return m_define_vect;}

private:

    std::vector<std::string> m_cmd_lines;
    std::vector<std::string> m_processed_cmd_lines;

    std::vector<std::vector<std::string>> m_list_vect;
    std::vector<std::string> m_list_names;

    // name, rhs expression and unit
    define_vect_type m_define_vect;

    int m_num_smry_files;

    void get_cmdlines(const std::string& filename);
    void remove_trailing_char(std::string& line, const std::string& charlist);
    bool update_variables();
    std::vector<std::string> split(const std::string& line, const std::string& delim);
    void process_cmdlines(const std::string& cmdl_list);
    std::vector<std::string> process_range(std::string& line, bool replace_on_line);

    void process_for_loop(const std::vector<std::string>& tokens, int& lnr,
                          std::map<std::string, std::string>& var_map,
                          std::vector<std::string>& processed_for_lines);


    std::string expand_line_add_series(const std::vector<std::string>& tokens, int smry_ind);
    std::string expand_line_define(const std::vector<std::string>& tokens, int smry_ind);

    void update_rhs_cmd_lines_define();
    void make_define_vect();

    void add_series(input_list_type& input_charts, int smry_ind, const std::string& name, int axis_ind, const std::string &xrange_input, bool is_derived);


    int replace_all(std::string& line, const std::string& repstr1, const std::string& repstr2, const std::string& newstr);
    bool is_number(const std::string& numstr);

};

#endif
