 # 
 #  VieSched++ Very Long Baseline Interferometry (VLBI) Scheduling Software
 #  Copyright (C) 2018  Matthias Schartner
 #
 #  This program is free software: you can redistribute it and/or modify
 #  it under the terms of the GNU General Public License as published by
 #  the Free Software Foundation, either version 3 of the License, or
 #  (at your option) any later version.
 #
 #  This program is distributed in the hope that it will be useful,
 #  but WITHOUT ANY WARRANTY; without even the implied warranty of
 #  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 #  GNU General Public License for more details.
 #
 #  You should have received a copy of the GNU General Public License
 #  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 #
 
cmake_minimum_required(VERSION 3.8)
project(VieSchedpp)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES
        Station/Antenna/AbstractAntenna.cpp Station/Antenna/AbstractAntenna.h
        Station/Baseline.cpp Station/Baseline.h
        Misc/AstronomicalParameters.cpp Misc/AstronomicalParameters.h
        Station/CableWrap/AbstractCableWrap.cpp Station/CableWrap/AbstractCableWrap.h
        Misc/CalibratorBlock.cpp Misc/CalibratorBlock.h
        Misc/Constants.h
        Station/Equip/Equipment.cpp Station/Equip/Equipment.h
        Misc/StationEndposition.cpp Misc/StationEndposition.h
        Source/Flux/AbstractFlux.cpp Source/Flux/AbstractFlux.h
        Station/HorizonMask/AbstractHorizonMask.cpp Station/HorizonMask/AbstractHorizonMask.h
        Initializer.cpp Initializer.h
        Misc/LookupTable.cpp Misc/LookupTable.h
        main.cpp
        Misc/MultiScheduling.cpp Misc/MultiScheduling.h
        Output/Output.cpp Output/Output.h
        XML/ParameterGroup.cpp XML/ParameterGroup.h
        XML/ParameterSettings.cpp XML/ParameterSettings.h
        XML/ParameterSetup.cpp XML/ParameterSetup.h
        Scan/PointingVector.cpp Scan/PointingVector.h
        Station/Position.cpp Station/Position.h
        README.md
        Scan/Scan.cpp Scan/Scan.h
        Scan/ScanTimes.cpp Scan/ScanTimes.h
        Scheduler.cpp Scheduler.h
        Input/SkdCatalogReader.cpp Input/SkdCatalogReader.h
        Station/SkyCoverage.cpp Station/SkyCoverage.h
        Misc/sofa.h Misc/sofam.h
        Source/Source.cpp Source/Source.h
        Station/Station.cpp Station/Station.h
        Scan/Subcon.cpp Scan/Subcon.h
        Misc/TimeSystem.cpp Misc/TimeSystem.h
        VieSchedpp.h VieSchedpp.cpp
        Misc/WeightFactors.cpp Misc/WeightFactors.h
        Output/Vex.cpp Output/Vex.h
        Output/Skd.cpp Output/Skd.h
        Input/SkdParser.cpp Input/SkdParser.h
        Input/LogParser.cpp Input/LogParser.h
        Misc/VieVS_Object.cpp
        Misc/VieVS_Object.h
        Misc/VieVS_NamedObject.cpp
        Misc/VieVS_NamedObject.h
        Source/Flux/Flux_B.cpp Source/Flux/Flux_B.h
        Source/Flux/Flux_M.cpp Source/Flux/Flux_M.h
        Station/Antenna/Antenna_AzEl.cpp Station/Antenna/Antenna_AzEl.h
        Station/Antenna/Antenna_HaDc.cpp Station/Antenna/Antenna_HaDc.h
        Station/Antenna/Antenna_XYew.cpp Station/Antenna/Antenna_XYew.h
        Station/CableWrap/CableWrap_AzEl.cpp Station/CableWrap/CableWrap_AzEl.h
        Station/CableWrap/CableWrap_HaDc.cpp Station/CableWrap/CableWrap_HaDc.h
        Station/CableWrap/CableWrap_XYew.cpp Station/CableWrap/CableWrap_XYew.h
        Station/Equip/Equipment_elDependent.cpp Station/Equip/Equipment_elDependent.h
        Station/HorizonMask/HorizonMask_line.cpp Station/HorizonMask/HorizonMask_line.h
        Station/HorizonMask/HorizonMask_step.cpp Station/HorizonMask/HorizonMask_step.h
        Misc/HighImpactScanDescriptor.cpp Misc/HighImpactScanDescriptor.h
        Misc/Subnetting.h
        Misc/util.h Misc/util.cpp
        Station/Network.cpp Station/Network.h
        Scan/Observation.cpp Scan/Observation.h
        Misc/Flags.h Misc/Flags.cpp
        ObservingMode/Mode.cpp ObservingMode/Mode.h
        ObservingMode/Bbc.cpp ObservingMode/Bbc.h
        ObservingMode/If.cpp ObservingMode/If.h
        ObservingMode/Track.cpp ObservingMode/Track.h
        ObservingMode/Freq.cpp ObservingMode/Freq.h ObservingMode/ObservingMode.cpp ObservingMode/ObservingMode.h Output/SNR_table.cpp Output/SNR_table.h Output/OperationNotes.cpp Output/OperationNotes.h)

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
# SET(BOOST_ROOT "C:/boost")

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

######### uncommend the following lines to automatically look for SOFA and link it ##########
#                                                                                           #
FIND_LIBRARY(SOFA                                                                          #
        NAMES libsofa_c.a libsofa_c.lib                                                    #
        HINTS "../IAU_SOFA/Release/")                                                      #
if (SOFA)                                                                                  #
    message("SOFA library found: ${SOFA}")                                                 #
    TARGET_LINK_LIBRARIES(VieSchedpp ${SOFA})                                              #
else()                                                                                     #
    message("ERROR: SOFA library not found!")                                              #
endif()                                                                                    #
#                                                                                           #
######################################## end of block #######################################


#############################################################################################
###################################### VERSION NUMBER #######################################
#############################################################################################
#                                                                                           #
if(EXISTS "${CMAKE_SOURCE_DIR}/.git")                                                      #
    execute_process(                                                                       #
        COMMAND git log -1 --format=%H                                                     #
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}                                              #
        OUTPUT_VARIABLE GIT_COMMIT_HASH                                                    #
        OUTPUT_STRIP_TRAILING_WHITESPACE                                                   #
    )                                                                                      #
else()                                                                                     #
    set(GIT_COMMIT_HASH "unknown")                                                         #
endif()                                                                                    #
                                                                                           #
message("VieSched++ version: ${GIT_COMMIT_HASH}")                                          #
                                                                                           #
add_definitions("-DGIT_COMMIT_HASH=\"${GIT_COMMIT_HASH}\"")                                #
#                                                                                           #                                                                                            #
#############################################################################################