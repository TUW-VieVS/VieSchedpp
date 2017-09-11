/**
 * @file VLBI_initializer.h
 * @brief class VLBI_initializer
 *
 *
 * @author Matthias Schartner
 * @date 28.06.2017
 */

#ifndef VLBI_INITIALIZER_H
#define VLBI_INITIALIZER_H
#include <vector>
#include <boost/date_time.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <algorithm>

#include "VieVS_constants.h"
#include "VLBI_station.h"
#include "VLBI_source.h"
#include "VLBI_skyCoverage.h"
#include "VieVS_nutation.h"
#include "VieVS_earth.h"
#include "VieVS_lookup.h"
#include "VLBI_weightFactors.h"
#include "VLBI_obsMode.h"
#include "VieVS_time.h"
#include "VLBI_baseline.h"


#include "sofa.h"
#include "VLBI_multiSched.h"

using namespace std;
namespace VieVS {

    class VLBI_initializer {

    public:
        /**
         * @brief Parameters used in VLBI_initializer.
         *
         * Most of this parameters are than passed to other classes like VLBI_scheduler.
         */
        struct PARAMETERS {
            string experimentName; ///< experimet Name from .xml file
            string experimentDescription; ///< experiment description form xml file
            bool subnetting = true; ///< if set to true subnetting is enabled
            bool fillinmode = true; ///< it set to true fillin scans are calculated

            double maxDistanceTwinTeleskopes = 0; ///<
            vector<string> selectedStations; ///< list of all selected station for this session from .xml file

            double skyCoverageDistance = 30 * deg2rad; ///< maximum influence distance on sphere for sky Coverage
            double skyCoverageInterval = 3600; ///< maximum temporal distance of impact of scans in sky Coverage

            double minAngleBetweenSubnettingSources =
                    120 * deg2rad; ///< minimum angle between subnetting sources in radians
        };

        /**
         * @brief pre calculated values
         */
        struct PRECALC {
            vector<vector<int>> subnettingSrcIds; ///< list of all available second sources in subnetting
        };


        /**
         * @brief All available and read sked catalog files which can be read.
         */
        enum class catalog {
            antenna, ///< antenna.cat file
            position, ///< position.cat file
            equip, ///< equip.cat file
            mask, ///< mask.cat file
            source, ///< source.cat file
            flux ///< flux.cat file
        };

        /** @brief empty default constructor.
         *
         */
        VLBI_initializer(const std::string &path);

        /**
         * @brief default copy constructor
         *
         * @param other other scan
         */
        VLBI_initializer(const VLBI_initializer &other) = default;

        /**
         * @brief default copy assignment operator
         *
         * @param other other scan
         * @return copy of other scan
         */
        VLBI_initializer &operator=(const VLBI_initializer &other) = default;

        /**
         * @brief destructor
         */
        virtual ~VLBI_initializer();


        /**
         *  @brief pre calculates all possible second scans used for subnetting
         */
        void precalcSubnettingSrcIds() noexcept;

        /**
         * @brief This function reads a specific sked catalog file and stores the data in a map.
         *
         * @param root path to catalog files
         * @param type catalog file which should be read
         * @return key is list of all Ids, value is corresponding catalog entry
         */
        map<string, vector<string>> readCatalog(const string &root, const string &fname, catalog type, ofstream &headerLog) noexcept;

        /**
         * @brief creates all selected stations from sked catalogs
         */
        void createStations(ofstream &headerLog) noexcept;

        /**
         * @brief creates all possible sources from sked catalogs
         */
        void createSources(ofstream &headerLog) noexcept;

        /**
         * @brief creates all sky Coverage objects
         */
        void createSkyCoverages() noexcept;

        /**
         * @brief displays a short summary of created stations and sources.
         *
         * This is only for debugging purpose and usually unused
         */
        void displaySummary(ofstream &headerLog) noexcept;

        /**
         * @brief getter fuction which returns all stations
         * @return vector of all created station objects
         */
        const vector<VLBI_station> &getStations() const noexcept {
            return stations;
        }

        /**
         * @brief getter function which returns all sources
         * @return vector of all created source objects
         */
        const vector<VLBI_source> &getSources() const noexcept {
            return sources;
        }

        /**
         * @brief getter function which returns all skyCoverages
         * @return vector of all created sky coverage objects
         */
        const vector<VLBI_skyCoverage> &getSkyCoverages() const noexcept {
            return skyCoverages;
        }

        /**
         * @brief getter function which returns all sources
         * @return all parameters from this VLBI_initializer
         */
        const PARAMETERS &getPARA() const noexcept {
            return PARA;
        }

        const PRECALC &getPRE() const {
            return PRE;
        }

        void initializeGeneral(ofstream &headerLog) noexcept;

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
        void initializeObservingMode() noexcept;

        /**
         * @brief reads all groups spezified in the root tree
         *
         * @param root tree start point
         * @return key is group name, value is list of group members
         */
        unordered_map<string, vector<string> > readGroups(boost::property_tree::ptree root) noexcept;


        void applyMultiSchedParameters(const VieVS::VLBI_multiSched::PARAMETERS &parameters, ofstream &bodyLog);

        vector<VLBI_multiSched::PARAMETERS> readMultiSched();

    private:

        boost::property_tree::ptree PARA_xml; ///< content of parameters.xml file
        vector<VLBI_station> stations; ///< all created stations
        vector<VLBI_source> sources; ///< all created sources
        vector<VLBI_skyCoverage> skyCoverages; ///< all created sky coverage objects

        PARAMETERS PARA; ///< parameters
        PRECALC PRE; ///< pre calculated values


        void stationSetup(vector<vector<VLBI_station::EVENT> > &events,
                          const boost::property_tree::ptree &tree,
                          const unordered_map<string, VLBI_station::PARAMETERS> &parameters,
                          const unordered_map<string, vector<string>> &groups,
                          const VLBI_station::PARAMETERS &parentPARA) noexcept;

        void sourceSetup(vector<vector<VLBI_source::EVENT> > &events,
                         const boost::property_tree::ptree &tree,
                         const unordered_map<std::string, VLBI_source::PARAMETERS> &parameters,
                         const unordered_map<std::string, std::vector<std::string> > &groups,
                         const VLBI_source::PARAMETERS &parentPARA) noexcept;

        void baselineSetup(vector<vector<vector<VLBI_baseline::EVENT> > > &events,
                           const boost::property_tree::ptree &tree,
                           const unordered_map<std::string, VLBI_baseline::PARAMETERS> &parameters,
                           const unordered_map<std::string, std::vector<std::string> > &groups,
                           const VLBI_baseline::PARAMETERS &parentPARA) noexcept;

    };
}
#endif /* VLBI_INITIALIZER_H */

