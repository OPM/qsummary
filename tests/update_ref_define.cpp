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


#include <iostream>

#include <opm/io/eclipse/ESmry.hpp>
#include <opm/io/eclipse/ExtESmry.hpp>
#include <opm/io/eclipse/EclOutput.hpp>

#include <appl/smry_appl.hpp>

//#include <tests/test_utilities.hpp>

#include <filesystem>

#include <appl/qsum_cmdf.hpp>


void add_to_file(Opm::EclIO::EclOutput& refFile, std::unique_ptr<DerivedSmry>& derived_smry, int& n)
{

    auto list = derived_smry->get_list();

    for (auto& key : list) {
        int smry_id = std::get<0>(key);
        std::string key_string = std::get<1>(key);

        std::vector<float> data = derived_smry->get(smry_id, key_string);
        std::string ref_key = "REF_" + std::to_string(n);

        std::cout << "adding smry_id " << smry_id << " key: " << key_string << " to : " << ref_key << std::endl;

        refFile.write<float>(ref_key, data);

        n++;
    }

}

int main(int argc, char *argv[])
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
        int n = 0;

        Opm::EclIO::EclOutput refFile("../tests/REF_DEFINE.DATA", false, std::ios::out);

        {
            std::string cmd_file = "../tests/cmd_files/test1a.txt";
            QsumCMDF cmdfile(cmd_file, num_files, "");

            std::unique_ptr<DerivedSmry> derived_smry;
            derived_smry = std::make_unique<DerivedSmry>(cmdfile, file_type, esmry_loader, lodsmry_loader);

            add_to_file(refFile, derived_smry, n);
        }

        {
            std::string cmd_file = "../tests/cmd_files/test1b.txt";
            QsumCMDF cmdfile(cmd_file, num_files, "");

            std::unique_ptr<DerivedSmry> derived_smry;
            derived_smry = std::make_unique<DerivedSmry>(cmdfile, file_type, esmry_loader, lodsmry_loader);

            add_to_file(refFile, derived_smry, n);
        }

        {
            std::string cmd_file = "../tests/cmd_files/test1c.txt";
            QsumCMDF cmdfile(cmd_file, num_files, "");

            std::unique_ptr<DerivedSmry> derived_smry;
            derived_smry = std::make_unique<DerivedSmry>(cmdfile, file_type, esmry_loader, lodsmry_loader);

            add_to_file(refFile, derived_smry, n);
        }

        {
            std::string cmd_file = "../tests/cmd_files/test1d.txt";
            QsumCMDF cmdfile(cmd_file, num_files, "");

            std::unique_ptr<DerivedSmry> derived_smry;
            derived_smry = std::make_unique<DerivedSmry>(cmdfile, file_type, esmry_loader, lodsmry_loader);

            add_to_file(refFile, derived_smry, n);
        }

        {
            std::string cmd_file = "../tests/cmd_files/test1e.txt";
            QsumCMDF cmdfile(cmd_file, num_files, "");

            std::unique_ptr<DerivedSmry> derived_smry;
            derived_smry = std::make_unique<DerivedSmry>(cmdfile, file_type, esmry_loader, lodsmry_loader);

            add_to_file(refFile, derived_smry, n);
        }

        {
            std::string cmd_file = "../tests/cmd_files/test1f.txt";
            QsumCMDF cmdfile(cmd_file, num_files, "");

            std::unique_ptr<DerivedSmry> derived_smry;
            derived_smry = std::make_unique<DerivedSmry>(cmdfile, file_type, esmry_loader, lodsmry_loader);

            add_to_file(refFile, derived_smry, n);
        }

        {
            std::string cmd_file = "../tests/cmd_files/test1g.txt";
            QsumCMDF cmdfile(cmd_file, num_files, "");

            std::unique_ptr<DerivedSmry> derived_smry;
            derived_smry = std::make_unique<DerivedSmry>(cmdfile, file_type, esmry_loader, lodsmry_loader);

            add_to_file(refFile, derived_smry, n);
        }

        {
            std::string cmd_file = "../tests/cmd_files/test3c.txt";
            QsumCMDF cmdfile(cmd_file, num_files, "");

            std::unique_ptr<DerivedSmry> derived_smry;
            derived_smry = std::make_unique<DerivedSmry>(cmdfile, file_type, esmry_loader, lodsmry_loader);

            add_to_file(refFile, derived_smry, n);
        }

        {
            std::string cmd_file = "../tests/cmd_files/test1h.txt";
            QsumCMDF cmdfile(cmd_file, num_files, "");

            std::unique_ptr<DerivedSmry> derived_smry;
            derived_smry = std::make_unique<DerivedSmry>(cmdfile, file_type, esmry_loader, lodsmry_loader);

            add_to_file(refFile, derived_smry, n);
        }

    }


    std::cout << "\nFinished, all good \n";
}


