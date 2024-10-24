
cmake_minimum_required(VERSION 3.0)

project(simple)

set(CMAKE_CXX_STANDARD 17)

set (opm-common_DIR "${CMAKE_SOURCE_DIR}/../opm-common/build" CACHE LOCATION "Build tree of opm-common")

find_package (opm-common)

include_directories(${CMAKE_SOURCE_DIR}
                    ${opm-common_INCLUDE_DIRS})

find_package(OpenMP REQUIRED)

set(CMAKE_AUTOMOC ON)

find_package(Qt5 5.12 REQUIRED COMPONENTS Widgets Charts Test REQUIRED)

add_library(smry_appl STATIC
   appl/smry_appl.cpp
   appl/smry_xaxis.cpp
   appl/xaxis_ticks.cpp
   appl/smry_yaxis.cpp
   appl/smry_series.cpp
   appl/chartview.cpp
   appl/point_info.cpp
   appl/qsum_cmdf.cpp
   appl/derived_smry.cpp
   appl/qsum_func_lib.cpp
  )

add_executable(qsummary main.cpp)

target_link_libraries(smry_appl opmcommon Qt5::Widgets Qt5::Core Qt5::Charts stdc++fs  OpenMP::OpenMP_CXX)

target_link_libraries(qsummary smry_appl ${Boost_LIBRARIES} Qt5::Widgets Qt5::Core Qt5::Charts OpenMP::OpenMP_CXX)


add_executable(update_ref_define ./tests/update_ref_define.cpp)

target_link_libraries(update_ref_define smry_appl ${Boost_LIBRARIES} Qt5::Widgets Qt5::Core Qt5::Charts OpenMP::OpenMP_CXX)


enable_testing(true)

add_executable(cmdf_define
    tests/test_cmdf_define.cpp
    )

add_executable(cmdf_input_chart
    tests/test_cmdf_input_charts.cpp
    )

add_executable(scale_yaxis
    tests/qsum_test_utility.cpp
    tests/test_scale_yaxis.cpp
    )

add_executable(smryappl
    tests/qsum_test_utility.cpp
    appl/qsum_func_lib.hpp
    tests/test_smry_appl.cpp
    )

add_executable(sep_folders
    appl/qsum_func_lib.hpp
    tests/test_sep_folders.cpp
    )


add_test(NAME cmdf_define COMMAND cmdf_define)
add_test(NAME cmdf_input_chart COMMAND cmdf_input_chart)
add_test(NAME scale_yaxis COMMAND scale_yaxis)
add_test(NAME smryappl COMMAND smryappl)
add_test(NAME sep_folders COMMAND sep_folders)


target_link_libraries(cmdf_define PRIVATE smry_appl Qt5::Test Qt5::Core Qt5::Widgets Qt5::Charts stdc++fs)
target_link_libraries(cmdf_input_chart PRIVATE smry_appl Qt5::Test Qt5::Core Qt5::Widgets Qt5::Charts stdc++fs)
target_link_libraries(scale_yaxis PRIVATE smry_appl Qt5::Test Qt5::Core Qt5::Widgets Qt5::Charts stdc++fs)
target_link_libraries(smryappl PRIVATE smry_appl Qt5::Test Qt5::Core Qt5::Widgets Qt5::Charts stdc++fs)
target_link_libraries(sep_folders PRIVATE smry_appl Qt5::Test Qt5::Core Qt5::Widgets Qt5::Charts stdc++fs)

