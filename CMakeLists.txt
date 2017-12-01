cmake_minimum_required(VERSION 3.8)
project(VLBI_scheduler)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES
        Antenna.cpp
        Antenna.h
        Baseline.cpp
        Baseline.h
        CableWrap.cpp
        CableWrap.h
        CalibratorBlock.cpp
        CalibratorBlock.h
        Constants.h
        Earth.cpp
        Earth.h
        Equipment.cpp
        Equipment.h
        FillinmodeEndposition.cpp
        FillinmodeEndposition.h
        Flux.cpp
        Flux.h
        HorizonMask.cpp
        HorizonMask.h
        Initializer.cpp
        Initializer.h
        libsofa_c.a
        LookupTable.cpp
        LookupTable.h
        main.cpp
        MultiScheduling.cpp
        MultiScheduling.h
        Nutation.cpp
        Nutation.h
        ObservationMode.cpp
        ObservationMode.h
        Output.cpp
        Output.h
        ParameterGroup.cpp
        ParameterGroup.h
        ParameterSettings.cpp
        ParameterSettings.h
        ParameterSetup.cpp
        ParameterSetup.h
        PointingVector.cpp
        PointingVector.h
        Position.cpp
        Position.h
        README.md
        Scan.cpp
        Scan.h
        ScanTimes.cpp
        ScanTimes.h
        Scheduler.cpp
        Scheduler.h
        SkdCatalogReader.cpp
        SkdCatalogReader.h
        SkyCoverage.cpp
        SkyCoverage.h
        sofa.h
        sofam.h
        Source.cpp
        Source.h
        Station.cpp
        Station.h
        Subcon.cpp
        Subcon.h
        TimeSystem.cpp
        TimeSystem.h
        WeightFactors.cpp
        WeightFactors.h)

add_executable(VLBI_scheduler ${SOURCE_FILES})

############################## OpenMP ###################################
find_package(OpenMP)
if (OPENMP_FOUND)
    message("OpenMP found")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

############################## BOOST ###################################

#SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "/home/mschartn/boost_1_64_0")
#SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "/home/mschartn/boost_1_64_0")

SET(BOOST_ROOT "/home/mschartn/boost_1_64_0")
#SET(BOOST_ROOT "C:/boost")

#SET(Boost_DEBUG ON)
#SET(Boost_DETAILED_FAILURE_MSG ON)

#SET(Boost_NO_SYSTEM_PATHS OFF)
#SET(Boost_USE_MULTITHREADED ON)
#SET(Boost_USE_STATIC_LIBS OFF)
#SET(Boost_USE_STATIC_RUNTIME OFF)
#SET(Boost_ALL_DYN_LINK OFF)

find_package(Boost 1.64.0 REQUIRED)
#find_package(BOOST 1.64.0 COMPONENTS system filesystem)

if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})

    message("Boost VERSION: ${Boost_VERSION}")
    message("Boost INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
    message("Boost Boost_LIBRARY_DIRS: ${Boost_LIBRARY_DIRS}")
    message("Boost LIBRARIES: ${Boost_LIBRARIES}")
    TARGET_LINK_LIBRARIES(VLBI_scheduler ${Boost_LIBRARIES})
endif ()


############################## SOFA ###################################

FIND_LIBRARY(SOFA libsofa_c.a /home/mschartn/SOFA/c/src REQUIRED)
#FIND_LIBRARY(SOFA libsofa_c.lib C:/Users/matth/CLionProjects/sofa_test_v2 REQUIRED)
if (SOFA)
    message("SOFA found here: ${SOFA}")
    TARGET_LINK_LIBRARIES(VLBI_scheduler ${SOFA})
endif ()
