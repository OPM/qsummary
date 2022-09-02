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

#include <appl/derived_smry.hpp>

#include <mathexpr/exprtk.hpp>

#include <iostream>
#include <set>

DerivedSmry::DerivedSmry(QsumCMDF cmdfile, const std::vector<FileType>& file_type,
                         std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>>& esmry_loader,
                         std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>>& lodsmry_loader
)
{
    QsumCMDF::define_vect_type define_vect = cmdfile.get_define_vect();
    m_max_cases = file_type.size();

    if (file_type[0]==FileType::SMSPEC)
        m_startdat = esmry_loader[0]->startdate();
    else
        m_startdat = lodsmry_loader[0]->startdate();

    make_define_table(define_vect);

    check_smry_exists(file_type, esmry_loader, lodsmry_loader);

    load_smry_data(file_type, esmry_loader, lodsmry_loader);

    chk_startd_concistency(file_type, esmry_loader, lodsmry_loader);

    make_global_time_vect(file_type, esmry_loader, lodsmry_loader);

    calc_derived_smry(file_type, esmry_loader, lodsmry_loader);

    for (auto it = m_smry_data.begin(); it != m_smry_data.end(); it++)
        m_derived_smry_list.push_back(it->first);
}

void DerivedSmry::recalc(const std::vector<FileType>& file_type,
                std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>>& esmry_loader,
                std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>>& lodsmry_loader)
{

    m_smry_data.clear();

    load_smry_data(file_type, esmry_loader, lodsmry_loader);

    make_global_time_vect(file_type, esmry_loader, lodsmry_loader);

    calc_derived_smry(file_type, esmry_loader, lodsmry_loader);

    m_derived_smry_list.clear();

    for (auto it = m_smry_data.begin(); it != m_smry_data.end(); it++)
        m_derived_smry_list.push_back(it->first);
}


void DerivedSmry::make_define_table(const QsumCMDF::define_vect_type& define_vect)
{
    for (size_t n = 0; n < define_vect.size(); n ++){

        auto name = std::get<0>(define_vect[n]);
        std::string expr = std::get<1>(define_vect[n]);
        std::string unit = std::get<2>(define_vect[n]);

        int p = name.find_first_of(":");
        int smry_id = std::stoi(name.substr(0,p)) - 1;

        name = name.substr(p + 1);

        var_type var = std::make_tuple(smry_id, name, unit);

        std::string mod_expr = expr;

        p = expr.find_first_of("$");
        int nvar = 0;
        int p_mod_expr = 0;

        param_list_type param_list;

        while (p != std::string::npos) {

            auto p_end = expr.find_first_of("}", p);

            std::string f_var_name;
            std::string var_name;

            if (p_end == std::string::npos)
                f_var_name = expr.substr(p + 0);
            else
                f_var_name = expr.substr(p + 0, p_end - p + 1);


            auto p1 = f_var_name.find_first_of(":");
            smry_id = std::stoi(f_var_name.substr(2, p1 - 2)) - 1;

            var_name = f_var_name.substr(p1 + 1);
            var_name.pop_back();

            std::string mod_vname;

            int param_ind = param_exists(param_list, smry_id, var_name);

            if (param_ind > -1) {
                mod_vname = std::get<0>(param_list[param_ind]);
            } else {
                nvar++;
                mod_vname = "X" + std::to_string(nvar);
                param_type param = std::make_tuple(mod_vname, smry_id, var_name);
                param_list.push_back(param);
            }

            p_mod_expr = mod_expr.find(f_var_name, p_mod_expr);
            mod_expr.replace(p_mod_expr, f_var_name.size(), mod_vname);

            p = expr.find_first_of("$", p+1);
        }

        define_type define = std::make_tuple(var, param_list, mod_expr);
        m_define_table.push_back(define);
    }
}

