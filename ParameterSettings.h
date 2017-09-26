/**
 * @file ParameterSettings.h
 * @brief class ParameterSettings
 *
 * @author Matthias Schartner
 * @date 23.08.17
 */

#ifndef PARAMETERSETTINGS_H
#define PARAMETERSETTINGS_H

#include <ostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/date_time.hpp>
#include <utility>

#include "Station.h"
#include "Source.h"
#include "Baseline.h"
#include "ObservationMode.h"
#include "ParameterSetup.h"
#include "MultiScheduling.h"
#include "ParameterGroup.h"

namespace VieVS {

    /**
     * @class ParameterSettings
     * @brief This class stores all setup information and writes a parameters.xml file
     *
     * @author Matthias Schartner
     * @date 22.08.2017
     */
    class ParameterSettings {
    public:

        /**
         * @brief possible setup and optimization flags
         */
        enum class Type {
            station, ///< stations wise group/setup
            source, ///< source wise group/setup
            baseline, ///< baseline wise group/setup
        };


        /**
         * @brief empty default constructor
         */
        ParameterSettings();

        /**
         * @brief software block in parameter.xml
         *
         * @param name software name
         * @param version software version
         * @param created local time and date of created parameter.xml file
         */
        void software(const std::string &name, const std::string &version, const boost::posix_time::ptime &created);

        /**
         * @brief general block in parameter.xml
         *
         * @param startTime session start time
         * @param endTime session end time
         * @param subnetting flag if subnetting is allowed
         * @param fillinmode flag if fillin modes are allowed
         * @param stations list of all stations
         */
        void general(const boost::posix_time::ptime &startTime, const boost::posix_time::ptime &endTime,
                     bool subnetting, bool fillinmode, double minElevation,
                     const std::vector<std::string> &stations);

        /**
         * @brief catalogs block in parameters.xml
         *
         * @param root root directory path
         * @param antenna antenna catalog name
         * @param equip equip catalog name
         * @param flux flux catalog name
         * @param freq freq catalog name
         * @param hdpos hdpos catalog name
         * @param loif loif catalog name
         * @param mask mask catalog name
         * @param modes modes catalog name
         * @param position position catalog name
         * @param rec rec catalog name
         * @param rx rx catlog name
         * @param source source catalog name
         */
        void
        catalogs(const std::string &root, const std::string &antenna, const std::string &equip, const std::string &flux,
                 const std::string &freq, const std::string &hdpos, const std::string &loif, const std::string &mask,
                 const std::string &modes, const std::string &position, const std::string &rec, const std::string &rx,
                 const std::string &source, const std::string &tracks);

        /**
         * @brief group defined in parameters.xml
         *
         * @param type group type
         * @param group group object
         */
        void group(Type type, ParameterGroup group);

        /**
         * @brief returns all members of a defined group
         *
         * @param type group type
         * @param groupName group name
         * @return list of all group members
         */
        const std::vector<std::string> &getGroupMembers(Type type, std::string groupName);

        /**
         * @brief defined station parameters in station block in parameters.xml
         *
         * @param name parameter name
         * @param PARA parameters
         */
        void parameters(const std::string &name, Station::PARAMETERS PARA);

        /**
         * @brief defined source parameters in source block in parameters.xml
         *
         * @param name parameter name
         * @param PARA parameters
         */
        void parameters(const std::string &name, Source::PARAMETERS PARA);

        /**
         * @brief defined baseline parameters in baseline block in parameters.xml
         *
         * @param name parameter name
         * @param PARA parameters
         */
        void parameters(const std::string &name, Baseline::PARAMETERS PARA);

        /**
         * @brief setup block in parameter.xml
         *
         * @param type flag where this setup belongs
         * @param setup setup object
         */
        void setup(Type type, const ParameterSetup &setup);

        /**
         * @brief station wait times block
         *
         * @param name group, station name or "__all__"
         * @param setup time for setup in seconds
         * @param source time for source command in seconds
         * @param tape time for tape command in seconds
         * @param calibration time for preobservation in seconds
         * @param corsynch time for correlator synchronization
         */
        void stationWaitTimes(const std::string &name, unsigned int setup, unsigned int source, unsigned int tape,
                              unsigned int calibration, unsigned int corsynch);


        void stationCableWrapBuffer(const std::string &name, double axis1LowOffset, double axis1UpOffset,
                                    double axis2LowOffset, double axis2UpOffset);

        /**
         * @brief skyCoverage block in parameter.xml
         *
         * @param influenceDistance maximum angular influence distance in degrees
         * @param influenceInterval maximum time influence distance in seconds
         * @param maxTwinTelecopeDistance maximum distance between twin telescopes
         */
        void skyCoverage(double influenceDistance, unsigned int influenceInterval, double maxTwinTelecopeDistance);

