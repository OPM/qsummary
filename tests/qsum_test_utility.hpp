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



#ifndef QSUM_TESTUTIL_HPP
#define QSUM_TESTUTIL_HPP

#include <appl/smry_appl.hpp>

#include <opm/io/eclipse/ESmry.hpp>
#include <opm/io/eclipse/ExtESmry.hpp>

#include <QtTest/QtTest>


namespace QSum {

    SmryAppl::loader_list_type make_loaders(std::vector<std::string>& fname_list);

    void add_cmd_line(const std::string& vect_cmd, QLineEdit* cmdline );




} // namespace QSum

#endif // QSUM_TESTUTIL_HPP
