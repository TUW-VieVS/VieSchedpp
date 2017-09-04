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
#include <fstream>

#include <boost/format.hpp>
#include <cmath>
#include <utility>
#include <boost/optional.hpp>
#include <unordered_map>

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
            boost::optional<bool> available = true;

            boost::optional<double> weight = 1; ///< multiplicative factor of score for scans to this source

            unordered_map<string, double> minSNR; ///< minimum required signal to noise ration for each band

            boost::optional<unsigned int> minNumberOfStations = 2; ///< minimum number of stations for a scan
            boost::optional<double> minFlux = .01; ///< minimum flux density required for this source in jansky
            boost::optional<unsigned int> minRepeat = 1800; ///< minimum time between two observations of this source in seconds
            boost::optional<unsigned int> maxScan = 600; ///< maximum allowed scan time in seconds
            boost::optional<unsigned int> minScan = 30; ///< minimum required scan time in seconds

            boost::optional<unsigned int> fixedScanDuration;

            vector<int> ignoreStations;
            vector<string> ignoreStations_str;
            vector<pair<int, int>> ignoreBaselines;
            vector<pair<string, string>> ignoreBaselines_str;
        };

        struct PRECALCULATED{
            vector<double> sourceInCrs;
        };


        /**
         * @brief changes in parameters
         */
        struct EVENT {
            unsigned int time;
            bool softTransition;
            PARAMETERS PARA;
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

        /**
         * @brief default copy constructor
         *
         * @param other other source
         */
        VLBI_source(const VLBI_source &other) = default;

        /**
         * @brief default move constructor
         *
         * @param other other source
         */
        VLBI_source(VLBI_source &&other) = default;

        /**
         * @brief default copy assignment operator
         *
         * @param other other source
         * @return copy of other source
         */
        VLBI_source &operator=(const VLBI_source &other) = default;

        /**
         * @brief default move assignment operator
         *
         * @param other other source
         * @return moved other source
         */
        VLBI_source &operator=(VLBI_source &&other) = default;

        const PARAMETERS &getPARA() const {
            return PARA;
        }

        PARAMETERS &referencePARA() {
            return PARA;
        }


        /**
         * @brief get source position in CRS
         *
         * @return source position vector
         */
        const vector<double> &getSourceInCrs() const {
            return PRECALC.sourceInCrs;
        }

        /**
         * @brief getter for source name
         *
         * @return name of the source
         */
        const string &getName() const noexcept {
            return name;
        }

        /**
         * @brief getter for right ascension
         *
         * @return right ascension of the source in radians
         */
        double getRa() const noexcept {
            return ra;
        }

        /**
         * @brief getter for declination
         *
         * @return declination of the source in radians
         */
        double getDe() const noexcept {
            return de;
        }

        /**
         * @brief getter for id
         *
         * @return source id
         */
        int getId() const noexcept {
            return id;
        }

        /**
         * @brief getter for number of observed baselines
         *
         * @return number of baselines already observed for this source
         */
        unsigned long getNbls() const noexcept {
            return nbls;
        }

        /**
         * @brief looks for last scan time
         *
         * @return last observation time in seconds since session start
         */
        unsigned int lastScanTime() const noexcept {
            return lastScan;
        }

        /**
         * @brief minimum time between two scans
         *
         * @return minimum time between two scans in seconds
         */
        unsigned int minRepeatTime() const noexcept {
            return *PARA.minRepeat;
        }

        /**
         * @brief getter for minimum required SNR per band
         * @param band band name
         *
         * @return minimum required SNR for all bands
         */
        double getMinSNR(string band) const noexcept {
            return PARA.minSNR.at(band);
        }

        /**
         * @brief getter for minimum required scan time
         *
         * @return minimum required scan time in seconds
         */
        unsigned int getMinScanTime() const noexcept {
            return *PARA.minScan;
        }

        /**
         * @brief getter for maximum allowed scan time
         *
         * @return maximum allowed scan time in seconds
         */
        unsigned int getMaxScanTime() const noexcept {
            return *PARA.maxScan;
        }

        /**
         * @brief checks if a source is available
         *
         * @return true if source is available, otherwise false
         */
        bool isAvailable() const noexcept {
            return *PARA.available;
        }

        /**
         * @brief getter for fixed scan duration time
         *
         * returns an uninitialized value if no fixed scan duration is set.
         *
         * @return fixed scan duration or uninitilized object
         */
        boost::optional<unsigned int> getFixedScanDuration() const noexcept {
            if (PARA.fixedScanDuration.is_initialized()) {
                return *PARA.fixedScanDuration;
            } else {
                return boost::none;
            }
        }



        /**
         * @brief sets source id
         *
         * @param id new id
         */
        void setId(int id) noexcept {
            VLBI_source::id = id;
        }

        void setAvailable(bool flag) noexcept {
            PARA.available = flag;
        }


        void setEVENTS(const vector<EVENT> &EVENTS) noexcept {
            VLBI_source::EVENTS = EVENTS;
            VLBI_source::nextEvent = EVENTS[0].time;
        }

        /**
         * @brief getter for minimum number of stations for a scan
         * @return minimum number of stations for a scan
         */
        unsigned int getMinNumberOfStations() noexcept {
            return *PARA.minNumberOfStations;
        }

        /**
         * @brief angular distance between two sources
         *
         * @param other other source
         * @return angular distance in radians
         */
        double angleDistance(const VLBI_source &other) const noexcept;

        /**
         * checks if source is strong enough
         *
         * !!! this function changes maxFlux !!!
         *
         * // TODO: change this fuction
         * @param maxFlux maximum flux density of this source (will be calculated)
         * @return true if source is strong enough, otherwise false
         */
        bool isStrongEnough(double &maxFlux) const noexcept;

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
        vector<pair<string, double> > observedFlux(double gmst, double dx, double dy, double dz) const noexcept;

        /**
         * @brief destructor
         */
        virtual ~VLBI_source();

        /**
         * @brief this function checks if it is time to change the parameters
         * @param time current time
         * @param output displays output (default is false)
         * @return true if a new event was found
         */
        bool checkForNewEvent(unsigned int time, bool &hardBreak, bool output, ofstream &bodyLog) noexcept;


        /**
         * @brief updates scan to this source
         *
         * @param nbl number of baselines observed in scan to this source
         * @param time scan end time in seconds since start
         */
        void update(unsigned long nbl, unsigned int time) noexcept;

        /**
         * @brief overload of the << operator for output to stream
         *
         * @param out output stream object
         * @param src source information that should be printed to stream
         * @return stream object
         */
        friend ostream &operator<<(ostream &out, const VLBI_source &src) noexcept;
        
    private:
        string name; ///< source name
        int id; ///< source id
        double ra; ///< source right ascension
        double de; ///< source declination
        vector<pair<string, VLBI_flux> > flux; ///< source flux information per band

        PARAMETERS PARA; ///< parameters
        PRECALCULATED PRECALC;

        vector<EVENT> EVENTS;
        unsigned int nextEvent;

        unsigned int lastScan; ///< last scan to this source
        int nscans; ///< number of scans to this source
        unsigned long nbls; ///< number of observed baselines to this source
    };
    
    
}
#endif /* VLBI_SOURCE_H */

