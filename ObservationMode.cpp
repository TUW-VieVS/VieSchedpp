//
// Created by mschartn on 21.08.17.
//

#include "ObservationMode.h"

double VieVS::ObservationMode::sampleRate = 0;
unsigned int VieVS::ObservationMode::bits = 0;
std::unordered_map<std::string,unsigned int> VieVS::ObservationMode::nChannels {};
std::unordered_map<std::string,double> VieVS::ObservationMode::wavelength {};

std::vector<std::string> VieVS::ObservationMode::bands; ///< list of bands
std::unordered_map<std::string, double> VieVS::ObservationMode::minSNR; ///< backup min SNR

std::unordered_map<std::string, VieVS::ObservationMode::Property> VieVS::ObservationMode::stationProperty; ///< is band required or optional for station
std::unordered_map<std::string, VieVS::ObservationMode::Backup> VieVS::ObservationMode::stationBackup; ///< backup version for station
std::unordered_map<std::string, double> VieVS::ObservationMode::stationBackupValue; ///< backup value for station

std::unordered_map<std::string, VieVS::ObservationMode::Property> VieVS::ObservationMode::sourceProperty; ///< is band required or optional for source
std::unordered_map<std::string, VieVS::ObservationMode::Backup> VieVS::ObservationMode::sourceBackup; ///< backup version for source
std::unordered_map<std::string, double> VieVS::ObservationMode::sourceBackupValue; ///< backup value for source

