/*
 *  VieSched++ Very Long Baseline Interferometry (VLBI) Scheduling Software
 *  Copyright (C) 2018  Matthias Schartner
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file MultiScheduling.h
 * @brief class MultiScheduling
 *
 * @author Matthias Schartner
 * @date 03.09.2017
 */

#ifndef MULTISCHEDULING_H
#define MULTISCHEDULING_H


#include <boost/date_time.hpp>
#include <boost/format.hpp>
#include <boost/optional.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <iterator>
#include <numeric>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

#include "../XML/ParameterGroup.h"
#include "Constants.h"
#include "VieVS_Object.h"
#include "WeightFactors.h"
#include "util.h"

#ifdef VIESCHEDPP_LOG
#include <boost/log/trivial.hpp>
#endif


namespace VieVS {
/**
 * @class MultiScheduling
 * @brief representation of VLBI multi scheduling tool
 *
 * @author Matthias Schartner
 * @date 03.09.2017
 */
class MultiScheduling : public VieVS_Object {
   public:
    /**
     * @brief possible multi scheduling parameters
     * @author Matthias Schartner
     */
    struct Parameters {
        boost::optional<boost::posix_time::ptime> start;  ///< start times

        boost::optional<bool> subnetting;                             ///< subnetting flag
        boost::optional<double> subnetting_minSourceAngle;            ///< subnetting flag
        boost::optional<double> subnetting_minParticipatingStations;  ///< subnetting flag

        boost::optional<bool> fillinmode_duringScanSelection;       ///< fillin mode flag
        boost::optional<bool> fillinmode_influenceOnScanSelection;  ///< fillin mode flag
        boost::optional<bool> fillinmode_aPosteriori;               ///< fillin mode flag
        boost::optional<double> focusCornerSwitchCadence;           ///< focus corner switch cadence interval

        boost::optional<double> weightSkyCoverage;           ///< weight factors for sky Coverage
        boost::optional<double> weightNumberOfObservations;  ///< weight factors for number of observations
        boost::optional<double> weightDuration;              ///< weight factors for duration
        boost::optional<double> weightAverageSources;        ///< weight factors for average source observations
        boost::optional<double> weightAverageStations;       ///< weight factors for average station usage
        boost::optional<double> weightAverageBaselines;      ///< weight factors for average baseline usage
        boost::optional<double> weightIdleTime;              ///< weight factor for idle time
        boost::optional<double> weightIdleTime_interval;     ///< idle time interval
        boost::optional<double> weightClosures;              ///< weight factor for closure delays
        boost::optional<double> weightMaxClosures;           ///< maxium significant closure delays
        boost::optional<double> weightLowDeclination;        ///< weight factor for low declination
        boost::optional<double> weightLowDeclination_begin;  ///< start declination for extra weight
        boost::optional<double> weightLowDeclination_full;   ///< start declination with full weight
        boost::optional<double> weightLowElevation;          ///< weight factor for low elevation
        boost::optional<double> weightLowElevation_begin;    ///< start elevation for extra weight
        boost::optional<double> weightLowElevation_full;     ///< start elevation for full weight

        boost::optional<double> skyCoverageInfluenceDistance;  ///< sky coverage influence distance
        boost::optional<double> skyCoverageInfluenceTime;      ///< sky coverage influence time

        std::map<std::string, double> stationWeight;                  ///< station weight
        std::map<std::string, unsigned int> stationMinSlewtime;       ///< station minimum slew time in seconds
        std::map<std::string, unsigned int> stationMaxSlewtime;       ///< station maximum slew time in seconds
        std::map<std::string, double> stationMinSlewDistance;         ///< station minimum slew distance
        std::map<std::string, double> stationMaxSlewDistance;         ///< station maximum slew distance
        std::map<std::string, unsigned int> stationMaxWait;           ///< station maximum wait time in seconds
        std::map<std::string, double> stationMinElevation;            ///< station minimum elevation
        std::map<std::string, unsigned int> stationMaxNumberOfScans;  ///< station maximum number of scans
        std::map<std::string, unsigned int> stationMaxScan;           ///< station maximum scan time in seconds
        std::map<std::string, unsigned int> stationMinScan;           ///< station minimum scan time in seconds

