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

#include "Satellite.h"

using namespace std;
using namespace VieVS;

Satellite::Satellite( const string& src_name, const string& src_name2,
                      unordered_map<std::string, std::unique_ptr<AbstractFlux>>& src_flux )
    : AbstractSource( src_name, src_name2, src_flux ) {}
double Satellite::getRa( unsigned int time ) const noexcept { return 0; }

double Satellite::getDe( unsigned int time ) const noexcept { return 0; }

const std::vector<double>& Satellite::getSourceInCrs( unsigned int time ) const { return preCalculated.sourceInCrs; }
