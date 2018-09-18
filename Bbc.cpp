//
// Created by mschartn on 10.09.18.
//

#include "Bbc.h"

using namespace VieVS;
using namespace std;

unsigned long VieVS::Bbc::nextId = 0;
unsigned long VieVS::Bbc::Bbc_assign::nextId = 0;

Bbc::Bbc(std::string name): VieVS_NamedObject{std::move(name), nextId++} {

}

void Bbc::addBbc(std::string name, unsigned int physical_bbc_number, unsigned int if_name) {

    bbc_assigns_.emplace_back(name, physical_bbc_number, if_name);

}
