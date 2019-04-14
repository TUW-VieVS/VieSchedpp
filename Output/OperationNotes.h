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
 * @file OperationNotes.h
 * @brief class OperationNotes
 *
 * @author Matthias Schartner
 * @date 06.02.2019
 */

#ifndef VIESCHEDPP_OPERATIONNOTES_H
#define VIESCHEDPP_OPERATIONNOTES_H

#include <boost/property_tree/xml_parser.hpp>

#include "../Scan/Scan.h"
#include "../ObservingMode/ObservingMode.h"
#include "../Misc/MultiScheduling.h"

namespace VieVS{

    /**
     * @class OperationNotes
     * @brief create Operation notes output or skdsum output
     *
     * @author Matthias Schartner
     * @date 22.08.2017
     */
    class OperationNotes: public VieVS_Object {
    public:
        /**
         * @brief constructor
         * @author Matthias Schartner
         *
         * @param file file name
         */
        explicit OperationNotes(const std::string &file);

        /**
         * @brief write operation notes file
         * @author Matthias Schartner
         *
         * @param network station network
         * @param sources list of all sources
         * @param scans list of all scans
         * @param obsModes observing mode
         * @param xml paramters.xml file
         * @param version version number
         * @param multiSchedulingParameters multi scheduling parameters
         */
        void writeOperationNotes(const Network &network, const std::vector<Source>& sources, const std::vector<Scan> & scans,
                      const std::shared_ptr<const ObservingMode> &obsModes, const boost::property_tree::ptree &xml,
                      int version, boost::optional<MultiScheduling::Parameters> multiSchedulingParameters = boost::none);

        /**
         * @brief write simple skdsum file
         * @author Matthias Schartner
         *
         * @param network station network
         * @param sources list of all sources
         * @param scans list of all scans
         */
        void writeSkdsum(const Network &network, const std::vector<Source>& sources, const std::vector<Scan> & scans);


    private:
        static unsigned long nextId; ///< next id for this object type

        std::ofstream of; ///< output stream object


        /**
         * @brief general statistics of the schedule
         * @author Matthias Schartner
         *
         * @param scans list of all scans
         */
        void displayGeneralStatistics(const std::vector<Scan> & scans);

        /**
         * @brief baseline dependent statistics of the schedule
         * @author Matthias Schartner
         *
         * @param network station network
         */
        void displayBaselineStatistics(const Network &network);

        /**
         * @brief displays some station dependent statistics of the schedule
         * @author Matthias Schartner
         *
         * @param network station network
         */
        void displayStationStatistics(const Network &network);

        /**
         * @brief displays some source dependent statistics of the schedule
         * @author Matthias Schartner
         *
         * @param sources list of all sources
         */
        void displaySourceStatistics(const std::vector<Source>& sources);

        /**
         * @brief number of stations per scan statistics
         * @author Matthias Schartner
         *
         * @param network station network
         * @param scans list of all scans
         */
        void displayNstaStatistics(const Network &network, const std::vector<Scan> & scans);

        /**
         * @brief source dependent statistics of the schedule
         * @author Matthias Schartner
         *
         * @param network station network
         * @param scans list of all scans
         */
        void displayScanDurationStatistics(const Network &network, const std::vector<Scan> & scans);

        /**
         * @brief time spend per station
         * @author Matthias Schartner
         *
         * @param network station network
         * @param obsModes observing mode
         */
        void displayTimeStatistics(const Network &network, const std::shared_ptr<const ObservingMode> &obsModes);

        /**
         * @brief list astronomical parameters
         * @author Matthias Schartner
         *
         */
        void displayAstronomicalParameters();


        /**
         * @brief display summary of theoretical SNR values
         * @author Matthias Schartner
         *
         * @param network station network
         * @param sources list of all sources
         * @param scans list of all scans
         * @param obsModes observing mode
         */
        void displaySNRSummary(const Network &network,
                               const std::vector<Source>& sources,
                               const std::vector<Scan> & scans,
                               const std::shared_ptr<const ObservingMode> &obsModes);

        /**
         * @brief list first and last observations in skd style
         * @author Matthias Schartner
         *
         * lists as many scans until all stations are listed (maximum of 5)
         *
         * @param expName experiment name
         * @param network station network
         * @param sources list of all sources
         * @param scans list of all scans
         */
        void firstLastObservations_skdStyle(const std::string &expName,
                                            const Network &network,
                                            const std::vector<Source>& sources,
                                            const std::vector<Scan> & scans);

        /**
         * @brief list key=stationName
         * @author Matthias Schartner
         *
         * @param network station network
         */
        void listKeys(const Network &network);

        /**
         * @brief list sky coverage scores
         * @author Matthias Schartner
         *
         * @param network station network
         */
        void displaySkyCoverageScore(const Network &network);

    };
}


#endif //VIESCHEDPP_OPERATIONNOTES_H
