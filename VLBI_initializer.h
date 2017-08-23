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
#include <boost/container/flat_map.hpp>
#include <boost/date_time.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "VieVS_constants.h"
#include "VLBI_station.h"
#include "VLBI_source.h"
#include "VLBI_skyCoverage.h"
#include "VieVS_nutation.h"
#include "VieVS_earth.h"
#include "VieVS_lookup.h"
#include "VLBI_weightFactors.h"
#include "VLBI_obsMode.h"
#include "VieVS_timeEvents.h"


#include "sofa.h"

using namespace std;
namespace VieVS{
    
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
        VLBI_initializer();

        /**
         * @brief destructor
         */
        virtual ~VLBI_initializer();

        /**
         * @brief This function reads a specific sked catalog file and stores the data in a map.
         *
         * @param path path to catalog files
         * @param type catalog file which should be read
         * @return key is list of all Ids, value is corresponding catalog entry
         */
        map<string,vector<string> > readCatalog(const string &path, catalog type);

        /**
         * @brief creates all selected stations from sked catalogs
         * @param catalogPath path to catalog files
         */
        void createStationsFromCatalogs(const string &catalogPath);

        /**
         * @brief creates all possible sources from sked catalogs
         * @param catalogPath path to catalog files
         */
        void createSourcesFromCatalogs(const string &catalogPath);

        /**
         * @brief creates all sky Coverage objects
         */
        void createSkyCoverages();

        /**
         * @brief displays a short summary of created stations and sources.
         *
         * This is only for debugging purpose and usually unused
         */
        void displaySummary();

        /**
         * @brief getter fuction which returns all stations
         * @return vector of all created station objects
         */
        const vector<VLBI_station> & getStations()const {
            return stations;
        }

        /**
         * @brief getter function which returns all sources
         * @return vector of all created source objects
         */
        const vector<VLBI_source> & getSources()const {
            return sources;
        }

        /**
         * @brief getter function which returns all skyCoverages
         * @return vector of all created sky coverage objects
         */
        const vector<VLBI_skyCoverage> & getSkyCoverages()const {
            return skyCoverages;
        }

        /**
         * @brief getter function which returns all sources
         * @return all parameters from this VLBI_initializer
         */
        const PARAMETERS & getPARA()const {
            return PARA;
        }

        /**
         * @brief initializes all stations with settings from .xml file
         */
        void initializeStations();

        /**
         * @brief initializes all sources with settings from .xml file
         */
        void initializeSources();

        /**
         * @brief calculates the nutation with the IAU2006a model in one hour steps
         *
         * This values are than used for interpolation of nuation in the calculation of each azimuth and elevation.
         * @see getAzEl
         */
        void initializeNutation();

        /**
         * @brief initializes lookup tables for trigonometric functions to speed up calculation.
         * @see scorePerPointingVector()
         */
        void initializeLookup();

        /**
         * @brief calculates velocity of earth at start time.
         *
         * used in the calculation of azimuth and elevation.
         * @see updateAzEl()
         */
        void initializeEarth();

        /**
         * @brief initializes the weight factors
         *
         */
        void initializeWeightFactors();


        /**
         * @brief inintializes the sky Coverage lookup table
         */
        void initializeSkyCoverages();

        /**
         * @brief initialzeBaselines
         */
        void initializeBaselines();

        /**
         * @brief reads the observing mode information from xml file
         */
        void initializeObservingMode();

    private:
        boost::property_tree::ptree PARA_xml; ///< content of parameters.xml file
        vector<VLBI_station> stations; ///< all created stations
        vector<VLBI_source> sources; ///< all created sources
        vector<VLBI_skyCoverage> skyCoverages; ///< all created sky coverage objects
        PARAMETERS PARA; ///< parameters
    };
}
#endif /* VLBI_INITIALIZER_H */

