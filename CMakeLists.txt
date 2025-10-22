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
 project(VieSchedpp LANGUAGES CXX C)
 message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
 message(STATUS "CMake version: ${CMAKE_VERSION}")
 message(STATUS "CMake generator: ${CMAKE_GENERATOR}")
 message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
 message(STATUS "C   compiler: ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")

 # ------------------------------------------------------------------------------
 # C++ standard settings
 # ------------------------------------------------------------------------------
 set(CMAKE_CXX_STANDARD 17)
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
         ${MINIZ_SRC}
 )

 option(WITH_COMPRESSION "Build with miniz support for compression" ON)
 if (WITH_COMPRESSION)
     add_library(miniz STATIC
             ${CMAKE_SOURCE_DIR}/miniz/miniz.c
     )
     target_include_directories(miniz PUBLIC ${CMAKE_SOURCE_DIR}/miniz)
     target_compile_definitions(VieSchedpp PRIVATE COMPRESSION_ENABLED)
     target_link_libraries(VieSchedpp PRIVATE miniz)
     message(STATUS "Building with miniz compression support")
 endif()

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

 if (MINGW)
     message(STATUS "Add explicit OpenMP flags for MinGW")
     # Some MinGW builds need explicit OpenMP flag at link time
     target_compile_options(VieSchedpp PRIVATE -fopenmp)
     target_link_options(VieSchedpp PRIVATE -fopenmp)
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
     if (WIN32)
         set(Boost_USE_STATIC_LIBS ON)
     else()
         set(Boost_USE_STATIC_LIBS OFF)
     endif()
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
 # Generate version header dynamically from Git (cross-platform)
 # ------------------------------------------------------------------------------

 # Path to the generated version header
 set(VERSION_HEADER "${CMAKE_BINARY_DIR}/generated/version.h")

 # Ensure the output directory exists
 file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/generated")

 # Try to get the latest commit date (formatted like v.YYYY-MM-DD)
 execute_process(
         COMMAND git -C "${CMAKE_SOURCE_DIR}" log -1 --date=format:%Y-%m-%d --format=v.%cd
         OUTPUT_VARIABLE VIESCHEDPP_VERSION
         OUTPUT_STRIP_TRAILING_WHITESPACE
         ERROR_QUIET
 )

 # Fallback if git is not available
 if (NOT VIESCHEDPP_VERSION)
     set(VIESCHEDPP_VERSION "v.unknown")
     message(STATUS "GIT not found -> no version could be extracted: ${VIESCHEDPP_VERSION}")
 else()
     message(STATUS "Extracted VieSched++ version: ${VIESCHEDPP_VERSION}")
 endif()

 # --- Determine the correct Git dependency file ---
 # Get the reference file HEAD points to (e.g. refs/heads/main)
 execute_process(
         COMMAND git -C "${CMAKE_SOURCE_DIR}" rev-parse --symbolic-full-name HEAD
         OUTPUT_VARIABLE GIT_HEAD_REF
         OUTPUT_STRIP_TRAILING_WHITESPACE
         ERROR_QUIET
 )

 # Default to .git/HEAD (for detached HEAD or shallow clones)
 set(GIT_DEP "${CMAKE_SOURCE_DIR}/.git/HEAD")

 # If HEAD points to a ref file (like refs/heads/main), use that instead
 if (EXISTS "${CMAKE_SOURCE_DIR}/.git/${GIT_HEAD_REF}")
     set(GIT_DEP "${CMAKE_SOURCE_DIR}/.git/${GIT_HEAD_REF}")
 endif()

 # --- Generate the version header ---
 add_custom_command(
         OUTPUT "${VERSION_HEADER}"
         COMMAND ${CMAKE_COMMAND} -E echo "/* Auto-generated file */" > "${VERSION_HEADER}"
         COMMAND ${CMAKE_COMMAND} -E echo "#pragma once" >> "${VERSION_HEADER}"
         COMMAND ${CMAKE_COMMAND} -E echo "#define VIESCHEDPP_VERSION \"${VIESCHEDPP_VERSION}\"" >> "${VERSION_HEADER}"
         DEPENDS "${GIT_DEP}"
         COMMENT "Generating VieSched++ version header: ${VIESCHEDPP_VERSION}"
         VERBATIM
 )

 # --- Target that ensures the header is always up to date ---
 add_custom_target(generate_version ALL
         DEPENDS "${VERSION_HEADER}"
 )

 # Ensure the main project depends on the generated version file
 add_dependencies(${PROJECT_NAME} generate_version)

 # Make the generated header available for includes
 target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_BINARY_DIR}/generated")
