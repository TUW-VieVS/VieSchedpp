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
 * File:   cableWrap.cpp
 * Author: mschartn
 *
 * Created on June 27, 2017, 11:47 AM
 */

#include "AbstractCableWrap.h"


using namespace std;
using namespace VieVS;

unsigned long AbstractCableWrap::nextId = 0;


AbstractCableWrap::AbstractCableWrap( double axis1_low_deg, double axis1_up_deg, double axis2_low_deg,
                                      double axis2_up_deg )
    : VieVS_Object( nextId++ ),
      axis1Low_{axis1_low_deg * deg2rad},
      axis1Up_{axis1_up_deg * deg2rad},
      axis2Low_{axis2_low_deg * deg2rad},
      axis2Up_{axis2_up_deg * deg2rad} {
    if ( ( axis1Up_ - axis1Low_ ) > twopi ) {
        double overlapping = ( axis1Up_ - axis1Low_ ) - twopi;
        if ( overlapping > twopi ) {
#ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL( error ) << "cable wrap limits to large";
#else
            cout << "[error] cable wrap limits to large";
#endif
        }
        wLow_ = axis1Low_;
        wUp_ = axis1Low_ + overlapping;
        nLow_ = axis1Low_ + overlapping;
        nUp_ = axis1Up_ - overlapping;
        cLow_ = axis1Up_ - overlapping;
        cUp_ = axis1Up_;
    } else {
        wLow_ = axis1Low_;
        wUp_ = axis1Low_;
        nLow_ = axis1Low_;
        nUp_ = axis1Up_;
        cLow_ = axis1Up_;
        cUp_ = axis1Up_;
    }
}


void AbstractCableWrap::setMinimumOffsets( double axis1_low_offset, double axis1_up_offset, double axis2_low_offset,
                                           double axis2_up_offset ) noexcept {
    axis1LowOffset_ = axis1_low_offset * deg2rad;
    axis1UpOffset_ = axis1_up_offset * deg2rad;
    axis2LowOffset_ = axis2_low_offset * deg2rad;
    axis2UpOffset_ = axis2_up_offset * deg2rad;
}


pair<double, double> AbstractCableWrap::getLimits( char section ) const {
    double lim1, lim2;

    if ( section == '-' ) {
        lim1 = nLow_;
        lim2 = nUp_;
    } else if ( section == 'C' ) {
        lim1 = cLow_;
        lim2 = cUp_;
    } else if ( section == 'W' ) {
        lim1 = wLow_;
        lim2 = wUp_;
    } else {
        lim1 = numeric_limits<double>::min();
        lim2 = numeric_limits<double>::max();
    }

    return {lim1, lim2};
}


bool AbstractCableWrap::axisInsideCableWrap( double ax1, double ax2 ) const noexcept {
    if ( ( ( axis1Up_ - axis1UpOffset_ ) - ( axis1Low_ + axis1LowOffset_ ) ) < 2 * pi ) {
        double ax1_1 = fmod( axis1Low_ + axis1LowOffset_, twopi );
        double ax1_2 = fmod( axis1Up_ - axis1UpOffset_, twopi );

        // over zero point or not
        if ( ax1_2 < ax1_1 ) {
            // over zero point
            if ( ( ax1 < ax1_1 && ax1 > ax1_2 ) || ax2 < axis2Low_ + axis2LowOffset_ ||
                 ax2 > axis2Up_ - axis2UpOffset_ ) {
                return false;
            }

        } else {
            // not over zero point
            if ( ax1 < ax1_1 || ax1 > ax1_2 || ax2 < axis2Low_ + axis2LowOffset_ || ax2 > axis2Up_ - axis2UpOffset_ ) {
                return false;
            }
        }
    } else {
        if ( ax2 < axis2Low_ + axis2LowOffset_ || ax2 > axis2Up_ - axis2UpOffset_ ) {
            return false;
        }
    }

    return true;
}


std::string AbstractCableWrap::pointingSector( const std::string &motion1, const std::string &motion2,
                                               char section ) const noexcept {
    if ( section == '-' ) {
        return ( boost::format( "        pointing_sector = &n     : %3s : %4.0f deg : %4.0f deg : %3s : %4.0f deg : "
                                "%4.0f deg ;\n" ) %
                 motion1 % ( nLow_ * rad2deg ) % ( nUp_ * rad2deg ) % motion2 % ( axis2Low_ * rad2deg ) %
                 ( axis2Up_ * rad2deg ) )
            .str();
    } else if ( section == 'C' ) {
        return ( boost::format( "        pointing_sector = &cw    : %3s : %4.0f deg : %4.0f deg : %3s : %4.0f deg : "
                                "%4.0f deg ;\n" ) %
                 motion1 % ( cLow_ * rad2deg ) % ( cUp_ * rad2deg ) % motion2 % ( axis2Low_ * rad2deg ) %
                 ( axis2Up_ * rad2deg ) )
            .str();
    } else if ( section == 'W' ) {
        return ( boost::format( "        pointing_sector = &ccw   : %3s : %4.0f deg : %4.0f deg : %3s : %4.0f deg : "
                                "%4.0f deg ;\n" ) %
                 motion1 % ( wLow_ * rad2deg ) % ( wUp_ * rad2deg ) % motion2 % ( axis2Low_ * rad2deg ) %
                 ( axis2Up_ * rad2deg ) )
            .str();
    }
}


double AbstractCableWrap::minLow( AbstractCableWrap::Axis axis ) const {
    switch ( axis ) {
        case Axis::axis1: {
            return axis1Low_ + axis1LowOffset_;
        }
        case Axis::axis2: {
            return axis2Low_ + axis2LowOffset_;
        }
    }
}


double AbstractCableWrap::maxUp( AbstractCableWrap::Axis axis ) const {
    switch ( axis ) {
        case Axis::axis1: {
            return axis1Up_ - axis1UpOffset_;
        }
        case Axis::axis2: {
            return axis2Up_ - axis2UpOffset_;
        }
    }
}


void AbstractCableWrap::calcUnwrappedAz( const PointingVector &old_pointingVector,
                                         PointingVector &new_pointingVector ) const noexcept {
    unwrapAzNearAz( new_pointingVector, old_pointingVector.getAz() );
}
