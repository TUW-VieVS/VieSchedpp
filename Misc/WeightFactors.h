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
 * @file WeightFactors.h
 * @brief class WeightFactors
 *
 *
 * @author Matthias Schartner
 * @date 14.08.2017
 */

#ifndef WEIGHTFACTORS_H
#define WEIGHTFACTORS_H


#include <fstream>
#include <sstream>
#include "Constants.h"


namespace VieVS {
/**
 * @class WeightFactors
 * @brief storage of all VLBI weight factors
 *
 * @author Matthias Schartner
 * @date 14.08.2017
 */
class WeightFactors {
   public:
    static thread_local double weightSkyCoverage;           ///< weight factor for sky Coverage
    static thread_local double weightNumberOfObservations;  ///< weight factor for number of observations
    static thread_local double weightDuration;              ///< weight factor for duration

    static thread_local double weightAverageSources;    ///< weight factor for average out sources
    static thread_local double weightAverageStations;   ///< weight factor for average out stations
    static thread_local double weightAverageBaselines;  ///< weight factor for average out baselines

    static thread_local double weightIdleTime;          ///< weight factor for extra weight after long idle time
    static thread_local unsigned int idleTimeInterval;  ///< long itel time interval

    static thread_local double weightDeclination;       ///< weight factor for declination
    static thread_local double declinationStartWeight;  ///< start declination of additional weight
    ///< (everything above has factor 0)
    static thread_local double declinationFullWeight;  ///< end declination of additional declination weight slope
    ///< (everything below has factor 1)

    static thread_local double weightLowElevation;       ///< weight factor for low elevation scans
    static thread_local double lowElevationStartWeight;  ///< start elevation of additional weight
    ///< (everything above has factor 0)
    static thread_local double lowElevationFullWeight;  ///< end elevation of additional declination weight slope
    ///< (everything below has factor 1)


    /**
     * @brief summary of all weight factors
     * @author Matthias Schartner
     *
     * @param of out stream object
     */
    static void summary( std::ofstream &of ) {
        of << "weight factors: \n";
        if ( weightSkyCoverage != 0 ) {
            of << " sky coverage:           " << weightSkyCoverage << "\n";
        }
        if ( weightNumberOfObservations != 0 ) {
            of << " number of observations: " << weightNumberOfObservations << "\n";
        }
        if ( weightDuration != 0 ) {
            of << " duration:               " << weightDuration << "\n";
        }
        if ( weightAverageSources != 0 ) {
            of << " average sources:        " << weightAverageSources << "\n";
        }
        if ( weightAverageStations != 0 ) {
            of << " average stations:       " << weightAverageStations << "\n";
        }
        if ( weightAverageBaselines != 0 ) {
            of << " average baselines:      " << weightAverageBaselines << "\n";
        }
        if ( weightIdleTime != 0 ) {
            of << " weight idle time:       " << weightIdleTime << "\n";
        }
        if ( idleTimeInterval != 0 ) {
            of << "     idle time interval: " << idleTimeInterval << "\n";
        }
        if ( weightDeclination != 0 ) {
            of << " declination             " << weightDeclination << "\n";
            of << "     from 90 to " << declinationStartWeight * rad2deg << " = 0\n";
            of << "     from " << declinationStartWeight * rad2deg << " to " << declinationFullWeight * rad2deg
               << " = linear interpolation\n";
            of << "     from " << declinationFullWeight * rad2deg << " to -90"
               << " = 1\n";
        }
        if ( weightLowElevation != 0 ) {
            of << " elevation               " << weightLowElevation << "\n";
            of << "     from 90 to " << lowElevationStartWeight * rad2deg << " = 0\n";
            of << "     from " << lowElevationStartWeight * rad2deg << " to " << lowElevationFullWeight * rad2deg
               << " = linear interpolation\n";
            of << "     from " << lowElevationFullWeight * rad2deg << " to 0"
               << " = 1\n";
        }
        of << "\n";
    }


    /**
     * @brief header entries for statistics.csv file
     * @author Matthias Schartner
     */
    static std::string statisticsHeader( ) {
        return "weight_factor_sky_coverage,"
               "weight_factor_number_of_observations,"
               "weight_factor_duration,"
               "weight_factor_average_sources,"
               "weight_factor_average_stations,"
               "weight_factor_average_baselines,"
               "weight_factor_idle_time,"
               "weight_factor_idle_time_interval,"
               "weight_factor_low_declination,"
               "weight_factor_low_declination_start_weight,"
               "weight_factor_low_declination_full_weight,"
               "weight_factor_low_elevation,"
               "weight_factor_low_elevation_start_weight,"
               "weight_factor_low_elevation_full_weight,";
    }


    /**
     * @brief value entries for statistics.csv file
     * @author Matthias Schartner
     */
    static std::string statisticsValues( ) {
        double sum = weightSkyCoverage + weightNumberOfObservations + weightDuration + weightAverageSources +
                     weightAverageStations + weightAverageBaselines + weightIdleTime + weightDeclination +
                     weightLowElevation;

        std::ostringstream str;

        str << weightSkyCoverage / sum << "," << weightNumberOfObservations / sum << "," << weightDuration / sum << ","
           << weightAverageSources / sum << "," << weightAverageStations / sum << "," << weightAverageBaselines / sum
           << "," << weightIdleTime / sum << ",";

        if(weightIdleTime == 0){
            str << 0 << "," ;
        }else{
            str << idleTimeInterval << "," ;
        }


        str << weightDeclination / sum << ",";
        if(weightDeclination == 0){
            str << 0 << "," << 0 << "," ;
        }else{
            str << declinationStartWeight << "," << declinationFullWeight << "," ;
        }


        str << weightLowElevation / sum << ",";
        if(weightIdleTime == 0){
            str << 0 << "," << 0 << "," ;
        }else{
            str << lowElevationStartWeight << "," << lowElevationFullWeight << "," ;
        }

        return  str.str();
    }
};
}  // namespace VieVS
#endif  // WEIGHTFACTORS_H
