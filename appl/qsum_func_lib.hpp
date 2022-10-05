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



#ifndef QSUM_FUNCLIB_HPP
#define QSUM_FUNCLIB_HPP

#include <appl/smry_appl.hpp>

#include <opm/io/eclipse/ESmry.hpp>
#include <opm/io/eclipse/ExtESmry.hpp>


namespace QSum {

void chart_input_from_string(std::string& vect_string,
                             SmryAppl::input_list_type& input_charts,
                             const std::vector<FileType>& file_type,
                             std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>>& esmry_loader,
                             std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>>& lodsmry_loader,
                             const int max_number_of_charts,
                             const std::string& xrange
                            );

void update_input(SmryAppl::input_list_type& input_charts,
                  const std::vector<std::string>& keyw_list,
                  const std::vector<FileType>& file_type,
                  std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>>& esmry_loader,
                  std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>>& lodsmry_loader,
                  const int max_number_of_charts,
                  const std::string& xrange
                 );

void pre_load_smry(const std::vector<std::filesystem::path>& smry_files,
                   const SmryAppl::input_list_type& input_charts,
                   const std::vector<FileType>& file_type,
                   std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>>& esmry_loader,
                   std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>>& lodsmry_loader,
                   const size_t nthreads);

void remove_zero_vect(const std::vector<std::filesystem::path>& smry_files,
                      SmryAppl::input_list_type& input_charts,
                      const std::vector<FileType>& file_type,
                      std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>>& esmry_loader,
                      std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>>& lodsmry_loader);


SmryAppl::input_list_type charts_separate_folders(const std::vector<std::filesystem::path>& smry_files,
        SmryAppl::input_list_type input_charts);


template <typename T>
bool double_check_well_vector(std::string& vect_name, std::unique_ptr<T>& smry);

void check_summary_vectors(SmryAppl::input_list_type& input_charts,
                           const std::vector<FileType>& file_type,
                           std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>>& esmry_loader,
                           std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>>& lodsmry_loader);



template <typename T>
bool has_nonzero(std::unique_ptr<T>& smry, const std::string key);

void print_input_charts(const SmryAppl::input_list_type& input_charts);

} // namespace QSum

#endif // QSUM_FUNCLIB_HPP
