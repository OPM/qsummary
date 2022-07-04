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

#include <string>
#include <vector>


class QsumCMDF
{
public:

    QsumCMDF(const std::string& cmd_file, int num_smry_files);

    void make_charts_from_cmd(SmryAppl::input_list_type& input_charts, const std::string xrange_str );

    void print_cmd_lines();
    void print_processd_cmd_lines();

private:

    std::vector<std::string> m_cmd_lines;
    std::vector<std::string> m_processed_cmd_lines;

    int m_num_smry_files;

    void get_cmdlines(const std::string& filename);
    void remove_trailing_char(std::string& line, const std::string& charlist);
    bool update_variables();
    std::vector<std::string> split(const std::string& line, const std::string& delim);
    std::vector<std::string> process_cmdlines();
    std::vector<std::string> process_range(std::string& line, bool replace_on_line);
    std::string make_new_line(const std::vector<std::string>& tokens, int smry_ind);
    void add_series(SmryAppl::input_list_type& input_charts, int smry_ind, const std::string& name, int axis_ind,
                    const std::string &xrange_input);

};

#endif