        /**
         * @brief weightFactor block in parameter.xml
         *
         * @param weight_skyCoverage weight of sky Coverage
         * @param weight_numberOfObservations weight of number of observations
         * @param weight_duration weight of duration of the scan
         * @param weight_averageSources weight of an average source observation distribution
         * @param weight_averageStations weight of an average station usage distribution
         * @param weightDeclination weight factor for declination
         * @param declinationSlopeStart start declination of additional weight (everything above has factor 0)
         * @param declinationSlopeEnd end declination of additional declination weight slope (everything below has factor 1)
         * @param weightLowElevation weight factor for low elevation scans
         * @param lowElevationSlopeStart start elevation of additional weight (everything above has factor 0)
         * @param lowElevationSlopeEnd end elevation of additional declination weight slope (everything below has factor 1)
         */
        void weightFactor(double weight_skyCoverage, double weight_numberOfObservations, double weight_duration,
                          double weight_averageSources, double weight_averageStations, double weightDeclination,
                          double declinationSlopeStart, double declinationSlopeEnd, double weightLowElevation,
                          double lowElevationSlopeStart, double lowElevationSlopeEnd);

        /**
         * @brief mode block in parameter.xml
         *
         * @param sampleRate sample rate
         * @param bits bits
         */
        void mode(unsigned int sampleRate, unsigned int bits);

        /**
         * @brief band sub-block in mode block in parameter.xml
         *
         * @param name band name
         * @param wavelength band wavelength
         * @param station station policy for this band
         * @param stationBackup station backup model
         * @param stationBackupValue station backup model value
         * @param source source policy for this band
         * @param sourceBackup source backup model
         * @param sourceBackupValue source backup model value
         * @param chanels number of channels
         */
        void mode_band(const std::string &name, double wavelength, ObservationMode::Property station,
                       ObservationMode::Backup stationBackup, double stationBackupValue, ObservationMode::Property source,
                       ObservationMode::Backup sourceBackup, double sourceBackupValue, unsigned int chanels);

        /**
         * @brief multisched block in parameter.xml
         *
         * @param multiSched multisched object
         */
        void multisched(const MultiScheduling &multiSched);

        /**
         * @brief output routine that produces .xml file
         *
         * @param name output file name (usually "parameter")
         */
        void write(const std::string &name);

        /**
         * @brief output block in parameter.xml
         *
         * @param experimentName experiment name
         * @param experimentDescription experiment description
         * @param scheduler scheduler
         * @param correlator correlator
         * @param createSummary create summary file
         * @param createNGS create NGS file
         * @param createSKD create SKD file
         * @param createSkyCoverage create sky coverage file
         */
        void output(const std::string &experimentName, const std::string &experimentDescription,
                    const std::string &scheduler,
                    const std::string &correlator, bool createSummary, bool createNGS, bool createSKD,
                    bool createSkyCoverage);


        /**
         * @brief specify the sequence in which sources should be observed
         *
         * If you say cadence = 5, modulo = 0, source = A than every 5th selected scan(or subnetting sequence) will be
         * (if possible) to a source of A. This does not effect the first scan of a schedule!
         *
         * You can use the modulo number to to combine different rules:
         * cadence = 10, modulo = 0, source = "1234+567" and cadence = 10, modulo = 1, source = "1234+568" means that
         * every 10th scan will be (if possible) to source 1234+567 followed by a scan to "1234+568".
         *
         * @param cadence cadence for this rule
         * @param modulo modulo operator for number of scan and cadence
         * @param member source(group) which should be observed
         */
        void ruleScanSequence(unsigned int cadence, const std::vector<unsigned int> &modulo,
                              const std::vector<std::string> &member);

        void ruleCalibratorBlockTime(unsigned int cadence, const std::string &member,
                                     const std::vector<std::pair<double,double> > &between_elevation,
                                     unsigned int nMaxScans, unsigned int scanTime);

        void ruleCalibratorBlockNScanSelections(unsigned int cadence, const std::string &member,
                                                unsigned int nMaxScans, unsigned int scanTime);

    private:
        boost::property_tree::ptree master_; ///< master property tree

        std::map<std::string, std::vector<std::string> > groupStations_; ///< defined station group
        std::map<std::string, std::vector<std::string> > groupSources_; ///< defined source group
        std::map<std::string, std::vector<std::string> > groupBaselines_; ///< defined baseline group

        /**
         * @brief returns a child tree of root setup object
         *
         * @param setup root setup object
         * @return child property tree
         */
        boost::property_tree::ptree getChildTree(const ParameterSetup &setup);
    };
}


#endif //PARAMETERSETTINGS_H
