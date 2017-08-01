cmake_minimum_required(VERSION 3.7)
project(VieVS_Scheduler_clion)
set(CMAKE_CXX_STANDARD 14)


#SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "C:/boost")
#SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "C:/boost")
SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "/home/mschartn/boost_1_64_0")
SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "/home/mschartn/boost_1_64_0")

find_package(Boost 1.64.0 REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

set(SOURCE_FILES
        main.cpp
        VieVS_constants.h
        VLBI_antenna.cpp
        VLBI_antenna.h
        VLBI_baseline.cpp
        VLBI_baseline.h
        VLBI_cableWrap.cpp
        VLBI_cableWrap.h
        VLBI_equip.cpp
        VLBI_equip.h
        VLBI_flux.cpp
        VLBI_flux.h
        VLBI_initializer.cpp
        VLBI_initializer.h
        VLBI_mask.cpp
        VLBI_mask.h
        VLBI_obs.cpp
        VLBI_obs.h
        VLBI_pointingVector.cpp
        VLBI_pointingVector.h
        VLBI_position.cpp
        VLBI_position.h
        VLBI_scan.cpp
        VLBI_scan.h
        VLBI_scheduler.cpp
        VLBI_scheduler.h
        VLBI_skyCoverage.cpp
        VLBI_skyCoverage.h
        VLBI_source.cpp
        VLBI_source.h
        VLBI_station.cpp
        VLBI_station.h
        VLBI_subcon.cpp
        VLBI_subcon.h
        sofa.h
        sofam.h VLBI_scanTimes.cpp VLBI_scanTimes.h VieVS_nutation.cpp VieVS_nutation.h VieVS_earth.cpp VieVS_earth.h VieVS_unordered_flat_map.cpp VieVS_unordered_flat_map.h)

add_executable(VieVS_Scheduler_clion ${SOURCE_FILES})

# FIND_LIBRARY(SOFA libsofa_c.lib C:/Users/matth/CLionProjects/sofa_test_v2 REQUIRED)
FIND_LIBRARY(SOFA libsofa_c.a /home/mschartn/SOFA/c/src REQUIRED)
TARGET_LINK_LIBRARIES(VieVS_Scheduler_clion ${SOFA})
