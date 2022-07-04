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

add_library(smry_appl
   appl/smry_appl.cpp
   appl/smry_xaxis.cpp
   appl/smry_yaxis.cpp
   appl/smry_series.cpp
   appl/chartview.cpp
   appl/point_info.cpp
   appl/qsum_cmdf.cpp
  )

add_executable(qsummary main.cpp)

target_link_libraries(smry_appl opmcommon Qt5::Widgets Qt5::Core Qt5::Charts stdc++fs  OpenMP::OpenMP_CXX)

target_link_libraries(qsummary smry_appl ${Boost_LIBRARIES} Qt5::Widgets Qt5::Core Qt5::Charts OpenMP::OpenMP_CXX)


