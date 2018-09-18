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
#include "ObservationMode.h"
#include "TimeSystem.h"
#include "Baseline.h"
#include "SkdCatalogReader.h"
#include "CalibratorBlock.h"
#include "Scan.h"
#include "ParameterSettings.h"
#include "Flux_B.h"
#include "Flux_M.h"
#include "HighImpactScanDescriptor.h"

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
         */
        enum class GroupType {
            station, ///< stations wise group
            source, ///< source wise group
            baseline, ///< baseline wise group
        };

        /**
         * @brief Parameters used in VLBI_initializer.
         *
         * Most of this parameters are than passed to other classes like VLBI_scheduler.
         */
        struct Parameters {
            bool subnetting = true; ///< if set to true subnetting is enabled
            double subnettingMinAngle = 150*deg2rad;
            double subnettingMinNStaPercent = 0.80;
            double subnettingMinNStaAllBut = 1;
            double subnettingMinNStaPercent_otherwiseAllBut = false;

            bool fillinmodeDuringScanSelection = true; ///< it set to true fillin scans are calculated
            bool fillinmodeInfluenceOnSchedule = true; ///< fillin modes scans influence schedule if set to true
            bool fillinmodeAPosteriori = false;
            bool idleToObservingTime = true;

            std::vector<std::string> selectedStations; ///< list of all selected station for this session from .xml file

            unsigned int maxNumberOfIterations = 999;
            unsigned int numberOfGentleSourceReductions = 0;
            unsigned int minNumberOfSourcesToReduce = 0;

            bool andAsConditionCombination = true;

        };

        /**
         * @brief pre calculated values
         */
        struct PRECALC {
            std::vector<std::vector<unsigned long>> subnettingSrcIds; ///< list of all available second sources in subnetting
        };

        Initializer();

        /**
         * @brief empty default constructor.
         */
        explicit Initializer(const std::string &path);

        explicit Initializer(const boost::property_tree::ptree &xml);
        

        const boost::property_tree::ptree &getXml() const {
            return xml_;
        }

        /**
         *  @brief pre calculates all possible second scans used for subnetting
         */
        void precalcSubnettingSrcIds() noexcept;

        /**
         * @brief creates all selected stations from sked catalogs
         *
         * @param of outstream to log file
         */
        void createStations(const SkdCatalogReader &reader, std::ofstream &of) noexcept;

        /**
         * @brief initializes all stations with settings from .xml file
         */
        void initializeStations() noexcept;

        void precalcAzElStations() noexcept;

        void initializeBaselines() noexcept;

        /**
         * @brief creates all possible sources from sked catalogs
         *
         * @param of outstream to log file
         */
        void createSources(const SkdCatalogReader &reader, std::ofstream &of) noexcept;

        /**
         * @brief initializes all sources with settings from .xml file
         */
        void initializeSources() noexcept;

        /**
         * @brief initializes general block in .xml file
         *
         * @param of outstream to log file
         */
        void initializeGeneral(std::ofstream &of) noexcept;


        void initializeAstronomicalParameteres() noexcept ;

        /**
         * @brief initializes the weight factors
         *
         */
        void initializeWeightFactors() noexcept;


        /**
         * @brief inintializes the sky Coverage lookup table
         */
        void initializeSkyCoverages() noexcept;

        /**
         * @brief reads the observing mode information from xml file
         */
        void initializeObservingMode(const SkdCatalogReader &reader, std::ofstream &of) noexcept;

        /**
         * @brief initializes a custom source sequence if there is one defined in the .xml file
         */
        void initializeSourceSequence() noexcept;

        /**
         * @brief reads all groups spezified in the root tree
         *
         * @param root tree start point
         * @return key is group name, value is list of group members
         */
        std::unordered_map<std::string, std::vector<std::string> > readGroups(boost::property_tree::ptree root, GroupType type) noexcept;

        /**
         * @brief applies all multi scheduling parameters to the initializer
         *
         * @param parameters multi scheduling parameters
         * @param bodyLog outstream to log file
         */
        void applyMultiSchedParameters(const VieVS::MultiScheduling::Parameters &parameters);

        /**
         * @brief reads multiSched block in .xml file
         *
         * @return vector of all possible multisched parameter combination
         */
        std::vector<MultiScheduling::Parameters> readMultiSched(std::ostream &out);

        void initializeCalibrationBlocks(std::ofstream &of);

        void statisticsLogHeader(std::ofstream &of);

        void initializeOptimization(std::ofstream &of);

        void initializeHighImpactScanDescriptor(std::ofstream &of);

    private:
        static unsigned long nextId;

        boost::property_tree::ptree xml_; ///< content of parameters.xml file
        std::vector<Source> sources_; ///< all created sources
        Network network_;

        Parameters parameters_; ///< parameters
        PRECALC preCalculated_; ///< pre calculated values

        std::unordered_map<std::string, std::vector<std::string>> staGroups_;
        std::unordered_map<std::string, std::vector<std::string>> srcGroups_;
        std::unordered_map<std::string, std::vector<std::string>> blGroups_;

        boost::optional<HighImpactScanDescriptor> himp_;
        boost::optional<MultiScheduling::Parameters> multiSchedulingParameters_;


        /**
         * @brief station setup function
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

        unsigned int minutesVisible(const Source &source, const Source::Parameters &parameters, unsigned int start,
                                    unsigned int end);

        std::vector<unsigned long> getMembers(const std::string &name, const std::vector<Station> &stations);

        std::vector<unsigned long> getMembers(const std::string &name, const std::vector<Baseline> &baselines);

        std::vector<unsigned long> getMembers(const std::string &name, const std::vector<Source> &sources);
    };
}
#endif /* INITIALIZER_H */

