/**
 * @file VLBI_source.h
 * @brief class VLBI_source
 *
 *
 * @author Matthias Schartner
 * @date 28.06.2017
 */

#ifndef VLBI_SOURCE_H
#define VLBI_SOURCE_H
#include <iostream>
#include <boost/date_time.hpp>
#include <boost/format.hpp>
#include <math.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <utility>

#include "VLBI_flux.h"
#include "VieVS_constants.h"

using namespace std;
namespace VieVS{
    /**
     * @class VLBI_source
     * @brief A quasar which can be scheduled
     *
     * First all source objects must be created, usually via the VLBI_initializer::createSourcesFromCatalogs()
     * Afterwards sources need to be initialized via VLBI_initializer::initializeSources()
     *
     * @author Matthias Schartner
     * @date 28.06.2017
    */
    class VLBI_source {
    public:
        /**
         * @brief source parameters
         */
        struct PARAMETERS{
            vector<string> parameterGroups; ///< name of .xml groups to which this source belongs

            double weight = 1; ///< multiplicative factor of score for scans to this source

            vector<pair<string, double> > minSNR; ///< minimum required SNR per band
            unsigned int minNumberOfStations = 2; ///< minimum number of stations for a scan
            double minFlux = .01; ///< minimum flux density required for this source in jansky
            unsigned int minRepeat = 1800; ///< minimum time between two observations of this source in seconds
            unsigned int maxScan = 600; ///< maximum allowed scan time in seconds
            unsigned int minScan = 30; ///< minimum required scan time in seconds
        };
        struct PRECALCULATED{
            vector<double> sourceInCrs;
        };

        /**
         * @brief empty default constructor
         */
        VLBI_source();

        /**
         * @brief constructor
         *
         * @param src_name name of the source
         * @param src_ra_deg right ascension in degrees
         * @param src_de_deg declination in degrees
         * @param src_flux flux information per band
         */
        VLBI_source(const string &src_name, double src_ra_deg, double src_de_deg,
                    const vector<pair<string, VLBI_flux> > &src_flux);

        const vector<double> &getSourceInCrs() const {
            return PRECALC.sourceInCrs;
        }

        /**
         * @brief getter for source name
         *
         * @return name of the source
         */
        const string &getName() const {
            return name;
        }

        /**
         * @brief getter for right ascension
         *
         * @return right ascension of the source in radians
         */
        double getRa() const {
            return ra;
        }

        /**
         * @brief getter for declination
         *
         * @return declination of the source in radians
         */
        double getDe() const {
            return de;
        }

        /**
         * @brief getter for id
         *
         * @return source id
         */
        int getId() const {
            return id;
        }

        /**
         * @brief getter for number of observed baselines
         *
         * @return number of baselines already observed for this source
         */
        unsigned long getNbls() const {
            return nbls;
        }

        /**
         * @brief looks for last scan time
         *
         * @return last observation time in seconds since session start
         */
        unsigned int lastScanTime() const {
            return lastScan;
        }

        /**
         * @brief minimum time between two scans
         *
         * @return minimum time between two scans in seconds
         */
        unsigned int minRepeatTime() const {
            return PARA.minRepeat;
        }

        /**
         * @brief getter for minimum required SNR per band
         *
         * @return minimum required SNR for all bands
         */
        const vector<pair<string, double> > &getMinSNR() const {
            return PARA.minSNR;
        }

        /**
         * @brief getter for minimum required scan time
         *
         * @return minimum required scan time in seconds
         */
        unsigned int getMinScanTime() const {
            return PARA.minScan;
        }

        /**
         * @brief getter for maximum allowed scan time
         *
         * @return maximum allowed scan time in seconds
         */
        unsigned int getMaxScanTime() const {
            return PARA.maxScan;
        }

        /**
         * @brief sets source id
         *
         * @param id new id
         */
        void setId(int id) {
            VLBI_source::id = id;
        }

        /**
         * @brief getter for minimum number of stations for a scan
         * @return minimum number of stations for a scan
         */
        unsigned int getMinNumberOfStations() {
            return PARA.minNumberOfStations;
        }

        /**
         * @brief angular distance between two sources
         *
         * @param other other source
         * @return angular distance in radians
         */
        double angleDistance(const VLBI_source &other) const;

        /**
         * checks if source is strong enough
         *
         * !!! this function changes maxFlux !!!
         *
         * //TODO: change this fuction
         * @param maxFlux maximum flux density of this source (will be calculated)
         * @return true if source is strong enough, otherwise false
         */
        bool isStrongEnough(double& maxFlux) const;

        /**
         * @brief sets all parameters from .xml group
         *
         * @param group group name
         * @param PARA_station .xml parameters
         */
        void setParameters(const string& group, const boost::property_tree::ptree& PARA_station);

        /**
         * @brief observed flux density per band
         *
         * // TODO CHECK gmst
         * @param gmst greenwhich meridian sedirial time
         * @param dx baseline delta x
         * @param dy baseline delta y
         * @param dz baseline delta z
         * @return observed flux density per band
         */
        vector<pair<string, double> > observedFlux(double gmst, double dx, double dy, double dz) const;

        /**
         * @brief destructor
         */
        virtual ~VLBI_source();

        /**
         * @brief updates scan to this source
         *
         * @param nbl number of baselines observed in scan to this source
         * @param time scan end time in seconds since start
         */
        void update(unsigned long nbl, unsigned int time);

        /**
         * @brief overload of the << operator for output to stream
         *
         * @param out output stream object
         * @param src source information that should be printed to stream
         * @return stream object
         */
        friend ostream& operator<<(ostream& out, const VLBI_source& src);
        
    private:
        string name; ///< source name
        int id; ///< source id
        double ra; ///< source right ascension
        double de; ///< source declination
        vector<pair<string, VLBI_flux> > flux; ///< source flux information per band

        PARAMETERS PARA; ///< parameters
        PRECALCULATED PRECALC;

        unsigned int lastScan; ///< last scan to this source
        int nscans; ///< number of scans to this source
        unsigned long nbls; ///< number of observed baselines to this source
    };
    
    
}
#endif /* VLBI_SOURCE_H */