        std::map<std::string, double> sourceWeight;                     ///< source weight
        std::map<std::string, unsigned int> sourceMinNumberOfStations;  ///< source minimum number of stations
        std::map<std::string, double> sourceMinFlux;                    ///< source minimum flux required
        std::map<std::string, unsigned int> sourceMaxNumberOfScans;     ///< source maximum number of scans
        std::map<std::string, double> sourceMinElevation;               ///< source minimum elevation
        std::map<std::string, double> sourceMinSunDistance;             ///< source minimum sun distance
        std::map<std::string, unsigned int> sourceMaxScan;              ///< source maximum scan time in seconds
        std::map<std::string, unsigned int> sourceMinScan;              ///< source minimum scan time in seconds
        std::map<std::string, unsigned int> sourceMinRepeat;            ///< source minimum repeat time in seconds

        std::map<std::string, double> baselineWeight;         ///< baseline weight
        std::map<std::string, unsigned int> baselineMaxScan;  ///< baseline maximum scan time in seconds
        std::map<std::string, unsigned int> baselineMinScan;  ///< baseline minimum scan time in seconds

        /**
         * @brief default constructor
         * @author Matthias Schartner
         */
        Parameters() = default;
        ;

        /**
         * @brief constructor based on two parents
         * @author Matthias Schartner
         *
         * @param v vector of parents
         * @param mutation mutation acceleration
         * @param minMutation minimum mutation
         */
        Parameters( const std::vector<Parameters> &v, double mutation, double minMutation );

        /**
         * @brief normalize weight factors
         * @author Matthias Schartner
         */
        void normalizeWeightFactors();


        /**
         * @brief normalize weight factors
         * @author Matthias Schartner
         * @param nsta number of stations
         * @param nsrc number of sources
         */
        void normalizeWeights( unsigned long nsta, unsigned long nsrc );