void DerivedSmry::chk_startd_concistency(const std::vector<FileType>& file_type,
                std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>>& esmry_loader,
                std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>>& lodsmry_loader)
{
    for (auto& define: m_define_table){

        var_type var = std::get<0>(define);
        int var_smry_id = std::get<0>(var);

        time_point startd_var;

        if (var_smry_id < 0)
            startd_var = startdate();
        else  if (file_type[var_smry_id] == FileType::SMSPEC)
            startd_var = esmry_loader[var_smry_id]->startdate();
        else  if (file_type[var_smry_id] == FileType::ESMRY)
            startd_var = lodsmry_loader[var_smry_id]->startdate();


        std::string var_smry_key = std::get<1>(var);

        param_list_type param_list = std::get<1>(define);

        std::set<int> smry_id_set;

        for (auto& param : param_list){

            int smry_id = std::get<1>(param);
            std::string smry_key = std::get<2>(param);

            smry_id_set.insert(smry_id);
        }

        for (auto it=smry_id_set.begin(); it!=smry_id_set.end(); ++it){
            int n = *it;
            time_point startd;

            if (n < 0)
                startd = startdate();
            else  if (file_type[n] == FileType::SMSPEC)
                startd = esmry_loader[n]->startdate();
            else  if (file_type[n] == FileType::ESMRY)
                startd = lodsmry_loader[n]->startdate();

            if (startd != startd_var){
                std::string message = "Start date inconcistency for dervived smry " + std::to_string(var_smry_id);
                message = message + ":" + var_smry_key + ". All parameters must have same start date";

                throw std::runtime_error(message);
            }
        }
    }
}


void DerivedSmry::check_smry_exists(const std::vector<FileType>& file_type,
                std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>>& esmry_loader,
                std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>>& lodsmry_loader)
{
    std::vector<std::tuple<int, std::string>> def_var_list;

    for (auto& define: m_define_table){

        var_type var = std::get<0>(define);

        int var_smry_id = std::get<0>(var);
        std::string var_smry_key = std::get<1>(var);

        param_list_type param_list = std::get<1>(define);

        for (auto& param : param_list){

            int smry_id = std::get<1>(param);
            std::string smry_key = std::get<2>(param);

            std::tuple<int, std::string> cand = std::make_tuple(smry_id, smry_key);

            if (smry_id < 0) {

                auto it = std::find (def_var_list.begin(), def_var_list.end(), cand);

                if (it == def_var_list.end()) {
                    std::cout << "in define " << var_smry_id << ":" << var_smry_key;
                    std::cout << " key " << smry_id << ":" << smry_key << " not found \n";
                    exit(1);
                }

            } else {

                auto it = std::find (def_var_list.begin(), def_var_list.end(), cand);

                if (it == def_var_list.end()){

                    if (file_type[smry_id] == FileType::SMSPEC) {

                        if (!esmry_loader[smry_id]->hasKey(smry_key)){
                            std::cout << "in define " << var_smry_id << ":" << var_smry_key;
                            std::cout << " key " << smry_id << ":" << smry_key << " not found \n";
                           exit(1);
                        }

                    } else if (file_type[smry_id] == FileType::ESMRY) {

                        if (!lodsmry_loader[smry_id]->hasKey(smry_key)){
                           std::cout << "in define " << var_smry_id << ":" << var_smry_key;
                           std::cout << " key " << smry_id << ":" << smry_key << " not found \n";
                           exit(1);
                        }
                    }
                }
            }
        }

        def_var_list.push_back({var_smry_id, var_smry_key});
    }
}


