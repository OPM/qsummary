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

#include <tests/qsum_test_utility.hpp>

//#include <algorithm>
//#include <omp.h>
#include <iostream>


SmryAppl::loader_list_type QSum::make_loaders(std::vector<std::string>& fname_list)
{
    SmryAppl::loader_list_type loaders;

    std::vector<std::filesystem::path> smry_files;
    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>> esmry_loader;
    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>> ext_esmry_loader;

    std::vector<FileType> ftype_list;

    for (auto& filename : fname_list)
       smry_files.push_back(filename);

    for (auto& smry_file: smry_files){
        std::string ext = smry_file.extension().string();
        if (ext == ".SMSPEC")
            ftype_list.push_back(FileType::SMSPEC);
        else if (ext == ".ESMRY")
            ftype_list.push_back(FileType::ESMRY);
        else
            throw std::invalid_argument("Invalid file type");
    }

    for (size_t n = 0; n < fname_list.size(); n++){
        if (ftype_list[n] == FileType::SMSPEC)
            esmry_loader[n] = std::make_unique<Opm::EclIO::ESmry>(fname_list[n]);
        else if (ftype_list[n] == FileType::ESMRY)
            ext_esmry_loader[n] = std::make_unique<Opm::EclIO::ExtESmry>(fname_list[n]);
        else
            throw std::invalid_argument("Invalid file type");
    }

    loaders = std::make_tuple(smry_files, ftype_list, std::move(esmry_loader), std::move(ext_esmry_loader));

    return loaders;
}

void QSum::add_vect_cmd_line(const std::string& vect_cmd, QLineEdit* cmdline )
{
    for (size_t n = 0; n < vect_cmd.size(); n++)
        QTest::keyClicks(cmdline, QString::fromStdString(vect_cmd.substr(n,1)));

    QTest::keyEvent(QTest::Click, cmdline, Qt::Key_Return );
}