        /**
         * @brief output function to stream object
         * @author Matthias Schartner
         *
         * @param of outfile stream object
         */
        void output( std::ofstream &of ) const {
            unsigned long nsta = stationWeight.size();

            if ( start.is_initialized() ) {
                of << "    start " << *start << "\n";
            }
            if ( subnetting.is_initialized() ) {
                of << "    subnetting " << *subnetting << "\n";
            }
            if ( subnetting_minSourceAngle.is_initialized() ) {
                of << "    subnetting minimum source distance " << *subnetting_minSourceAngle << " [deg]\n";
            }
            if ( subnetting_minParticipatingStations.is_initialized() ) {
                of << "    subnetting minimum participating stations " << *subnetting_minParticipatingStations
                   << " [%]\n";
            }

            if ( fillinmode_duringScanSelection.is_initialized() ) {
                of << "    fillinmode during scan selection " << *fillinmode_duringScanSelection << "\n";
            }
            if ( fillinmode_aPosteriori.is_initialized() ) {
                of << "    fillinmode a posteriori " << *fillinmode_aPosteriori << "\n";
            }
            if ( fillinmode_influenceOnScanSelection.is_initialized() ) {
                of << "    fillinmode influence scan selection " << *fillinmode_influenceOnScanSelection << "\n";
            }
            if ( focusCornerSwitchCadence.is_initialized() ) {
                of << "    focus corner switch cadence " << *focusCornerSwitchCadence << " [s]\n";
            }

            if ( weightSkyCoverage.is_initialized() ) {
                of << "    weight sky-coverage " << *weightSkyCoverage << "\n";
            }
            if ( weightNumberOfObservations.is_initialized() ) {
                of << "    weight number of observations " << *weightNumberOfObservations << "\n";
            }
            if ( weightDuration.is_initialized() ) {
                of << "    weight duration " << *weightDuration << "\n";
            }
            if ( weightAverageSources.is_initialized() ) {
                of << "    weight average sources " << *weightAverageSources << "\n";
            }
            if ( weightAverageStations.is_initialized() ) {
                of << "    weight average stations " << *weightAverageStations << "\n";
            }
            if ( weightAverageBaselines.is_initialized() ) {
                of << "    weight average baselines " << *weightAverageBaselines << "\n";
            }
            if ( weightIdleTime.is_initialized() ) {
                of << "    weight idle time " << *weightIdleTime << "\n";
            }
            if ( weightIdleTime_interval.is_initialized() ) {
                of << "    weight idle time interval " << *weightIdleTime_interval << " [s]\n";
            }
            if ( weightClosures.is_initialized() ) {
                of << "    weight closures " << *weightClosures << "\n";
            }
            if ( weightMaxClosures.is_initialized() ) {
                of << "    weight max closures " << *weightMaxClosures << " [s]\n";
            }
            if ( weightLowDeclination.is_initialized() ) {
                of << "    weight low declination " << *weightLowDeclination << "\n";
            }
            if ( weightLowDeclination_begin.is_initialized() ) {
                of << "    weight low declination start " << *weightLowDeclination_begin << " [deg]\n";
            }
            if ( weightLowDeclination_full.is_initialized() ) {
                of << "    weight low declination full " << *weightLowDeclination_full << " [deg]\n";
            }
            if ( weightLowElevation.is_initialized() ) {
                of << "    weight low elevation " << *weightLowElevation << "\n";
            }
            if ( weightLowElevation_begin.is_initialized() ) {
                of << "    weight low elevation start " << *weightLowElevation_begin << " [deg]\n";
            }
            if ( weightLowElevation_full.is_initialized() ) {
                of << "    weight low elevation full " << *weightLowElevation_full << " [deg]\n";
            }

            if ( skyCoverageInfluenceDistance.is_initialized() ) {
                of << "    sky-coverage influence distance " << *skyCoverageInfluenceDistance << " [deg]\n";
            }
            if ( skyCoverageInfluenceTime.is_initialized() ) {
                of << "    sky-coverage influence time " << *skyCoverageInfluenceTime << " [s]\n";
            }

            for ( const auto &any : stationWeight ) {
                of << "    station weight " << any.first << ": " << any.second << "\n";
            }
            for ( const auto &any : stationMinSlewtime ) {
                of << "    station min slewtime " << any.first << ": " << any.second << " [s]\n";
            }
            for ( const auto &any : stationMaxSlewtime ) {
                of << "    station max slewtime " << any.first << ": " << any.second << " [s]\n";
            }
            for ( const auto &any : stationMinSlewDistance ) {
                of << "    station max slew distance " << any.first << ": " << any.second << " [deg]\n";
            }
            for ( const auto &any : stationMaxSlewDistance ) {
                of << "    station min slew distance " << any.first << ": " << any.second << " [deg]\n";
            }
            for ( const auto &any : stationMaxWait ) {
                of << "    station max wait " << any.first << ": " << any.second << " [s]\n";
            }
            for ( const auto &any : stationMinElevation ) {
                of << "    station min elevation " << any.first << ": " << any.second << " [deg]\n";
            }
            for ( const auto &any : stationMaxNumberOfScans ) {
                of << "    station max number of scans " << any.first << ": " << any.second << "\n";
            }
            for ( const auto &any : stationMaxScan ) {
                of << "    station max scan time " << any.first << ": " << any.second << " [s]\n";
            }
            for ( const auto &any : stationMinScan ) {
                of << "    station min scan time " << any.first << ": " << any.second << " [s]\n";
            }

            for ( const auto &any : sourceWeight ) {
                of << "    source weight  " << any.first << ": " << any.second << "\n";
            }
            for ( const auto &any : sourceMinNumberOfStations ) {
                of << "    source min number of stations " << any.first << ": " << any.second << "\n";
            }
            for ( const auto &any : sourceMinFlux ) {
                of << "    source min flux " << any.first << ": " << any.second << " [Jy]\n";
            }
            for ( const auto &any : sourceMaxNumberOfScans ) {
                of << "    source max number of scans " << any.first << ": " << any.second << "\n";
            }
            for ( const auto &any : sourceMinElevation ) {
                of << "    source min elevation " << any.first << ": " << any.second << " [deg]\n";
            }
            for ( const auto &any : sourceMinSunDistance ) {
                of << "    source min sun distance " << any.first << ": " << any.second << " [deg]\n";
            }
            for ( const auto &any : sourceMaxScan ) {
                of << "    source max scan time " << any.first << ": " << any.second << " [s]\n";
            }
            for ( const auto &any : sourceMinScan ) {
                of << "    source min scan time " << any.first << ": " << any.second << " [s]\n";
            }
            for ( const auto &any : sourceMinRepeat ) {
                of << "    source min repeat " << any.first << ": " << any.second << " [s]\n";
            }

            for ( const auto &any : baselineWeight ) {
                of << "    baseline weight " << any.first << ": " << any.second << "\n";
            }
            for ( const auto &any : baselineMaxScan ) {
                of << "    baseline max scan time " << any.first << ": " << any.second << " [s]\n";
            }
            for ( const auto &any : baselineMinScan ) {
                of << "    baseline min scan time " << any.first << ": " << any.second << " [s]\n";
            }
        }


