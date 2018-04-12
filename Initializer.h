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
#include "Station.h"
#include "Source.h"
#include "SkyCoverage.h"
#include "Nutation.h"
#include "Earth.h"
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
            bool fillinmode = true; ///< it set to true fillin scans are calculated
            bool fillinmodeInfluenceOnSchedule = true; ///< fillin modes scans influence schedule if set to true
            std::vector<std::string> selectedStations; ///< list of all selected station for this session from .xml file

            double minAngleBetweenSubnettingSources = 120 * deg2rad; ///< minimum angle between subnetting sources in radians
            unsigned int maxNumberOfIterations = 999;
            unsigned int numberOfGentleSourceReductions = 0;
            unsigned int minNumberOfSourcesToReduce = 0;

            bool andAsConditionCombination = true;

        };

        /**
         * @brief pre calculated values
         */
        struct PRECALC {
            std::vector<std::vector<int>> subnettingSrcIds; ///< list of all available second sources in subnetting
        };

        Initializer();
        /**
         * @brief empty default constructor.
         */
        explicit Initializer(const std::string &path);

//        Initializer(Initializer&&) = default;
//        Initializer& operator=(Initializer&&) = default;
//
//        Initializer(const Initializer&);
//        Initializer& operator=(const Initializer&) = delete;
//
//        virtual ~Initializer() = default;

        const boost::property_tree::ptree &getXml() const {
            return xml_;
        }

        SkdCatalogReader createSkdCatalogReader() const noexcept;

        /**
         *  @brief pre calculates all possible second scans used for subnetting
         */
        void precalcSubnettingSrcIds() noexcept;

        /**
         * @brief creates all selected stations from sked catalogs
         *
         * @param headerLog outstream to log file
         */
        void createStations(SkdCatalogReader &reader, std::ofstream &headerLog) noexcept;

        /**
         * @brief creates all possible sources from sked catalogs
         *
         * @param headerLog outstream to log file
         */
        void createSources(SkdCatalogReader &reader, std::ofstream &headerLog) noexcept;

        /**
         * @brief creates all sky Coverage objects
         */
        void createSkyCoverages(std::ofstream &headerLog) noexcept;

        /**
         * @brief displays a short summary of created stations and sources.
         *
         * This is only for debugging purpose and usually unused
         *
         * @param headerLog outstream to log file
         */
        void displaySummary(std::ofstream &headerLog) noexcept;

        /**
         * @brief initializes general block in .xml file
         *
         * @param headerLog outstream to log file
         */
        void initializeGeneral(std::ofstream &headerLog) noexcept;

        /**
         * @brief initializes all stations with settings from .xml file
         */
        void initializeStations() noexcept;

        /**
         * @brief initializes all sources with settings from .xml file
         */
        void initializeSources() noexcept;

        /**
         * @brief calculates the nutation with the IAU2006a model in one hour steps
         *
         * This values are than used for interpolation of nuation in the calculation of each azimuth and elevation.
         * @see getAzEl
         */
        void initializeNutation() noexcept;

        /**
         * @brief initializes lookup tables for trigonometric functions to speed up calculation.
         * @see scorePerPointingVector()
         */
        void initializeLookup() noexcept;

        /**
         * @brief calculates velocity of earth at start time.
         *
         * used in the calculation of azimuth and elevation.
         * @see updateAzEl()
         */
        void initializeEarth() noexcept;

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
         * @brief initialzeBaselines
         */
        void initializeBaselines() noexcept;

        /**
         * @brief reads the observing mode information from xml file
         */
        void initializeObservingMode(SkdCatalogReader &reader, std::ofstream &headerLog) noexcept;

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
        void applyMultiSchedParameters(const VieVS::MultiScheduling::Parameters &parameters, std::ofstream &bodyLog);

        /**
         * @brief reads multiSched block in .xml file
         *
         * @return vector of all possible multisched parameter combination
         */
        std::vector<MultiScheduling::Parameters> readMultiSched();

        void initializeCalibrationBlocks(std::ofstream &headerLog);

        #ifdef _OPENMP
        void initializeMultiCore(int& nThreads, std::string & jobScheduling, int& chunkSize, std::string & threadPlace);
        #endif

        void statisticsLogHeader(std::ofstream &ofstream);

        void initializeOptimization(std::ofstream &ofstream);

    private:
        static int nextId;

        boost::property_tree::ptree xml_; ///< content of parameters.xml file
        std::vector<Station> stations_; ///< all created stations
        std::vector<Source> sources_; ///< all created sources
        std::vector<SkyCoverage> skyCoverages_; ///< all created sky coverage objects

        Parameters parameters_; ///< parameters
        PRECALC preCalculated_; ///< pre calculated values


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
        void baselineSetup(std::vector<std::vector<std::vector<Baseline::Event> > > &events,
                           const boost::property_tree::ptree &tree,
                           const std::unordered_map<std::string, ParameterSettings::ParametersBaselines> &parameters,
                           const std::unordered_map<std::string, std::vector<std::string> > &groups,
                           const Baseline::Parameters &parentPARA) noexcept;

        unsigned int minutesVisible(const Source &source, const Source::Parameters &parameters, unsigned int start,
                                    unsigned int end);
    };
}
#endif /* INITIALIZER_H */

