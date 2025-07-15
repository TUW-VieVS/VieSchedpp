//
// Created by mschartner on 2/19/25.
//

#include "Flux_satellite.h"

using namespace std;
using namespace VieVS;

Flux_satellite::Flux_satellite( double wavelength, double flux, double db_loss )
    : AbstractFlux{ wavelength }, flux_{ flux }, db_loss_{ db_loss } {}

double Flux_satellite::observedFluxElDist( double el, double dist ) const noexcept {
    double flux_90deg = flux_;
    double r = 6371000;  // Earth radius
    double h = 6000000;  // Gensis height
    double tmp = r / ( r + h ) * sin( 90 * deg2rad + el );
    double gamma = asin( tmp );  // angle between nadir and telescope - theta in Hakan's slides
    double db_reduction = -db_loss_ + db_loss_ * cos( gamma / 31 * 90 );
    double flux = flux_90deg / pow( 10, -db_reduction / 10.0 );

    double el_deg = el * ( 180.0 / M_PI );
    double gamma_deg = gamma * rad2deg;
    //    double flux_5deg = flux_90deg / pow(10, db_loss_ / 10.0);
    //    double cos_5deg = cos(5.0 * M_PI / 180.0);  // Convert 5 degrees to radians
    //    double flux = flux_5deg + (flux_90deg - flux_5deg) * sin((el_deg - 5) * (M_PI / 170.0));
    return flux;
}