        /**
         * @brief output function to write header to statistics file
         * @author Matthias Schartner
         *
         * @param of outfile stream object
         */
        void statisticsHeaderOutput( std::ofstream &of ) const {
            if ( start.is_initialized() ) {
                of << "start,";
            }
            if ( subnetting.is_initialized() ) {
                of << "subnetting,";
            }
            if ( subnetting_minSourceAngle.is_initialized() ) {
                of << "subnetting_min_source_angle,";
            }
            if ( subnetting_minParticipatingStations.is_initialized() ) {
                of << "subnetting_min_participating_stations,";
            }

            if ( fillinmode_duringScanSelection.is_initialized() ) {
                of << "fillin-mode_during_scan_selection,";
            }
            if ( fillinmode_aPosteriori.is_initialized() ) {
                of << "fillin-mode_a_posteriori,";
            }
            if ( fillinmode_influenceOnScanSelection.is_initialized() ) {
                of << "fillin-mode_influence_on_scan_selection,";
            }
            if ( focusCornerSwitchCadence.is_initialized() ) {
                of << "focus_corner_switch_cadence,";
            }

            if ( weightSkyCoverage.is_initialized() ) {
                of << "weight_sky-coverage,";
            }
            if ( weightNumberOfObservations.is_initialized() ) {
                of << "weight_number_of_observations,";
            }
            if ( weightDuration.is_initialized() ) {
                of << "weight_duration,";
            }
            if ( weightAverageSources.is_initialized() ) {
                of << "weight_average_sources,";
            }
            if ( weightAverageStations.is_initialized() ) {
                of << "weight_average_stations,";
            }
            if ( weightAverageBaselines.is_initialized() ) {
                of << "weight_average_baselines,";
            }
            if ( weightIdleTime.is_initialized() ) {
                of << "weight_idle_time,";
            }
            if ( weightIdleTime_interval.is_initialized() ) {
                of << "weight_idle_time_interval,";
            }
            if ( weightClosures.is_initialized() ) {
                of << "weight_closures,";
            }
            if ( weightMaxClosures.is_initialized() ) {
                of << "weight_max_closures,";
            }
            if ( weightLowDeclination.is_initialized() ) {
                of << "weight_low_declination,";
            }
            if ( weightLowDeclination_begin.is_initialized() ) {
                of << "weight_low_declination_begin,";
            }
            if ( weightLowDeclination_full.is_initialized() ) {
                of << "weight_low_declination_full,";
            }
            if ( weightLowElevation.is_initialized() ) {
                of << "weight_low_elevation,";
            }
            if ( weightLowElevation_begin.is_initialized() ) {
                of << "weight_low_elevation_begin,";
            }
            if ( weightLowElevation_full.is_initialized() ) {
                of << "weight_low_elevation_full,";
            }

            if ( skyCoverageInfluenceDistance.is_initialized() ) {
                of << "sky-coverage_influence_distance,";
            }
            if ( skyCoverageInfluenceTime.is_initialized() ) {
                of << "sky-coverage_influence_time,";
            }

            for ( const auto &any : stationWeight ) {
                of << "station_weight_" << any.first << ",";
            }
            for ( const auto &any : stationMinSlewtime ) {
                of << "station_min_slewtime_" << any.first << ",";
            }
            for ( const auto &any : stationMaxSlewtime ) {
                of << "station_max_slewtime_" << any.first << ",";
            }
            for ( const auto &any : stationMinSlewDistance ) {
                of << "station_min_slew_distance_" << any.first << ",";
            }
            for ( const auto &any : stationMaxSlewDistance ) {
                of << "station_max_slew_distance_" << any.first << ",";
            }
            for ( const auto &any : stationMaxWait ) {
                of << "station_max_wait_" << any.first << ",";
            }
            for ( const auto &any : stationMinElevation ) {
                of << "station_min_elevation_" << any.first << ",";
            }
            for ( const auto &any : stationMaxNumberOfScans ) {
                of << "station_max_number_of_scans_" << any.first << ",";
            }
            for ( const auto &any : stationMaxScan ) {
                of << "station_max_scan_" << any.first << ",";
            }
            for ( const auto &any : stationMinScan ) {
                of << "station_min_scan_" << any.first << ",";
            }

            for ( const auto &any : sourceWeight ) {
                of << "source_weight_" << any.first << ",";
            }
            for ( const auto &any : sourceMinNumberOfStations ) {
                of << "source_min_number_of_stations_" << any.first << ",";
            }
            for ( const auto &any : sourceMinFlux ) {
                of << "source_min_flux_" << any.first << ",";
            }
            for ( const auto &any : sourceMaxNumberOfScans ) {
                of << "source_max_number_of_scans_" << any.first << ",";
            }
            for ( const auto &any : sourceMinElevation ) {
                of << "source_min_elevation_" << any.first << ",";
            }
            for ( const auto &any : sourceMinSunDistance ) {
                of << "source_min_sun_distance_" << any.first << ",";
            }
            for ( const auto &any : sourceMaxScan ) {
                of << "source_max_scan_" << any.first << ",";
            }
            for ( const auto &any : sourceMinScan ) {
                of << "source_min_scan_" << any.first << ",";
            }
            for ( const auto &any : sourceMinRepeat ) {
                of << "source_min_repeat_" << any.first << ",";
            }

            for ( const auto &any : baselineWeight ) {
                of << "baseline_weight_" << any.first << ",";
            }
            for ( const auto &any : baselineMaxScan ) {
                of << "baseline_max_scan_" << any.first << ",";
            }
            for ( const auto &any : baselineMinScan ) {
                of << "baseline_min_scan_" << any.first << ",";
            }
        }