void DerivedSmry::load_smry_data(const std::vector<FileType>& file_type,
                std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>>& esmry_loader,
                std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>>& lodsmry_loader)
{
    std::vector<std::vector<std::string>> vect_load_list;
    int max_cases = file_type.size();

    for (size_t n = 0; n < max_cases; n++)
        vect_load_list.push_back({"TIME"});

    for (auto& define: m_define_table){

        var_type var = std::get<0>(define);
        int var_smry_id = std::get<0>(var);
        std::string var_smry_key = std::get<1>(var);
        param_list_type param_list = std::get<1>(define);

        for (auto& param : param_list){

            int smry_id = std::get<1>(param);
            std::string smry_key = std::get<2>(param);

            if (smry_id > (max_cases - 1)){
                std::cout << "\nsmry id out of range in define " << var_smry_key << "\n";
                exit(1);
            }

            if ((smry_id > -1) && (!is_derived(smry_id, smry_key)))
                if (std::find(vect_load_list[smry_id].begin(), vect_load_list[smry_id].end(), smry_key) == vect_load_list[smry_id].end())
                    vect_load_list[smry_id].push_back(smry_key);
        }
    }

    for (size_t n = 0; n < vect_load_list.size(); n++){
        if (vect_load_list[n].size() > 0)
            if (file_type[n] == FileType::SMSPEC)
                esmry_loader[n]->loadData(vect_load_list[n]);
            else if (file_type[n] == FileType::ESMRY)
                lodsmry_loader[n]->loadData(vect_load_list[n]);
            else
                throw std::invalid_argument("invalied summary file type");
    }
}


bool DerivedSmry::is_derived(int smry_id, const std::string& name)
{
    for (auto& define: m_define_table){
        var_type var = std::get<0>(define);
        int var_smry_id = std::get<0>(var);
        std::string var_smry_key = std::get<1>(var);

        if ((var_smry_id == smry_id) && (var_smry_key == name))
            return true;
    }

    return false;
}

void DerivedSmry::print_m_define_table()
{
    std::cout << m_define_table.size() << std::endl;

    for (auto& define: m_define_table){

        var_type var = std::get<0>(define);
        param_list_type param_list = std::get<1>(define);
        std::string expr = std::get<2>(define);

        std::cout << "(" << std::get<0>(var) << ", " << std::get<1>(var) << ", " << std::get<2>(var) << ") ";
        std::cout << " expr: " << expr;
        std::cout << "\n";

        for (auto& param : param_list){
            std::cout <<  "  " << std::get<0>(param);
            std::cout <<  "  = " << std::get<1>(param);
            std::cout <<  " " << std::get<2>(param);

            std::cout << "\n";
        }

        std::cout << "\n";
    }
}


int DerivedSmry::param_exists(const param_list_type& param_list, int smry_id, const std::string& key)
{

    for (int n = 0; n < param_list.size(); n++)
        if ((std::get<1>(param_list[n]) == smry_id) && (std::get<2>(param_list[n]) == key))
            return n;

    return -1;
}


void DerivedSmry::make_global_time_vect(const std::vector<FileType>& file_type,
                std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>>& esmry_loader,
                std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>>& lodsmry_loader)
{
    std::set<int> smry_id_used_in_global;

    for (auto& define: m_define_table){

        var_type var = std::get<0>(define);

        int var_smry_id = std::get<0>(var);
        std::string var_smry_key = std::get<1>(var);

        if (var_smry_id < 0){

            param_list_type param_list = std::get<1>(define);

            for (auto& param : param_list){
                auto smry_id = std::get<1>(param);
                if (smry_id > -1)
                    smry_id_used_in_global.insert(std::get<1>(param));
            }
        }
    }

    std::vector<float> time_data;

    if (file_type[0] == FileType::SMSPEC)
        time_data = esmry_loader[0]->get("TIME");
    else
        time_data = lodsmry_loader[0]->get("TIME");

    std::set<float> time_data_set;

    for (auto it=smry_id_used_in_global.begin(); it!=smry_id_used_in_global.end(); ++it){
        int n = *it;

        std::vector<float> time_data;

        if (file_type[n] == FileType::SMSPEC)
            time_data = esmry_loader[n]->get("TIME");
        else  if (file_type[n] == FileType::ESMRY)
            time_data = lodsmry_loader[n]->get("TIME");

        for (auto time : time_data)
            time_data_set.insert(time);
    }

    if (time_data_set.size() > 0){
        std::tuple<int, std::string> smry_key = std::make_tuple(-1, "TIME");
        std::vector<float> time_vect;

        for (auto it = time_data_set.begin(); it != time_data_set.end(); ++it)
            time_vect.push_back(*it);

        m_smry_data.insert({smry_key, time_vect});
    }
}

