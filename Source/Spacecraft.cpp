//
// Created by mschartner on 10/22/25.
//

#include "Spacecraft.h"


using namespace std;
using namespace VieVS;

Spacecraft::Spacecraft(const std::string &name, std::unordered_map<std::string, std::unique_ptr<AbstractFlux>> &src_flux )
    :AbstractSource( name, name, src_flux ){

}