        /**
         * @brief output function to data to statistics file
         * @author Matthias Schartner
         *
         * @return string with statistics information
         */
        std::string statisticsOutput() const {
            std::string str;
            if ( start.is_initialized() ) {
                //        str.append( std::to_string(*start)) ).append(",");
            }
            if ( subnetting.is_initialized() ) {
                str.append( std::to_string( *subnetting ) ).append( "," );
            }
            if ( subnetting_minSourceAngle.is_initialized() ) {
                str.append( std::to_string( *subnetting_minSourceAngle ) ).append( "," );
            }
            if ( subnetting_minParticipatingStations.is_initialized() ) {
                str.append( std::to_string( *subnetting_minParticipatingStations ) ).append( "," );
            }

            if ( fillinmode_duringScanSelection.is_initialized() ) {
                str.append( std::to_string( *fillinmode_duringScanSelection ) ).append( "," );
            }
            if ( fillinmode_aPosteriori.is_initialized() ) {
                str.append( std::to_string( *fillinmode_aPosteriori ) ).append( "," );
            }
            if ( fillinmode_influenceOnScanSelection.is_initialized() ) {
                str.append( std::to_string( *fillinmode_influenceOnScanSelection ) ).append( "," );
            }
            if ( focusCornerSwitchCadence.is_initialized() ) {
                str.append( std::to_string( *focusCornerSwitchCadence ) ).append( "," );
            }

            if ( weightSkyCoverage.is_initialized() ) {
                str.append( std::to_string( *weightSkyCoverage ) ).append( "," );
            }
            if ( weightNumberOfObservations.is_initialized() ) {
                str.append( std::to_string( *weightNumberOfObservations ) ).append( "," );
            }
            if ( weightDuration.is_initialized() ) {
                str.append( std::to_string( *weightDuration ) ).append( "," );
            }
            if ( weightAverageSources.is_initialized() ) {
                str.append( std::to_string( *weightAverageSources ) ).append( "," );
            }
            if ( weightAverageStations.is_initialized() ) {
                str.append( std::to_string( *weightAverageStations ) ).append( "," );
            }
            if ( weightAverageBaselines.is_initialized() ) {
                str.append( std::to_string( *weightAverageBaselines ) ).append( "," );
            }
            if ( weightIdleTime.is_initialized() ) {
                str.append( std::to_string( *weightIdleTime ) ).append( "," );
            }
            if ( weightIdleTime_interval.is_initialized() ) {
                str.append( std::to_string( *weightIdleTime_interval ) ).append( "," );
            }
            if ( weightClosures.is_initialized() ) {
                str.append( std::to_string( *weightClosures ) ).append( "," );
            }
            if ( weightMaxClosures.is_initialized() ) {
                str.append( std::to_string( *weightMaxClosures ) ).append( "," );
            }
            if ( weightLowDeclination.is_initialized() ) {
                str.append( std::to_string( *weightLowDeclination ) ).append( "," );
            }
            if ( weightLowDeclination_begin.is_initialized() ) {
                str.append( std::to_string( *weightLowDeclination_begin ) ).append( "," );
            }
            if ( weightLowDeclination_full.is_initialized() ) {
                str.append( std::to_string( *weightLowDeclination_full ) ).append( "," );
            }
            if ( weightLowElevation.is_initialized() ) {
                str.append( std::to_string( *weightLowElevation ) ).append( "," );
            }
            if ( weightLowElevation_begin.is_initialized() ) {
                str.append( std::to_string( *weightLowElevation_begin ) ).append( "," );
            }
            if ( weightLowElevation_full.is_initialized() ) {
                str.append( std::to_string( *weightLowElevation_full ) ).append( "," );
            }

            if ( skyCoverageInfluenceDistance.is_initialized() ) {
                str.append( std::to_string( *skyCoverageInfluenceDistance ) ).append( "," );
            }
            if ( skyCoverageInfluenceTime.is_initialized() ) {
                str.append( std::to_string( *skyCoverageInfluenceTime ) ).append( "," );
            }

            for ( const auto &any : stationWeight ) {
                str.append( std::to_string( any.second ) ).append( "," );
            }
            for ( const auto &any : stationMinSlewtime ) {
                str.append( std::to_string( any.second ) ).append( "," );
            }
            for ( const auto &any : stationMaxSlewtime ) {
                str.append( std::to_string( any.second ) ).append( "," );
            }
            for ( const auto &any : stationMinSlewDistance ) {
                str.append( std::to_string( any.second ) ).append( "," );
            }
            for ( const auto &any : stationMaxSlewDistance ) {
                str.append( std::to_string( any.second ) ).append( "," );
            }
            for ( const auto &any : stationMaxWait ) {
                str.append( std::to_string( any.second ) ).append( "," );
            }
            for ( const auto &any : stationMinElevation ) {
                str.append( std::to_string( any.second ) ).append( "," );
            }
            for ( const auto &any : stationMaxNumberOfScans ) {
                str.append( std::to_string( any.second ) ).append( "," );
            }
            for ( const auto &any : stationMaxScan ) {
                str.append( std::to_string( any.second ) ).append( "," );
            }
            for ( const auto &any : stationMinScan ) {
                str.append( std::to_string( any.second ) ).append( "," );
            }

            for ( const auto &any : sourceWeight ) {
                str.append( std::to_string( any.second ) ).append( "," );
            }
            for ( const auto &any : sourceMinNumberOfStations ) {
                str.append( std::to_string( any.second ) ).append( "," );
            }
            for ( const auto &any : sourceMinFlux ) {
                str.append( std::to_string( any.second ) ).append( "," );
            }
            for ( const auto &any : sourceMaxNumberOfScans ) {
                str.append( std::to_string( any.second ) ).append( "," );
            }
            for ( const auto &any : sourceMinElevation ) {
                str.append( std::to_string( any.second ) ).append( "," );
            }
            for ( const auto &any : sourceMinSunDistance ) {
                str.append( std::to_string( any.second ) ).append( "," );
            }
            for ( const auto &any : sourceMaxScan ) {
                str.append( std::to_string( any.second ) ).append( "," );
            }
            for ( const auto &any : sourceMinScan ) {
                str.append( std::to_string( any.second ) ).append( "," );
            }
            for ( const auto &any : sourceMinRepeat ) {
                str.append( std::to_string( any.second ) ).append( "," );
            }

            for ( const auto &any : baselineWeight ) {
                str.append( std::to_string( any.second ) ).append( "," );
            }
            for ( const auto &any : baselineMaxScan ) {
                str.append( std::to_string( any.second ) ).append( "," );
            }
            for ( const auto &any : baselineMinScan ) {
                str.append( std::to_string( any.second ) ).append( "," );
            }
            return str;
        }
    };


    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param sta_group station groups
     * @param src_group source groups
     * @param bls_group baseline groups
     */
    MultiScheduling( std::unordered_map<std::string, std::vector<std::string>> sta_group,
                     std::unordered_map<std::string, std::vector<std::string>> src_group,
                     std::unordered_map<std::string, std::vector<std::string>> bls_group );

    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param nsta number of stations
     * @param nsrc number of sources
     */
    static void setConstants( unsigned long nsta, unsigned long nsrc ) {
        MultiScheduling::nsta_ = nsta;
        MultiScheduling::nsrc_ = nsrc;
    }

