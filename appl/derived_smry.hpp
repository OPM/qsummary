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

#ifndef DERIVED_SMRY
#define DERIVED_SMRY

#include <appl/qsum_cmdf.hpp>

#include <unordered_map>

enum class FileType;
class QsumCMDF;


class DerivedSmry
{

public:

    using time_point = std::chrono::time_point<std::chrono::system_clock, std::chrono::duration<int64_t, std::ratio<1,1000>>>;

    // smry_id, name and unit
    using var_type = std::tuple<int, std::string, std::string>;

    // name, smry_id and smry_key
    using param_type = std::tuple<std::string, int, std::string>;
    using param_list_type = std::vector<param_type>;

    // var, params and expression
    using define_type = std::tuple<var_type, param_list_type, std::string>;

    // name, expression and unit
    using define_vect_type = std::vector<std::tuple<std::string, std::string, std::string>>;


    DerivedSmry(QsumCMDF cmdfile, const std::vector<FileType>& file_type,
                std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>>& esmry_loader,
                std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>>& lodsmry_loader);

    const std::vector<float>& get(int smry_id, const std::string& name) const;
    const std::string& get_unit(int smry_id, const std::string& name) const;

    bool is_derived(int smry_id, const std::string& name);

    void print_m_define_table();

    time_point startdate() const { return m_startdat; }

    const std::vector<define_type>& get_table() const { return m_define_table;}

    const std::vector<std::tuple<int, std::string>>& get_list() const { return m_derived_smry_list; }

private:

    int m_max_cases;
    time_point m_startdat;

    std::map<std::tuple<int, std::string>, std::vector<float>> m_smry_data;
    std::vector<std::tuple<int, std::string>> m_derived_smry_list;

    std::map<std::tuple<int, std::string>, std::string> m_unit_list;


    std::vector<define_type> m_define_table;


    int param_exists(const param_list_type& param_list, int smry_id, const std::string& key);
    void make_define_table(const define_vect_type& define_vect);

    void check_smry_exists(const std::vector<FileType>& file_type,
                std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>>& esmry_loader,
                std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>>& lodsmry_loader);

    void chk_startd_concistency(const std::vector<FileType>& file_type,
                std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>>& esmry_loader,
                std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>>& lodsmry_loader);

    void load_smry_data(const std::vector<FileType>& file_type,
                std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>>& esmry_loader,
                std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>>& lodsmry_loader);

    void make_global_time_vect(const std::vector<FileType>& file_type,
                std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>>& esmry_loader,
                std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>>& lodsmry_loader);

    void calc_derived_smry(const std::vector<FileType>& file_type,
                std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>>& esmry_loader,
                std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>>& lodsmry_loader);


    std::vector<float> calc_derived_vect(const std::string& expr,
                                         const std::vector<std::string>& param_name_list,
                                         const std::vector<std::vector<float>>& time_vect,
                                         const std::vector<int> param_time_vect_ind,
                                         const std::vector<std::vector<float>>& param_data);

    void calc_math_expr(std::vector<float>& derived_vect, const std::string& expr,
                        const std::vector<std::string>& param_name_list,
                        const std::vector<std::vector<float>>& param_vect);


};

#endif
