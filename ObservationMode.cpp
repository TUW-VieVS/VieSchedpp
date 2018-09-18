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

#include "ObservationMode.h"

double VieVS::ObservationMode::sampleRate = 0;
unsigned int VieVS::ObservationMode::bits = 0;
bool VieVS::ObservationMode::manual = true;

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