    /**
     * @brief set session start times
     * @author Matthias Schartner
     *
     * @param start session start times
     */
    void setStart( const std::vector<boost::posix_time::ptime> &start ) { MultiScheduling::start_ = start; }


    /**
     * @brief add new boolean parameter
     * @author Matthias Schartner
     *
     * @param name parameter name
     */
    void addParameters( const std::string &name );


    /**
     * @brief add new value based parameter
     * @author Matthias Schartner
     *
     * @param name parameter name
     * @param values parameter values
     */
    void addParameters( const std::string &name, const std::vector<double> &values );


    /**
     * @brief add new value and member based parameter
     * @author Matthias Schartner
     *
     * @param name parameter name
     * @param member parameter member
     * @param values parameter values
     */
    void addParameters( const std::string &name, const std::string &member, const std::vector<double> &values );


    /**
     * @brief create vector of all possible multi scheduling parameter combinations
     * @author Matthias Schartner
     *
     * @param maxNr maximum number of parameters
     * @return all possible multi scheduling parameter combinations
     */
    std::vector<Parameters> createMultiScheduleParameters( unsigned int maxNr );


    /**
     * @brief generate new population of multi-scheduling parameters
     * @author Matthias Schartner
     *
     * @param gen generation number
     * @param old_pop old populatoin of parameters
     * @param scores score of each parameter
     * @param tree xml property tree
     * @return new population of parameters
     */
    static std::vector<Parameters> evolution_step( int gen, const std::vector<Parameters> &old_pop,
                                                   const std::map<int, double> &scores,
                                                   const boost::property_tree::ptree &tree );


