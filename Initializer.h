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
 * @file Initializer.h
 * @brief class Initializer
 *
 * @author Matthias Schartner
 * @date 28.06.2017
 */

#ifndef INITIALIZER_H
#define INITIALIZER_H
#include <vector>
#include <boost/date_time.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <algorithm>
#include <numeric>
#include <thread>
#include <memory>

#ifdef VIESCHEDPP_LOG
#include <boost/log/trivial.hpp>
#endif

#include "Antenna_AzEl.h"
#include "Antenna_HaDc.h"
#include "Antenna_XYew.h"

#include "CableWrap_AzEl.h"
#include "CableWrap_HaDc.h"
#include "CableWrap_XYew.h"

#include "Equipment_elDependent.h"

#include "HorizonMask_line.h"
#include "HorizonMask_step.h"

#include "Constants.h"
#include "Network.h"
#include "Source.h"
#include "AstronomicalParameters.h"
#include "LookupTable.h"
#include "WeightFactors.h"
//#include "ObservationMode.h"
#include "TimeSystem.h"
#include "Baseline.h"
#include "SkdCatalogReader.h"
#include "CalibratorBlock.h"
#include "Scan.h"
#include "ParameterSettings.h"
#include "Flux_B.h"
#include "Flux_M.h"
#include "HighImpactScanDescriptor.h"
#include "Mode.h"

#include "sofa.h"
#include "MultiScheduling.h"

namespace VieVS {
    /**
     * @class Initializer
     * @brief this is the VLBI initializer that creats every objects and passes them to the VLBI_scheduler
     *
     * @author Matthias Schartner
     * @date 28.06.2017
     */
    class Initializer: public VieVS_Object {
        friend class Scheduler;
        friend class SkdParser;
    public:
        /**
         * @brief possible group types
         * @author Matthias Schartner
         */
        enum class GroupType {
            station, ///< stations wise group
            source, ///< source wise group
            baseline, ///< baseline wise group
        };

        /**
         * @brief Parameters used in VLBI_initializer.
         * @author Matthias Schartner
         *
         * Most of this parameters are than passed to other classes like VLBI_scheduler.
         */
        struct Parameters {
            bool subnetting = true; ///< if set to true subnetting is enabled
            double subnettingMinAngle = 150*deg2rad; ///< backup value for minimum angle of subnetting sources
            double subnettingMinNStaPercent = 0.80; ///< backup value for minimum station percentage
            double subnettingMinNStaAllBut = 1; ///< backup value for minimum station all but value
            bool subnettingMinNStaPercent_otherwiseAllBut = false; ///< if set to true percentage value is used for subnetting minimum number of station calculation otherwise all but value

            bool fillinmodeDuringScanSelection = true; ///< schedule fillin mode scans
            bool fillinmodeInfluenceOnSchedule = true; ///< fillin modes scans influence schedule
            bool fillinmodeAPosteriori = false; ///< schedule fillin mode a posteriori
            bool idleToObservingTime = true; ///< transform idle time to additional observing time

            std::vector<std::string> selectedStations; ///< list of all selected station for this session from .xml file

            unsigned int maxNumberOfIterations = 999; ///< backup value for max number of iterations
            unsigned int numberOfGentleSourceReductions = 0; ///< backup value for gentle source reduction interations
            unsigned int minNumberOfSourcesToReduce = 0; ///< backup value for minimum number of sources to reduce

            bool andAsConditionCombination = true; ///< backup for condition combination. TRUE = and, FALSE = or

        };

        /**
         * @brief pre calculated values
         * @author Matthias Schartner
         */
        struct PRECALC {
            std::vector<std::vector<unsigned long>> subnettingSrcIds; ///< list of all available second sources in subnetting
        };

        Initializer();

        /**
         * @brief constructor
         * @author Matthias Schartner
         *
         * @param path path to VieSchedpp.xml file
         */
        explicit Initializer(const std::string &path);

        /**
         * @brief constructor
         * @author Matthias Schartner
         *
         * @param xml VieSchedpp.xml file
         */
        explicit Initializer(const boost::property_tree::ptree &xml);
        
        /**
         * @brief getter for VieSchedpp.xml content
         * @author Matthias Schartner
         *
         * @return VieSchedpp.xml content
         */
        const boost::property_tree::ptree &getXml() const {
            return xml_;
        }

