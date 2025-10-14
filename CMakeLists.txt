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
 project(VieSchedpp LANGUAGES CXX)
 message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

 # ------------------------------------------------------------------------------
 # C++ standard settings
 # ------------------------------------------------------------------------------
 set(CMAKE_CXX_STANDARD 14)
 set(CMAKE_CXX_STANDARD_REQUIRED ON)
 set(CMAKE_CXX_EXTENSIONS OFF)

 # ------------------------------------------------------------------------------
 # Create target
 # ------------------------------------------------------------------------------
 add_executable(VieSchedpp)

 # ------------------------------------------------------------------------------
 # Source files
 # ------------------------------------------------------------------------------
 file(GLOB ROOT_SRC "${CMAKE_SOURCE_DIR}/*.cpp" "${CMAKE_SOURCE_DIR}/*.h")
 file(GLOB_RECURSE ALGORITHM_SRC "${CMAKE_SOURCE_DIR}/Algorithm/*.cpp" "${CMAKE_SOURCE_DIR}/Algorithm/*.h")
 file(GLOB_RECURSE STATION_SRC "${CMAKE_SOURCE_DIR}/Station/*.cpp" "${CMAKE_SOURCE_DIR}/Station/*.h")
 file(GLOB_RECURSE SOURCE_SRC "${CMAKE_SOURCE_DIR}/Source/*.cpp" "${CMAKE_SOURCE_DIR}/Source/*.h")
 file(GLOB_RECURSE SCAN_SRC "${CMAKE_SOURCE_DIR}/Scan/*.cpp" "${CMAKE_SOURCE_DIR}/Scan/*.h")
 file(GLOB_RECURSE MISC_SRC "${CMAKE_SOURCE_DIR}/Misc/*.cpp" "${CMAKE_SOURCE_DIR}/Misc/*.h")
 file(GLOB_RECURSE INPUT_SRC "${CMAKE_SOURCE_DIR}/Input/*.cpp" "${CMAKE_SOURCE_DIR}/Input/*.h")
 file(GLOB_RECURSE OUTPUT_SRC "${CMAKE_SOURCE_DIR}/Output/*.cpp" "${CMAKE_SOURCE_DIR}/Output/*.h")
 file(GLOB_RECURSE OBS_MODE_SRC "${CMAKE_SOURCE_DIR}/ObservingMode/*.cpp" "${CMAKE_SOURCE_DIR}/ObservingMode/*.h")
 file(GLOB_RECURSE SIM_SRC "${CMAKE_SOURCE_DIR}/Simulator/*.cpp" "${CMAKE_SOURCE_DIR}/Simulator/*.h")
 file(GLOB_RECURSE XML_SRC "${CMAKE_SOURCE_DIR}/XML/*.cpp" "${CMAKE_SOURCE_DIR}/XML/*.h")
 file(GLOB_RECURSE SGP4_SRC "${CMAKE_SOURCE_DIR}/SGP4/*.h")  # SGP4 is header-only

 # Add all sources to the target
 target_sources(VieSchedpp PRIVATE
         ${ROOT_SRC}
         ${ALGORITHM_SRC}
         ${STATION_SRC}
         ${SOURCE_SRC}
         ${SCAN_SRC}
         ${MISC_SRC}
         ${INPUT_SRC}
         ${OUTPUT_SRC}
         ${OBS_MODE_SRC}
         ${SIM_SRC}
         ${XML_SRC}
         ${SGP4_SRC}
 )

 # ------------------------------------------------------------------------------
 # Platform-specific flags
 # ------------------------------------------------------------------------------
 if (WIN32)
     message(STATUS "Windows build: adjusting compiler flags...")
     target_compile_options(VieSchedpp PRIVATE -Wa,-mbig-obj)
     target_link_options(VieSchedpp PRIVATE -static -static-libgcc -static-libstdc++)
 endif ()

 # ------------------------------------------------------------------------------
 # OpenMP
 # ------------------------------------------------------------------------------
 find_package(OpenMP)
 if (OpenMP_CXX_FOUND)
     message(STATUS "OpenMP found")
     target_link_libraries(VieSchedpp PRIVATE OpenMP::OpenMP_CXX)
 endif ()

 # ------------------------------------------------------------------------------
 # Eigen (simulation mode)
 # ------------------------------------------------------------------------------
 option(BUILD_SIMULATIONS "Build with Eigen simulation mode" ON)
 if (BUILD_SIMULATIONS)
     message(STATUS "Building with simulator (Eigen enabled)")
     add_subdirectory(Eigen)
     target_compile_definitions(VieSchedpp PRIVATE SIMULATOR_MODE)
 else ()
     message(STATUS "Building without simulator (Eigen disabled)")
 endif ()

 # ------------------------------------------------------------------------------
 # Boost
 # ------------------------------------------------------------------------------
 option(LINK_BOOST "Link against Boost libraries" ON)
 set(PATH_BOOST_ROOT "" CACHE PATH "Optional path to Boost installation")
 if(LINK_BOOST)
     message(STATUS "Linking against Boost libraries")
     # If user provided a custom Boost path, set the relevant variables
     if (PATH_BOOST_ROOT)
         set(Boost_ROOT "${PATH_BOOST_ROOT}")
         set(BOOST_INCLUDEDIR "${PATH_BOOST_ROOT}/include")
         set(BOOST_LIBRARYDIR "${PATH_BOOST_ROOT}/lib")
         message(STATUS "Using Boost from custom path: ${PATH_BOOST_ROOT}")
     endif ()
     find_package(Boost CONFIG REQUIRED COMPONENTS log log_setup system)
     target_link_libraries(VieSchedpp PRIVATE Boost::log Boost::log_setup Boost::system)
     target_compile_definitions(VieSchedpp PRIVATE VIESCHEDPP_LOG)
 else()
     message(STATUS "Using Boost header-only mode")
     find_package(Boost CONFIG REQUIRED)
 endif()

 if (Boost_FOUND)
     message(STATUS "Found Boost: ${Boost_VERSION}")
     message(STATUS "Boost include dirs: ${Boost_INCLUDE_DIRS}")
     if (Boost_LIBRARIES)
         message(STATUS "Boost libraries: ${Boost_LIBRARIES}")
     endif ()
 else ()
     message(FATAL_ERROR "Boost not found!")
 endif ()

 # ------------------------------------------------------------------------------
 # SOFA
 # ------------------------------------------------------------------------------
 set(PATH_IAU_SOFA "../IAU_SOFA/Release/" CACHE PATH "Path to IAU SOFA library")
 find_library(SOFA_LIB
         NAMES sofa_c libsofa_c.a libsofa_c.lib
         HINTS ${PATH_IAU_SOFA}
         NO_DEFAULT_PATH
 )
 if (SOFA_LIB)
     message(STATUS "Found SOFA library: ${SOFA_LIB}")
     target_link_libraries(${PROJECT_NAME} PRIVATE ${SOFA_LIB})
 else ()
     message(WARNING "SOFA library not found! Please set PATH_IAU_SOFA.")
 endif ()

 # ------------------------------------------------------------------------------
 # SGP4
 # ------------------------------------------------------------------------------
 set(PATH_SGP4 "../sgp4/Release/libsgp4/" CACHE PATH "Path to SGP4 library")
 find_library(SGP4_LIB
         NAMES sgp4 libsgp4.a libsgp4.lib
         HINTS ${PATH_SGP4}
         NO_DEFAULT_PATH
 )
 if (SGP4_LIB)
     message(STATUS "Found SGP4 library: ${SGP4_LIB}")
     target_link_libraries(${PROJECT_NAME} PRIVATE ${SGP4_LIB})
 else ()
     message(WARNING "SGP4 library not found! Please set PATH_SGP4.")
 endif ()

 # ------------------------------------------------------------------------------
 # Git version information
 # ------------------------------------------------------------------------------
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
 message(STATUS "VieSched++ version: ${GIT_COMMIT_HASH}")
 target_compile_definitions(VieSchedpp PRIVATE "GIT_COMMIT_HASH=\"${GIT_COMMIT_HASH}\"")
