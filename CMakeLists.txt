cmake_minimum_required(VERSION 3.8)
project(VLBI_scheduler)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES
        AbstractAntenna.cpp AbstractAntenna.h
        Baseline.cpp Baseline.h
        AstronomicalParameters.cpp AstronomicalParameters.h
        AbstractCableWrap.cpp AbstractCableWrap.h
        CalibratorBlock.cpp CalibratorBlock.h
        Constants.h
        Equipment.cpp Equipment.h
        StationEndposition.cpp StationEndposition.h
        AbstractFlux.cpp AbstractFlux.h
        AbstractHorizonMask.cpp AbstractHorizonMask.h
        Initializer.cpp Initializer.h
        LookupTable.cpp LookupTable.h
        main.cpp
        MultiScheduling.cpp MultiScheduling.h
        ObservationMode.cpp ObservationMode.h
        Output.cpp Output.h
        ParameterGroup.cpp ParameterGroup.h
        ParameterSettings.cpp ParameterSettings.h
        ParameterSetup.cpp ParameterSetup.h
        PointingVector.cpp PointingVector.h
        Position.cpp Position.h
        README.md
        Scan.cpp Scan.h
        ScanTimes.cpp ScanTimes.h
        Scheduler.cpp Scheduler.h
        SkdCatalogReader.cpp SkdCatalogReader.h
        SkyCoverage.cpp SkyCoverage.h
        sofa.h sofam.h
        Source.cpp Source.h
        Station.cpp Station.h
        Subcon.cpp Subcon.h
        TimeSystem.cpp TimeSystem.h
        VieSchedpp.h VieSchedpp.cpp
        WeightFactors.cpp WeightFactors.h
        Vex.cpp Vex.h
        Skd.cpp Skd.h
        SkdParser.cpp SkdParser.h
        LogParser.cpp LogParser.h
        VieVS_Object.cpp
        VieVS_Object.h
        VieVS_NamedObject.cpp
        VieVS_NamedObject.h
        Flux_B.cpp Flux_B.h
        Flux_M.cpp Flux_M.h
        Antenna_AzEl.cpp Antenna_AzEl.h
        Antenna_HaDc.cpp Antenna_HaDc.h
        Antenna_XYew.cpp Antenna_XYew.h
        CableWrap_AzEl.cpp CableWrap_AzEl.h
        CableWrap_HaDc.cpp CableWrap_HaDc.h
        CableWrap_XYew.cpp CableWrap_XYew.h
        Equipment_elDependent.cpp Equipment_elDependent.h
        HorizonMask_line.cpp HorizonMask_line.h
        HorizonMask_step.cpp HorizonMask_step.h
        HighImpactScanDescriptor.cpp HighImpactScanDescriptor.h
        Subnetting.h
        util.h util.cpp
        Network.cpp Network.h
        Observation.cpp Observation.h
        Flags.h Flags.cpp)

add_executable(VieSchedpp ${SOURCE_FILES})

#############################################################################################
########################################## OpenMP ###########################################
#############################################################################################

find_package(OpenMP)
if (OPENMP_FOUND)
    message("OpenMP found")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

#############################################################################################
########################################### BOOST ###########################################
#############################################################################################
SET(BOOST_ROOT "C:/boost")

##### uncommend the following lines to use boost libraries and therefore enable logging #####
#                                                                                           #
add_definitions( -DVIESCHEDPP_LOG )                                                        #
SET(Boost_USE_STATIC_LIBS     ON)                                                          #
find_package(Boost COMPONENTS log log_setup system REQUIRED)                               #
#                                                                                           #
######################################## end of block #######################################

# OR:

#### uncommend the following lines to only use header only libraries and  disable logging ###
#                                                                                           #
# find_package(Boost  REQUIRED)                                                             #
#                                                                                           #
######################################## end of block #######################################


if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})

    message("Boost VERSION: ${Boost_VERSION}")
    message("Boost INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
    message("Boost Boost_LIBRARY_DIRS: ${Boost_LIBRARY_DIRS}")
    message("Boost LIBRARIES: ${Boost_LIBRARIES}")
    TARGET_LINK_LIBRARIES(VieSchedpp ${Boost_LIBRARIES})
endif ()


#############################################################################################
########################################### SOFA ############################################
#############################################################################################

############ uncommend the following lines to manually link SOFA to your project ############
#                                                                                           #
#FIND_LIBRARY(SOFA libsofa_c.lib C:/Users/matth/CLionProjects/sofa_test_v2 REQUIRED)        #
#if (SOFA)                                                                                  #
#    message("SOFA found here: ${SOFA}")                                                    #
#    TARGET_LINK_LIBRARIES(VLBI_scheduler ${SOFA})                                          #
#endif()                                                                                    #
#                                                                                           #
######################################## end of block #######################################

# OR:

######### uncommend the following lines to automatically look for SOFA and link it ##########
#                                                                                           #
FIND_LIBRARY(SOFA_LNX libsofa_c.a ./)                                                      #
FIND_LIBRARY(SOFA_WIN libsofa_c.lib ./)                                                    #
if (SOFA_LNX)                                                                              #
    message("SOFA library found: ${SOFA_LNX}")                                             #
    TARGET_LINK_LIBRARIES(VieSchedpp ${SOFA_LNX})                                          #
elseif(SOFA_WIN)                                                                           #
    message("SOFA library found: ${SOFA_WIN}")                                             #
    TARGET_LINK_LIBRARIES(VieSchedpp ${SOFA_WIN})                                          #
else()                                                                                     #
    message("ERROR: SOFA library not found!")                                              #
endif ()                                                                                   #
#                                                                                           #
######################################## end of block #######################################