        /**
         *  @brief pre calculates all possible second scans used for subnetting
         * @author Matthias Schartner
         */
        void precalcSubnettingSrcIds() noexcept;

        /**
         * @brief creates all selected stations from sked catalogs
         * @author Matthias Schartner
         *
         * @param reader sked catalog reader
         * @param of outstream to log file
         */
        void createStations(const SkdCatalogReader &reader, std::ofstream &of) noexcept;

        /**
         * @brief initializes all stations with settings from VieSchedpp.xml file
         * @author Matthias Schartner
         */
        void initializeStations() noexcept;

        /**
         * @brief precalc azimuth elevations for stations
         * @author Matthias Schartner
         */
        void precalcAzElStations() noexcept;

        /**
         * @brief initializes all baselines with settings from VieSchedpp.xml file
         * @author Matthias Schartner
         */
        void initializeBaselines() noexcept;

        /**
         * @brief creates all possible sources from sked catalogs
         * @author Matthias Schartner
         *
         * @param of outstream to log file
         */
        void createSources(const SkdCatalogReader &reader, std::ofstream &of) noexcept;

        /**
         * @brief initializes all sources with settings from VieSchedpp.xml file
         * @author Matthias Schartner
         *
         */
        void initializeSources() noexcept;

        /**
         * @brief initializes general block with settings from VieSchedpp.xml file
         * @author Matthias Schartner
         *
         * @param of outstream to log file
         */
        void initializeGeneral(std::ofstream &of) noexcept;


        /**
         * @brief initializes astronomical parameters
         * @author Matthias Schartner
         */
        void initializeAstronomicalParameteres() noexcept ;

        /**
         * @brief initializes the weight factors
         * @author Matthias Schartner
         */
        void initializeWeightFactors() noexcept;


        /**
         * @brief inintializes the sky Coverage lookup table
         * @author Matthias Schartner
         */
        void initializeSkyCoverages() noexcept;

        /**
         * @brief reads the observing mode information from VieSchedpp.xml file
         * @author Matthias Schartner
         *
         * @param skdCatalogs sked catalogs
         * @param of outstream to log file
         */
        void initializeObservingMode(const SkdCatalogReader &skdCatalogs, std::ofstream &of) noexcept;

        /**
         * @brief sets station names to observing mode and displays summary
         * @author Matthias Schartner
         *
         * @param of outstream to log file
         */
        void connectObservingMode( std::ofstream &of) noexcept;

        /**
         * @brief initializes a custom source sequence if there is one defined in the VieSchedpp.xml file
         * @author Matthias Schartner
         */
        void initializeSourceSequence() noexcept;

        /**
         * @brief reads all groups specified in the root tree
         * @author Matthias Schartner
         *
         * @param root tree start point
         * @param type group type
         * @return key is group name, value is list of group members
         */
        std::unordered_map<std::string, std::vector<std::string> > readGroups(boost::property_tree::ptree root, GroupType type) noexcept;

        /**
         * @brief applies all multi scheduling parameters to the initializer
         * @author Matthias Schartner
         *
         * @param parameters multi scheduling parameters
         * @param bodyLog outstream to log file
         */
        void applyMultiSchedParameters(const VieVS::MultiScheduling::Parameters &parameters);

        /**
         * @brief reads multiSched block from VieSchedpp.xml file
         * @author Matthias Schartner
         *
         * @return vector of all possible multisched parameter combination
         */
        std::vector<MultiScheduling::Parameters> readMultiSched(std::ostream &out);

        /**
         * @brief initializes calibration block with settings from VieSchedpp.xml file
         * @author Matthias Schartner
         *
         * @param of outstream to log file
         */
        void initializeCalibrationBlocks(std::ofstream &of);

        /**
         * @brief writes statistics log header
         * @author Matthias Schartner
         *
         * @param of outstream to statistics.csv file
         */
        void statisticsLogHeader(std::ofstream &of);

        /**
         * @brief initializes optimization conditions with settings from VieSchedpp.xml file
         * @author Matthias Schartner
         *
         * @param of outstream to log file
         */
        void initializeOptimization(std::ofstream &of);

        /**
         * @brief initializes high impact scan descriptors with settings from VieSchedpp.xml file
         * @author Matthias Schartner
         *
         * @param of outstream to log file
         */
        void initializeHighImpactScanDescriptor(std::ofstream &of);

    private:
        static unsigned long nextId; ///< next id for this object type