    /**
     * @brief create property tree used for parameter.xml file
     * @author Matthias Schartner
     *
     * @return property tree for multiSched block
     */
    boost::property_tree::ptree createPropertyTree() const;

    /**
     * @brief set seed for RNG generator
     * @author Matthias Schartner
     *
     * @param seed seed
     */
    static void setSeed( unsigned int seed ) { random_engine_ = std::default_random_engine( seed ); }

    /**
     * @brief set pick random values
     * @author Matthias Schartner
     *
     * @param flag flag
     */
    static void pick_random_values( bool flag ) { pick_random = flag; }


   private:
    static unsigned long nextId;                       ///< next id
    static std::default_random_engine random_engine_;  ///< random number generator engine
    static bool pick_random;
    static unsigned long nsta_;
    static unsigned long nsrc_;

    std::unordered_map<std::string, std::vector<std::string>> stationGroups_;   ///< used station groups
    std::unordered_map<std::string, std::vector<std::string>> sourceGroups_;    ///< used source groups
    std::unordered_map<std::string, std::vector<std::string>> baselineGroups_;  ///< used baseline groups

    std::vector<boost::posix_time::ptime> start_;  ///< start times

    std::vector<std::string> singleArgumentLogical;                                  ///< boolean parameters
    std::vector<std::pair<std::string, std::vector<double>>> singleArgumentNumeric;  ///< value based parameters
    std::vector<std::pair<std::string, std::pair<std::string, std::vector<double>>>>
        doubleArgumentNumeric;  ///< value and member based parameters

