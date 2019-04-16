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

/*
 * File:   mask.cpp
 * Author: mschartn
 *
 * Created on June 27, 2017, 12:58 PM
 */

#include "AbstractHorizonMask.h"
using namespace std;
using namespace VieVS;
unsigned long AbstractHorizonMask::nextId = 0;

AbstractHorizonMask::AbstractHorizonMask() : VieVS_Object( nextId++ ) {}
