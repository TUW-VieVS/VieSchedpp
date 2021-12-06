//
// Created by mschartner on 12/6/21.
//

#include "AbstractEquipment.h"

using namespace std;
using namespace VieVS;

unsigned long AbstractEquipment::nextId_ = 0;
AbstractEquipment::AbstractEquipment() : VieVS_Object( nextId_++ ) {}