    /**
     * @brief add boolean parameters
     * @author Matthias Schartner
     *
     * @param allPara list of all parameters
     * @param n_before number of blocks
     * @param name parameter name
     * @param pick_random enable random values of multi-scheduling parameters
     */
    void addParameter( std::vector<Parameters> &allPara, unsigned long &n_before, const std::string &name,
                       bool pick_random = false );


    /**
     * @brief add value based parameters
     * @author Matthias Schartner
     *
     * @param allPara list of all parameters
     * @param n_before number of blocks
     * @param name parameter name
     * @param value number of values
     * @param pick_random enable random values of multi-scheduling parameters
     */
    void addParameter( std::vector<Parameters> &allPara, unsigned long &n_before, const std::string &name,
                       const std::vector<double> &value, bool pick_random = false );


    /**
     * @brief add value and member based parameters
     * @author Matthias Schartner
     *
     * @param allPara list of all parameters
     * @param n_before number of blocks
     * @param name parameter name
     * @param member member name
     * @param value number of values
     * @param pick_random enable random values of multi-scheduling parameters
     */
    void addParameter( std::vector<Parameters> &allPara, unsigned long &n_before, const std::string &name,
                       const std::string &member, const std::vector<double> &value, bool pick_random = false );

    /**
     * @brief create vector of all possible multi scheduling parameter combinations (gridwise selection)
     * @author Matthias Schartner
     *
     * @param maxNr maximum number of parameters
     * @return all possible multi scheduling parameter combinations
     */
    std::vector<Parameters> createMultiScheduleParameters_gridwise( unsigned int maxNr );


    /**
     * @brief create vector of all possible multi scheduling parameter combinations (random selection)
     * @author Matthias Schartner
     *
     * @param maxNr maximum number of parameters
     * @return all possible multi scheduling parameter combinations
     */
    std::vector<Parameters> createMultiScheduleParameters_random( unsigned int maxNr );


    /**
     * @brief collecs all entries of a fiven field of a vector of parameters
     * @author Matthias Schartner
     *
     * @tparam T type
     * @param f field
     * @param v vector of parameters
     * @return vector of field entries (type T)
     */
    template <typename T>
    static std::vector<T> collect( T Parameters::*f, std::vector<Parameters> const &v ) {
        std::vector<T> output;
        for ( auto const &elem : v ) {
            output.push_back( elem.*f );
        }
        return output;
    }
};
}  // namespace VieVS

#endif  // MULTISCHEDULING_H