std::vector<float> DerivedSmry::calc_derived_vect(const std::string& expr,
                                     const std::vector<std::string>& param_name_list,
                                     const std::vector<std::vector<float>>& time_vect,
                                     const std::vector<int> param_time_vect_ind,
                                     const std::vector<std::vector<float>>& param_data)
{
    float nan_val = NAN;
    int n_tstep = time_vect[0].size();
    int nparam = param_name_list.size();

    std::vector<float> param_val(nparam, 0.0);
    std::vector<std::vector<float>> param_vect;

    std::vector<float> derived_vect(n_tstep, nan_val);

    float max_time_calc = time_vect[0].back();

    for (size_t n = 1; n < time_vect.size(); n++)
        if (time_vect[n].back() < max_time_calc)
            max_time_calc = time_vect[n].back();

    int t_max;

    if (max_time_calc == time_vect[0].back())
        t_max = time_vect[0].size();
    else
        for (auto it = time_vect[0].begin(); it != time_vect[0].end(); ++it) {
            if (*it > max_time_calc) {
                --it;
                t_max = std::distance(time_vect[0].begin(), it) + 1;
                break;
            }
        }


    for (int t = 0; t < t_max; t++){

        for (int p = 0; p < nparam; p++){

            if (param_time_vect_ind[p] == 0){
                param_val[p] = param_data[p][t];
            } else {

                int t_ind = param_time_vect_ind[p];
                int t1;

                auto it = std::find(time_vect[t_ind].begin(), time_vect[t_ind].end(), time_vect[0][t]);

                if (it == time_vect[t_ind].end()) {

                    for (auto it = time_vect[t_ind].begin(); it != time_vect[t_ind].end(); ++it) {
                        if (*it >= time_vect[0][t]) {
                            t1 = std::distance(time_vect[t_ind].begin(), it);
                            break;
                        }
                    }

                    float tm1 = time_vect[t_ind][t1-1];
                    float tm2 = time_vect[t_ind][t1];
                    float v1 = param_data[p][t1-1];
                    float v2 = param_data[p][t1];

                    param_val[p] = v1 + (v2 - v1)/(tm2 - tm1)*(time_vect[0][t] - tm1);

                } else {
                    t1 = std::distance(time_vect[t_ind].begin(), it);
                    param_val[p] = param_data[p][t1];
                }

            }
        }

        param_vect.push_back(param_val);
    }

    calc_math_expr(derived_vect, expr, param_name_list, param_vect);


    return derived_vect;
}

int DerivedSmry::replace_all(std::string& line, const std::string& repstr1, const std::string& repstr2, const std::string& newstr)
{
    int count = 0;

    for (auto& repstr : {
                repstr1, repstr2
            }) {

        int p1 = line.find(repstr);

        while ( p1 != std::string::npos ) {

            line.replace(p1, repstr.size(), newstr);
            count ++;

            p1 = line.find(repstr);
        }
    }

    return count;
}



void DerivedSmry::calc_math_expr(std::vector<float>& derived_vect, const std::string& expr,
                                 const std::vector<std::string>& param_name_list,
                                 const std::vector<std::vector<float>>& param_vect)
{
    int t_max = param_vect.size();
    int nparam = param_name_list.size();

    std::vector<float> param_val(nparam, 0.0);

    typedef exprtk::symbol_table<float> symbol_table_t;
    typedef exprtk::expression<float>   expression_t;
    typedef exprtk::parser<float>       parser_t;

    symbol_table_t symbol_table;

    for (size_t n = 0; n < nparam; n++)
        symbol_table.add_variable(param_name_list[n], param_val[n]);

    expression_t expression;
    expression.register_symbol_table(symbol_table);

    parser_t parser;
    parser.compile(expr, expression);

    for (int t = 0; t < t_max; t++){

        for (int p = 0; p < nparam; p++)
            param_val[p] = param_vect[t][p];

        derived_vect[t] = expression.value();
    }
}

