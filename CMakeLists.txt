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

 cmake_minimum_required(VERSION 3.15)
 project(VieSchedpp)
 message(Build: ${CMAKE_BUILD_TYPE})

 set(CMAKE_CXX_STANDARD 14)

 set(SOURCE_FILES
         Station/Antenna/AbstractAntenna.cpp Station/Antenna/AbstractAntenna.h
         Station/Baseline.cpp Station/Baseline.h
         Misc/AstronomicalParameters.cpp Misc/AstronomicalParameters.h
         Station/CableWrap/AbstractCableWrap.cpp Station/CableWrap/AbstractCableWrap.h
         Misc/AstrometricCalibratorBlock.cpp Misc/AstrometricCalibratorBlock.h
         Misc/Constants.h
         Station/Equip/Equipment_constant.cpp Station/Equip/Equipment_constant.h
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
         Source/AbstractSource.cpp Source/AbstractSource.h
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
         Station/Equip/Equipment_elModel.cpp Station/Equip/Equipment_elModel.h
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
         ObservingMode/Freq.cpp ObservingMode/Freq.h
         ObservingMode/ObservingMode.cpp ObservingMode/ObservingMode.h
         Output/SNR_table.cpp Output/SNR_table.h
         Output/OperationNotes.cpp Output/OperationNotes.h
         Output/Ast.cpp Output/Ast.h
         Output/SourceStatistics.cpp Output/SourceStatistics.h
         Algorithm/FocusCorners.cpp Algorithm/FocusCorners.h
         Misc/CalibratorBlock.cpp Misc/CalibratorBlock.h
         Simulator/Simulator.cpp Simulator/Simulator.h
         Simulator/Solver.cpp Simulator/Solver.h
         Simulator/Unknown.cpp Simulator/Unknown.h
         Source/SourceList.cpp Source/SourceList.h
         Source/Quasar.cpp Source/Quasar.h
         Source/Satellite.cpp Source/Satellite.h
         Station/Antenna/Antenna_GGAO.cpp Station/Antenna/Antenna_GGAO.h
         SGP4/CoordGeodetic.h
         SGP4/Tle.h
         SGP4/TleException.h
         SGP4/DateTime.h
         SGP4/SGP4.h
         SGP4/OrbitalElements.h
         SGP4/Observer.h
         SGP4/Eci.h
         Station/Antenna/Antenna_AzEl_acceleration.cpp Station/Antenna/Antenna_AzEl_acceleration.h Input/StpParser.cpp Input/StpParser.h Station/Equip/Equipment_elTable.cpp Station/Equip/Equipment_elTable.h Station/Equip/AbstractEquipment.cpp Station/Equip/AbstractEquipment.h Source/Flux/Flux_constant.cpp Source/Flux/Flux_constant.h Station/Antenna/Antenna_ONSALA_VGOS.cpp Station/Antenna/Antenna_ONSALA_VGOS.h Misc/AvoidSatellites.cpp Misc/AvoidSatellites.h Misc/ParallacticAngleBlock.cpp Misc/ParallacticAngleBlock.h Misc/DifferentialParallacticAngleBlock.cpp Misc/DifferentialParallacticAngleBlock.h Source/Flux/Flux_satellite.cpp Source/Flux/Flux_satellite.h)

 if (WIN32)
     message("Windows build! Add some compiler flags...")
     set(CMAKE_CXX_FLAGS "-Wa,-mbig-obj")
 endif ()

 message("C++ compiler flags: ${CMAKE_CXX_FLAGS}")

 add_executable(VieSchedpp ${SOURCE_FILES})

 ###########################################################################################
 ########################################## OpenMP #########################################
 ###########################################################################################

 find_package(OpenMP)
 if (OPENMP_FOUND)
     message("OpenMP found")
     set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
     set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
 endif ()

 ###########################################################################################
 ########################################## Eigen ##########################################
 ###########################################################################################
 set(BUILD_SIMULATIONS ON)
 if (BUILD_SIMULATIONS)
     message("Building with simulations (Eigen enabled)")
     add_subdirectory(Eigen)
     add_compile_definitions(SIMULATOR_MODE)
 else ()
     message("Building without simulations (Eigen disabled)")
 endif ()

 # find_package(Eigen3 REQUIRED HINTS ./EIGEN)
 # if (Eigen3_FOUND)
 #     message("Eigen found")
 #     message("  Eigen location: ${Eigen3_DIR}")
 #     message("  Eigen Version: ${Eigen3_VERSION}")
 #     target_link_libraries (VieSchedpp Eigen3::Eigen)
 # endif ()

 ###########################################################################################
 ########################################### BOOST #########################################
 ###########################################################################################
 set(LINK_BOOST "True" CACHE STRING "link against boost libraries")

 if(LINK_BOOST)
     message("Link against BOOST libraries")
     add_definitions(-DVIESCHEDPP_LOG)
     set(Boost_USE_STATIC_LIBS ON)
     find_package(Boost CONFIG REQUIRED COMPONENTS log log_setup system)
 else()
     message("Use BOOST header-only mode")
     find_package(Boost CONFIG REQUIRED)
 endif()

 if (Boost_FOUND)
     message("Boost found:")
     message("  Boost VERSION: ${Boost_VERSION}")
     message("  Boost INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
     target_link_libraries(VieSchedpp Boost::log Boost::log_setup Boost::system)
 endif ()

 ###########################################################################################
 ########################################### SOFA ##########################################
 ###########################################################################################

 set(PATH_IAU_SOFA "../IAU_SOFA/Release/" CACHE STRING "path to folder containing IAU SOFA library")
 FIND_LIBRARY(SOFA
         NAMES libsofa_c.a
         HINTS ${PATH_IAU_SOFA})
 if (SOFA)
     message("SOFA library found")
     message("  SOFA location: ${SOFA}")
     TARGET_LINK_LIBRARIES(VieSchedpp ${SOFA})
 else ()
     message("ERROR: SOFA library not found!")
 endif ()
 ######################################## end of block #####################################


 ###########################################################################################
 ########################################### SGP4 ##########################################
 ###########################################################################################

 set(PATH_SGP4 "../sgp4/Release/libsgp4/" CACHE STRING "path to folder containing SGP4 library")
 FIND_LIBRARY(SGP4
         NAMES libsgp4.a libsgp4.lib
         HINTS ${PATH_SGP4})
 if (SGP4)
     message("SGP4 library found")
     message("  SGP4 location: ${SGP4}")
     TARGET_LINK_LIBRARIES(VieSchedpp ${SGP4})
 else ()
     message("ERROR: SGP4 library not found!")
 endif ()
 ######################################## end of block #####################################


 ###########################################################################################
 ###################################### VERSION NUMBER #####################################
 ###########################################################################################
 if (EXISTS "${CMAKE_SOURCE_DIR}/.git")
     execute_process(
             COMMAND git log -1 --date=format:%Y-%m-%d --format=v.%cd
             WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
             OUTPUT_VARIABLE GIT_COMMIT_HASH
             OUTPUT_STRIP_TRAILING_WHITESPACE
     )
 else ()
     set(GIT_COMMIT_HASH "unknown")
 endif ()
 message("VieSched++ version: ${GIT_COMMIT_HASH}")
 add_definitions("-DGIT_COMMIT_HASH=\"${GIT_COMMIT_HASH}\"")
 ###########################################################################################