        boost::property_tree::ptree xml_; ///< content of VieSchedpp.xml file
        std::vector<Source> sources_; ///< list of all sources
        Network network_; ///< station network
        std::shared_ptr<Mode> mode_ = nullptr; ///< observing mode

        Parameters parameters_; ///< parameters
        PRECALC preCalculated_; ///< pre calculated values

        std::unordered_map<std::string, std::vector<std::string>> staGroups_; ///< station groups
        std::unordered_map<std::string, std::vector<std::string>> srcGroups_; ///< source groups
        std::unordered_map<std::string, std::vector<std::string>> blGroups_; ///< baseline groups

        boost::optional<HighImpactScanDescriptor> himp_; ///< high impact scan descriptor
        boost::optional<MultiScheduling::Parameters> multiSchedulingParameters_; ///< multi scheduling paramters


        /**
         * @brief station setup function
         * @author Matthias Schartner
         *
         * As a start all parameter form parentPARA are used.
         * If different parameter values are defined in the event these parameters are used instead of the parentPARA
         * parameters.
         *
         * @param events list of all events for stations
         * @param tree property tree that holds station setup information
         * @param parameters all defined parameters
         * @param groups all defined groups
         * @param parentPARA previously used parameters which are are use as template
         */
        void stationSetup(std::vector<std::vector<Station::Event> > &events,
                          const boost::property_tree::ptree &tree,
                          const std::unordered_map<std::string, ParameterSettings::ParametersStations> &parameters,
                          const std::unordered_map<std::string, std::vector<std::string>> &groups,
                          const Station::Parameters &parentPARA) noexcept;

        /**
         * @brief source setup function
         * @author Matthias Schartner
         *
         * As a start all parameter form parentPARA are used.
         * If different parameter values are defined in the event these parameters are used instead of the parentPARA
         * parameters.
         *
         * @param events list of all events for sources
         * @param tree property tree that holds source setup information
         * @param parameters all defined parameters
         * @param groups all defined groups
         * @param parentPARA previously used parameters which are are use as template
         */
        void sourceSetup(std::vector<std::vector<Source::Event> > &events,
                         const boost::property_tree::ptree &tree,
                         const std::unordered_map<std::string, ParameterSettings::ParametersSources> &parameters,
                         const std::unordered_map<std::string, std::vector<std::string> > &groups,
                         const Source::Parameters &parentPARA) noexcept;

        /**
         * @brief baseline setup function
         * @author Matthias Schartner
         *
         * As a start all parameter form parentPARA are used.
         * If different parameter values are defined in the event these parameters are used instead of the parentPARA
         * parameters.
         *
         * @param events list of all events for baseline
         * @param tree property tree that holds baseline setup information
         * @param parameters all defined parameters
         * @param groups all defined groups
         * @param parentPARA previously used parameters which are are use as template
         */
        void baselineSetup(std::vector<std::vector<Baseline::Event> > &events,
                           const boost::property_tree::ptree &tree,
                           const std::unordered_map<std::string, ParameterSettings::ParametersBaselines> &parameters,
                           const std::unordered_map<std::string, std::vector<std::string> > &groups,
                           const Baseline::Parameters &parentPARA) noexcept;

        /**
         * @brief number of minutes where source is visible
         * @author Matthias Schartner
         *
         * @param source target source
         * @param parameters source parameters
         * @param start start time
         * @param end end time
         * @return number of minutes where source was visible
         */
        unsigned int minutesVisible(const Source &source, const Source::Parameters &parameters, unsigned int start,
                                    unsigned int end);

        /**
         * @brief get members of station group
         * @author Matthias Schartner
         *
         * @param name group name
         * @param stations list of all available stations
         * @return list of station ids
         */
        std::vector<unsigned long> getMembers(const std::string &name, const std::vector<Station> &stations);

        /**
         * @brief get members of baseline group
         * @author Matthias Schartner
         *
         * @param name group name
         * @param stations list of all available sources
         * @return list of baseline ids
         */
        std::vector<unsigned long> getMembers(const std::string &name, const std::vector<Baseline> &baselines);

        /**
         * @brief get members of source group
         * @author Matthias Schartner
         *
         * @param name group name
         * @param stations list of all available baselines
         * @return list of source ids
         */
        std::vector<unsigned long> getMembers(const std::string &name, const std::vector<Source> &sources);
    };
}
#endif /* INITIALIZER_H */