void DerivedSmry::calc_derived_smry(const std::vector<FileType>& file_type,
                std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>>& esmry_loader,
                std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>>& lodsmry_loader)
{
    //m_smry_data.clear();
    //m_unit_list.clear();

    for (auto& define: m_define_table){

        var_type var = std::get<0>(define);
        param_list_type param_list = std::get<1>(define);
        std::string expr_str = std::get<2>(define);

        int var_smry_id = std::get<0>(var);
        std::string var_smry_key = std::get<1>(var);
        std::string unit = std::get<2>(var);

        std::vector<std::vector<float>> time_vect;
        std::vector<int> time_vect_smry_id_list = {var_smry_id};
        std::vector<int> param_time_vect_ind;

        if (var_smry_id < 0)
            time_vect.push_back(m_smry_data.at({-1, "TIME"}));
        else if (file_type[var_smry_id] == FileType::SMSPEC)
            time_vect.push_back(esmry_loader[var_smry_id]->get("TIME"));
        else if (file_type[var_smry_id] == FileType::ESMRY)
            time_vect.push_back(lodsmry_loader[var_smry_id]->get("TIME"));
        else
            throw std::invalid_argument("in calculate routine, unknown file type");

        std::vector<std::vector<float>> param_data;
        std::vector<std::string> param_name_list;

        for (auto& param : param_list) {
            param_name_list.push_back(std::get<0>(param));

            int smry_id = std::get<1>(param);
            std::string smry_key = std::get<2>(param);

            std::vector<float> smry_vect;

            if (is_derived(smry_id, smry_key))
                smry_vect = get(smry_id, smry_key);
            else if (file_type[smry_id] == FileType::SMSPEC)
                smry_vect = esmry_loader[smry_id]->get(smry_key);
            else if (file_type[smry_id] == FileType::ESMRY)
                smry_vect = lodsmry_loader[smry_id]->get(smry_key);
            else
                throw std::invalid_argument("in calculate routine, unknown file type");

            auto it = std::find(time_vect_smry_id_list.begin(), time_vect_smry_id_list.end(), smry_id);

            if (it == time_vect_smry_id_list.end()) {

                param_time_vect_ind.push_back(time_vect_smry_id_list.size());
                time_vect_smry_id_list.push_back(smry_id);

                if (file_type[smry_id] == FileType::SMSPEC)
                    time_vect.push_back(esmry_loader[smry_id]->get("TIME"));
                else if (file_type[smry_id] == FileType::ESMRY)
                    time_vect.push_back(lodsmry_loader[smry_id]->get("TIME"));
                else
                    throw std::invalid_argument("in calculate routine, unknown file type");

            } else {
                int index = std::distance(time_vect_smry_id_list.begin(), it);
                param_time_vect_ind.push_back(index);
            }

            param_data.push_back(smry_vect);
        }

        int ant = replace_all(expr_str, "NaN", "NAN", "0.0/0.0");
        auto derived_vect = calc_derived_vect(expr_str, param_name_list, time_vect, param_time_vect_ind, param_data);

        std::tuple<int, std::string> smry_key = std::make_tuple(var_smry_id, var_smry_key);

        m_smry_data.insert({smry_key, derived_vect});

        if (unit == "None")
            m_unit_list.insert({smry_key, ""});
        else
            m_unit_list.insert({smry_key, unit});
    }
}

const std::vector<float>& DerivedSmry::get(int smry_id, const std::string& name) const
{
    std::tuple<int, std::string> key = {smry_id, name};

    if (m_smry_data.count(key) == 0)
        throw std::runtime_error("key " + std::to_string(smry_id) + ":" + name + " not found in derived smry");

    return m_smry_data.at(key);
}

const std::string& DerivedSmry::get_unit(int smry_id, const std::string& name) const
{
    std::tuple<int, std::string> key = {smry_id, name};

    if (m_unit_list.count(key) == 0)
        throw std::runtime_error("key " + std::to_string(smry_id) + ":" + name + " not found in derived smry unit list");

    return m_unit_list.at(key);
}


